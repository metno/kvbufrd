/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: BufrWorker.cc,v 1.27.2.21 2007/09/27 09:02:23 paule Exp $

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "BufrWorker.h"
#include "App.h"
#include "Data.h"
#include "LoadBufrData.h"
#include "SemiUniqueName.h"
#include "StationInfo.h"
#include "base64.h"
#include "boost/crc.hpp"
#include "boost/cstdint.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "bufr/EncodeBufrManager.h"
#include "bufr.h"
#include "kvDbGateProxy.h"
#include "kvalobs/kvPath.h"
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "utils.h"
#include <errno.h>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <list>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <set>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

namespace {
const char* STATUS_CORRECTION = "correction";
const char* STATUS_NEW = "new";
const char* STATUS_DUPLICATE = "duplicate";
const char* STATUS_DELAYED = "delay";



void
_logObservation(const char* status,
                StationInfoPtr info,
                const pt::ptime obstime,
                const pt::ptime delayTo,
                int ccx,
                const std::string &stationAndTypes,
                const pt::time_duration duration)
{
  std::string sDelay;
  std::string sDuration("0");
  std::string cntxt=milog::Logger::logger().getContextString();
  
  BufrWorker::logMetrics->setContext(cntxt);
  BufrWorker::logMetrics->setLoglevel("INFO");

  if (!delayTo.is_special()) {
    sDelay = pt::to_kvalobs_string(delayTo);
  }

  if (!duration.is_special()) {
    ostringstream o;
    o << duration.total_milliseconds();
    sDuration = o.str();
  }

  ostringstream o;
  o << status << ": (" << info->wmono() << "/" << info->stationID() << "/"
                   << info->callsign() << "/" << info->codeToString() << "/"
                   << ccx << "/" << pt::to_kvalobs_string(obstime) << "/"
                   << sDelay << ") " << stationAndTypes
                   << " duration=" << sDuration << "ms";
  BufrWorker::logMetrics->log(o.str());
  /*
  IDLOGINFO("observation",
            status << ": (" << info->wmono() << "/" << info->stationID() << "/"
                   << info->callsign() << "/" << info->codeToString() << "/"
                   << ccx << "/" << pt::to_kvalobs_string(obstime) << "/"
                   << sDelay << ") " << stationAndTypes
                   << " duration=" << sDuration << "ms");
  */
}

void
logObservation(StationInfoPtr info,
               const pt::ptime obstime,
               int ccx,
               const std::string &stationAndTypes,
               const pt::time_duration duration)
{
  const char* status = STATUS_NEW;

  if (ccx != 0) {
    status = STATUS_CORRECTION;
  }
  _logObservation(status, info, obstime, pt::ptime(), ccx, stationAndTypes, duration);
}

void
logDuplicateObservation(StationInfoPtr info,
                        const pt::ptime obstime,
                        int ccx,
                        const std::string &stationAndTypes,
                        const pt::time_duration duration)
{
  _logObservation(STATUS_DUPLICATE, info, obstime, pt::ptime(), ccx,stationAndTypes, duration);
}

void
logDelayObservation(StationInfoPtr info,
                    const pt::ptime obstime,
                    const std::string &stationAndTypes,
                    const pt::ptime delayTo)
{
  _logObservation(
    STATUS_DELAYED, info, obstime, delayTo, -1, stationAndTypes, pt::time_duration());
}


std::string compactTime(const pt::ptime &t) {
  char b[32];
  auto d = t.date();
  auto tm = t.time_of_day();
  int Y = d.year();
  int M = d.month();
  int D = d.day();
  int h=tm.hours();
  int m = tm.minutes();
  int s=tm.seconds();
  snprintf(b, 32, "%4d%02d%02dT%02d%02d%02d", Y, M, D, h, m, s);
  b[31]='\0';
  return std::string(b);
}

std::string
stationAndType(const DataEntryList& data,
               StationInfoPtr stInfo,
               const pt::ptime& obstime)
{
  typedef std::map<long, std::map<int,pt::ptime> > T;
  StationInfo::TLongList tids = stInfo->typepriority();
  DataEntryList::CITDataEntryList dit = data.find(obstime);
  T s;

  if (dit == data.end()) {
    LOGDEBUG("stationAndTypes: No data for: "
             << stInfo->toIdentString()
             << " obstime: " << pt::to_kvalobs_string(obstime));
    return "";
  }

  if (dit->obstime() != obstime) {
    LOGDEBUG("stationAndTypes: No data for obstime: "
             << pt::to_kvalobs_string(obstime)
             << " station: " << stInfo->toIdentString());
    return "";
  }

  for (auto tid : tids) {
    auto sids = dit->stationIds(tid);
    for( auto sid : sids ) {
      s[sid][tid]=dit->maxTbTime(sid, tid);
    }
  }

  auto f = [](std::ostringstream &o, const T::value_type &e) {
    o << e.first << "/";
    auto first = true;
    for ( auto tid : e.second) {
      if ( first ) {
        first=false;
        o << tid.first;
      } else {
        o << "," << tid.first;
      }
      if ( ! tid.second.is_special() ) {
          o << "." << compactTime(tid.second);
      }
    }
  };

  std::ostringstream o;
  o << "[";
  auto first = true;
  for (auto &e : s) {
    if (first) {
      first = false;
      f(o, e);
    } else {
      o << "|";
      f(o,e);
    }
  }
  
  o << "]";
  return o.str();
}
}

LogAppender *BufrWorker::logMetrics=nullptr;

BufrWorker::BufrWorker(App& app_,
                       std::shared_ptr<dnmi::thread::CommandQue> que_)
  : app(app_)
  , que(que_)
  , swmsg(*(new std::ostringstream()))
  , encodeBufrManager(new EncodeBufrManager())
   
{
  ostringstream o;
  if( EncodeBufrManager::masterBufrTable < 10 || EncodeBufrManager::masterBufrTable>99) {
    LOGFATAL("Buffer master table must be in the intervall [10,99], it is " << EncodeBufrManager::masterBufrTable );
    exit(1);
  }

  IDLOGINFO("main", "BufrWorker: Logging metrics to '" << app.options.obslogfile<< "'");

  logMetrics = new LogAppender(app.options.obslogfile);

  if ( logMetrics == nullptr ) {
    LOGFATAL("FAILED to create metric logger. Logfile '" << app.options.obslogfile << "'.");
    exit(1);
  }

  o << string(DATADIR) << "/B00000000000000" << EncodeBufrManager::masterBufrTable <<  "000.TXT";
  //string filename = string(DATADIR) + "/B0000000000000019000.TXT";
  bufrParamValidater = BufrParamValidater::loadTable(o.str());
}

void
BufrWorker::operator()()
{
  dnmi::thread::CommandBase* com;
  ObsEvent* event;

  milog::LogContext context("BufrWorker");

  while (!app.shutdown()) {
    try {
      com = que->get(1);
    } catch (dnmi::thread::QueSuspended& e) {
      LOGDEBUG6("EXCEPTION(QueSuspended): Input que is susspended!" << endl);
      continue;
    } catch (...) {
      LOGDEBUG("EXCEPTION(Unknown): Uknown exception from input que!" << endl);
      continue;
    }

    if (!com)
      continue;

    event = dynamic_cast<ObsEvent*>(com);

    if (!event) {
      LOGERROR("Unexpected event!");
      delete com;
      continue;
    }

    LOGINFO("New observation: ("
            << event->stationInfo()->toIdentString() << ") "
            << pt::to_kvalobs_string(event->obstime())
            << " regenerate: " << (event->regenerate() ? "T" : "F")
            << " note: " <<  event->note());

    try {
      FLogStream* logs = new FLogStream(2, 307200); // 300k
      std::ostringstream ost;

      ost << kvPath("logdir") << "/" << app.options.progname << "/"
          << event->stationInfo()->toIdentString() << ".log";

      if (logs->open(ost.str())) {
        Logger::setDefaultLogger(logs);
        Logger::logger().logLevel(event->stationInfo()->loglevel());

        // Log to stationspecific logfile also.
        LOGINFO("+++++++ Start processing observation +++++++"
                << endl
                << "New observation: (" << event->stationInfo()->toIdentString()
                << ") " << pt::to_kvalobs_string(event->obstime())
                << " regenerate: " << (event->regenerate() ? "T" : "F"));
      } else {
        LOGERROR("Cant open the logfile <" << ost.str() << ">!");
        delete logs;
      }
    } catch (...) {
      LOGERROR("Cant create a logstream for station: "
               << event->stationInfo()->toIdentString());
    }

    swmsg.str("");

    try {
      newObs(*event);
    } catch (...) {
      LOGERROR("EXCEPTION(Unknown): Unexpected exception from "
               << "BufrWorker::newObs" << endl);
    }

    delete event;

    LOGINFO("------- End processing observation -------");

    Logger::resetDefaultLogger();

    LOGINFO(swmsg.str());
  }
}

bool
BufrWorker::readyForBufr(const DataEntryList& data, ObsEvent& e) const
{
  bool haveAllTypes;
  bool mustHaveTypes;
  bool force;
  int delayMin;
  bool relativToFirst;

  pt::ptime obstime = e.obstime();
  StationInfoPtr info = e.stationInfo();

  milog::LogContext context("readyForBufr");

  pt::ptime delayTime;
  pt::ptime nowTime(pt::second_clock::universal_time());

  haveAllTypes = checkTypes(data, info, obstime, mustHaveTypes);
  delayMin = info->delay(obstime, force, relativToFirst);

  LOGDEBUG3("haveAllTypes:  "
            << (haveAllTypes ? "TRUE" : "FALSE") << endl
            << "mustHaveTypes: " << (mustHaveTypes ? "TRUE" : "FALSE") << endl
            << "delay: " << (delayMin != 0 ? "TRUE" : "FALSE")
            << " min: " << delayMin << " force: " << (force ? "TRUE" : "FALSE")
            << " relativToFirst: " << (relativToFirst ? "TRUE" : "FALSE")
            << endl
            << " nowTime: " << nowTime << endl
            << " #ContinuesTimes: " << data.nContinuesTimes() << endl);

  if (!haveAllTypes && !mustHaveTypes) {
    // We do not have all types we need, we are also missing
    // the types we need to make an incomplete bufr. Just
    // drop this event, dont make a waiting element for it, it is
    // useless;

    swmsg << "Missing mustHaveTypes!";
    return false;
  }

  if (delayMin > 0) {
    if (relativToFirst) {
      // If we allready have a registred waiting element dont replace it.
      // This ensures that we only register one waiting element for
      // the first data received.

      if (haveAllTypes)
        return true;

      delayTime = nowTime;
      delayTime += pt::minutes(delayMin);
      WaitingPtr wp = e.waiting();

      if (!wp) {
        LOGDEBUG1("Delaying (relativeToFirst): " << delayTime);
        auto wpRet = app.addWaiting(
          WaitingPtr(new Waiting(delayTime, obstime, info,"relative to first")), false);
        if (!wpRet) {
          logDelayObservation(info, obstime, stationAndType(data, info, obstime),delayTime);
        }

        swmsg << "Delay (relativToFirst): " << delayTime;
        return false;
      } else {
        LOGDEBUG1("Is delayed (relativeToFirst) to: " << wp->delay());
        if (wp->delay() <= nowTime) {
          LOGDEBUG1("Is delayed (relativeToFirst): expired!");
          return true;
        } else {
          LOGDEBUG1("Is delayed (relativeToFirst): Not expired!");
          // Reinsert in the delaylist.
          auto wpRet = app.addWaiting(wp, false);

          swmsg << "Delay (relativToFirst) Not expired: " << delayTime;
          if (!wpRet) {
            logDelayObservation(info, obstime, stationAndType(data, info, obstime), delayTime);
          }

          return false;
        }
      }
    } else { // Delay relative to obstime.
      delayTime = pt::ptime(
        obstime.date(),
        pt::time_duration(obstime.time_of_day().hours(), delayMin, 0));
    }
  }

  if (haveAllTypes) {
    if (delayMin > 0) {
      if (!force) {
        return true;
      } else {
        if (delayTime < nowTime)
          return true;

        try {
          auto wpRet = app.addWaiting(
            WaitingPtr(new Waiting(delayTime, obstime, info, "delay")), true);
          logDelayObservation(info, obstime, stationAndType(data, info, obstime), delayTime);
        } catch (...) {
          LOGFATAL("NOMEM: cant allocate delay element!");
        }

        swmsg << "Delay: " << delayTime;

        return false;
      }
    } else {
      return true;
    }
  }

  if (delayMin > 0) {
    if (delayTime < nowTime) {
     return mustHaveTypes;
    } else {
      try {
        std::string note("delay");
        if( !mustHaveTypes ) {
          note="Missing required types";
        } 
        auto wpRet = app.addWaiting(
          WaitingPtr(new Waiting(delayTime, obstime, info, note)), true);
        logDelayObservation(info, obstime, stationAndType(data, info, obstime),delayTime);
      } catch (...) {
        LOGFATAL("NOMEM: cant allocate delay element!");
      }
    }
  } else {
    if (mustHaveTypes)
      return true;
  }

  swmsg << "Not enough data!";
  return false;
}

/*
  If the event has a callback registered we write some error
  message that can be returned to the caller.
 */
void
BufrWorker::newObs(ObsEvent& event)
{
  EReadData dataRes;
  DataEntryList data;
  DataElementList bufrData;
  Bufr bufrEncoder;
  std::shared_ptr<BufrHelper> bufrHelper;
  StationInfoPtr info;
  ostringstream ost;
  boost::uint32_t oldcrc = 0;
  int ccx = 0;
  list<TblBufr> tblBufrList;
  pt::ptime start = pt::microsec_clock::universal_time();
  auto logLevel = LOGLEVEL();
  bool debug = logLevel >= milog::LogLevel::DEBUG;

  info = event.stationInfo();

  LOGINFO("New observation: ("
            << event.stationInfo()->toIdentString() << ") "
            << pt::to_kvalobs_string(event.obstime())
            << " regenerate: " << (event.regenerate() ? "T" : "F")
            << " delay note: '" <<  event.note() << "'\n"
            << "Station configuration:\n" << *info)

  if (!info->msgForTime(event.obstime())) {
    LOGINFO("Skip BUFR for time: " << pt::to_kvalobs_string(event.obstime())
                                   << " " << info->toIdentString());
    swmsg << "Skip BUFR for time: " << pt::to_kvalobs_string(event.obstime())
          << " " << info->toIdentString();

    return;
  }

  // We check if this is a event for regeneraiting a BUFR
  // due to changed data. If it is, the bufr for this time
  // must allready exist. If it don't exist we could generate
  // a BUFR that is incomplete because of incomplete data.

  if (event.regenerate()) {
    list<TblBufr> tblBufrList;

    LOGINFO("Regenerate event: " << info->toIdentString() << ", obstime "
                                 << pt::to_kvalobs_string(event.obstime()));

    if (app.getSavedBufrData(info, event.obstime(), tblBufrList)) {
      if (tblBufrList.size() > 0) {
        LOGINFO("Regenerate event: Regenerate BUFR.");
      } else {
        LOGINFO("Regenerate event: No BUFR exist, don't regenerate!");
        swmsg << "Regenerate: No bufr exist.";

        return;
      }
    } else {
      LOGERROR("DBERROR: Regenerate event: Cant look up the bufr!");
      swmsg << "Regenerate: DB error!";

      return;
    }
  }

  dataRes = readData(event, data);

  if (dataRes != RdOK) {
    swmsg << event.msg().str();

    return;
  }

  // If this event comes from the DelayControll and
  // is for data waiting on continues types don't run
  // it through the readyForBufr test. We know that
  // the readyForBufr has previously returned true for
  // this event.

  if (!event.waitingOnContinuesData()) {
    // Don't delay a observation that is explicit asked for.
    // A BUFR that is explicit asked for has a callback.

    if (!readyForBufr(data, event)) {
      return;
    }
  }

  // Check if we shall test for continuesTypes. We do that
  // if we have no Waiting pointer or we have a Waiting pointer,
  // but it has not been tested for waiting on continues data.
  if (!checkContinuesTypes(event, data)) {
    return;
  }

  app.removeWaiting(info, event.obstime());

  LOGINFO("ReadData: " << info->toIdentString()
                       << " obstime: " << pt::to_kvalobs_string(event.obstime())
                       << " # " << data.size()
                       << " debug: " << (debug ? "true" : "false")
                       << " loglevel: " << logLevel);

  try {
    loadBufrData(data, bufrData, event.stationInfo());
  } catch (...) {
    LOGDEBUG("EXCEPTION(Unknown): Unexpected exception from "
             << endl
             << "BufrWorker::loadBufrData" << endl);
  }

  CIDataElementList it = bufrData.begin();
  ost.str("");

  for (int i = 0; it != bufrData.end(); i++, it++)
    ost << it->time() << "  [" << i << "] #params "
        << bufrData[i].numberOfValidParams() << endl;

  LOGINFO("# number of bufrdata: " << bufrData.size() << endl
                                   << "Continues: "
                                   << bufrData.nContinuesTimes() << endl
                                   << "Time(s): " << endl
                                   << ost.str());

  if (bufrData.firstTime() != event.obstime()) {
    LOGWARN(
      "NO data at '" << pt::to_kvalobs_string(event.obstime())
                     << "' passed the valid checks. Skipping BUFR generation.");
    return;
  }

  if (app.getSavedBufrData(info, event.obstime(), tblBufrList)) {
    if (tblBufrList.size() > 0) {
      ccx = tblBufrList.front().ccx();
      oldcrc = tblBufrList.front().crc();
      ++ccx;
      LOGDEBUG("A BUFR for: " << info->toIdentString() << " obstime: "
                              << pt::to_kvalobs_string(event.obstime())
                              << " exist. ccx=" << ccx - 1
                              << " crc: " << oldcrc);
    }
  }

  LOGDEBUG6(bufrData);

  try {
    bufrHelper = bufrEncoder.encodeBufr(info, bufrData);
  } catch (const IdException& e) {
    LOGWARN("EXCEPTION: Cant resolve for BUFR id: "
            << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl
            << "what: " << e.what() << endl);
    swmsg << "Cant create a bufr!" << endl;
  } catch (const NotImplementedException& e) {
    LOGWARN("EXCEPTION: No template implemmented for: "
            << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl
            << "what: " << e.what() << endl);
    swmsg << "Cant create a bufr!" << endl;
  } catch (std::out_of_range& e) {
    LOGWARN("EXCEPTION: out_of_range: "
            << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl
            << "what: " << e.what() << endl);
    swmsg << "Cant create a bufr!" << endl;
  } catch (DataListEntry::TimeError& e) {
    LOGWARN("Exception: TimeError: "
            << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl
            << "what: " << e.what() << endl);
    swmsg << "Cant create a bufr!" << endl;
  } catch (std::logic_error& e) {
    LOGWARN("EXCEPTION: logic_error: "
            << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl
            << "what: " << e.what() << endl);
    swmsg << "Cant create a bufr!" << endl;
  } catch (...) {
    LOGWARN("EXCEPTION(Unknown): Unexpected exception in Bufr::doBufr:"
            << endl
            << " station: " << info->toIdentString() << " obstime: "
            << ((bufrData.begin() != bufrData.end())
                  ? pt::to_kvalobs_string(bufrData.begin()->time())
                  : "(NULL)")
            << endl);
    swmsg << "Cant create a bufr!" << endl;
  }

  if (!bufrHelper) {
    LOGERROR("Cant create BUFR for <"
             << info->toIdentString()
             << "> obstime: " << pt::to_kvalobs_string(event.obstime()));
    swmsg << "Cant create a BUFR!" << endl;
  } else {
    boost::uint32_t crc = bufrHelper->computeCRC();
    string base64;

    LOGINFO("Data used to generate BUFR CRC, crc: " << crc << " (" << oldcrc << ")" << endl << *bufrHelper->getData()); 
    
    bool newBufr(crc != oldcrc);

    if (newBufr) {
      pt::ptime createTime(pt::second_clock::universal_time());
      ostringstream dataOst;

      bufrData.writeTo(dataOst, true, debug);
      if (saveTo(bufrHelper, ccx, &base64)) {
        if (app.saveBufrData(TblBufr(info->wmono(),
                                     info->stationID(),
                                     info->callsign(),
                                     info->codeToString(),
                                     event.obstime(),
                                     createTime,
                                     crc,
                                     ccx,
                                     dataOst.str(),
                                     base64))) {
          LOGINFO("BUFR information saved to database! ("
                  << info->toIdentString() << ") ccx: " << ccx
                  << " crc: " << crc);
        } else {
          LOGERROR("FAILED to save BUFR information to the database! ("
                   << info->toIdentString() << ") ccx: " << ccx
                   << " crc: " << crc);
        }

        pt::ptime now = pt::microsec_clock::universal_time();
        logObservation(info, event.obstime(), ccx, stationAndType(data, info, event.obstime()), now - start);

        swmsg << "New BUFR created. (" << info->toIdentString() << ") "
              << pt::to_kvalobs_string(event.obstime()) << endl;
      }
    } else {
      LOGINFO("DUPLICATE: (" << info->toIdentString() << ") "
                             << pt::to_kvalobs_string(event.obstime()));

      swmsg << "Duplicate BUFR created. (" << info->toIdentString() << ") "
            << pt::to_kvalobs_string(event.obstime()) << endl;
      pt::ptime now = pt::microsec_clock::universal_time();
      logDuplicateObservation(info, event.obstime(), ccx, stationAndType(data, info, event.obstime()), now - start);
    }

    ost.str("");
  }
}

namespace {
  struct SidTidData {
    long sid;
    long tid;
    SidTidData(long s, long t):sid(s), tid(t){}
    bool operator<(const SidTidData &d) const {
      return (sid < d.sid) || 
      (sid==d.sid && tid < d.tid);
    }
  };

  class SidTidDataSet: public virtual std::set<SidTidData>{
    boost::posix_time::ptime obstime;
    std::list<long> sids;
  
    public:
      SidTidDataSet(const boost::posix_time::ptime &obstime, const std::list<long> &sids):
      obstime(obstime), sids(sids){}
      
      void add( long sid, long tid, const pt::ptime &obstime) {
        if ( obstime != this->obstime) {
          return;
        }

        insert( SidTidData(sid, tid) );
      }

      void logObservationLoaded() {
        std::ostringstream ost;
        ost << "Observation loaded for obstime "<< pt::to_kvalobs_string(obstime) << ":"; 
        if( size() == 0 ) {
          ost << " No observations loaded for station(s)";
          for ( auto sid : sids ) {
            ost << " " << sid;
          }
          LOGINFO(ost.str());
          return;
        } 

        for( auto &e : *this ) {
          ost << " (" << e.sid << "/" << e.tid << ")";
        }
        LOGINFO(ost.str());
      }
  };

}




BufrWorker::EReadData
BufrWorker::readData(ObsEvent& event, DataEntryList& data) const
{
  ostringstream ost;
  kvDbGateProxy gate(app.dbThread->dbQue);
  list<Data> dataList;
  list<Data>::iterator dit;
  DataEntryList::CITDataEntryList it;
  StationInfo::TLongList stIDs;
  StationInfo::RITLongList itStId;
  StationInfoPtr station = event.stationInfo();
  pt::ptime from(event.obstime());
  pt::ptime to(event.obstime());
  bool hasObstime = false;
  kvdatacheck::Validate validate(kvdatacheck::Validate::UseOnlyUseInfo);

  data.clear();

  if (!station)
    return RdERROR;

  gate.busytimeout(300);

  from -= pt::hours(24);

  stIDs = station->definedStationID();
  itStId = stIDs.rbegin();

  SidTidDataSet loadedData(event.obstime(), stIDs);


  if (itStId == stIDs.rend()) {
    LOGERROR("No stationid's for station <" << station->toIdentString()
                                            << ">!");
    return RdNoStation;
  }

  for (; itStId != stIDs.rend(); itStId++) {
    ost.str("");
    ost << " where stationid=" << *itStId << " AND "
        << " obstime>=\'" << pt::to_kvalobs_string(from) << "\' AND "
        << " obstime<=\'" << pt::to_kvalobs_string(to) << "\'"
        << " order by obstime, typeid;";

    LOGDEBUG("query: " << ost.str());

    if (!gate.select(dataList, ost.str())) {
      LOGERROR("Cant get data from the database!\nQuerry: "
               << ost.str() << endl
               << "Reason: " << gate.getErrorStr());
      return RdERROR;
    }

    bool doLogTypeidInfo = true;

    for (dit = dataList.begin(); dit != dataList.end(); dit++) {
      try {
        if( station->hasDefinedStationIdAndTypeId(dit->stationID(), dit->typeID(), dit->obstime().time_of_day().hours())) {
          if (validate(*dit)) {
            loadedData.add(dit->stationID(), dit->typeID(), dit->obstime());
            data.insert(*dit);

            if (!hasObstime && dit->obstime() == event.obstime())
              hasObstime = true;
          }
        }
        /*
        if (event.hasReceivedTypeid(
              dit->stationID(), dit->typeID(), doLogTypeidInfo)) {

          if (validate(*dit)) {
            data.insert(*dit);

            if (!hasObstime && dit->obstime() == event.obstime())
              hasObstime = true;
          }
        }

*/
        doLogTypeidInfo = false;
      } catch (DataListEntry::TimeError& e) {
        LOGDEBUG("EXCEPTION(DataListEntry::TimeError): Hmmm... "
                 << "This should not happend!!" << endl);
      }
    }
  }

  loadedData.logObservationLoaded();

  string rejected = validate.getLog();

  if (!rejected.empty()) {
    LOGWARN("RJECTED data." << endl << rejected);
  }

  it = data.begin();

  if (it != data.end()) {
    ostringstream ost;
    ost << "First obstime: " << pt::to_kvalobs_string(it->obstime()) << " - ";
    it = data.end();
    it--;
    ost << pt::to_kvalobs_string(it->obstime());
    LOGDEBUG(ost.str());
  } else {
    LOGWARN("No data in the cache for the station!");
    return RdNoData;
  }

  if (!hasObstime) {
    LOGERROR(
      "No data for the obstime: " << pt::to_kvalobs_string(event.obstime()));
    return RdMissingObstime;
  }

  return RdOK;
}

void
BufrWorker::loadBufrData(const DataEntryList& dl,
                         DataElementList& sd,
                         StationInfoPtr info) const
{
  kvdatacheck::Validate validate(kvdatacheck::Validate::NoCheck);
  ::loadBufrData(dl, sd, info, validate);
}

bool
BufrWorker::checkTypes(const DataEntryList& data,
                       StationInfoPtr stInfo,
                       const pt::ptime& obstime,
                       bool& mustHaveTypes) const
{
  StationInfo::TLongList tids = stInfo->typepriority();
  DataEntryList::CITDataEntryList dit = data.find(obstime);
  StationInfo::CITLongList it;

  mustHaveTypes = false;

  if (dit == data.end()) {
    LOGDEBUG("checkTypes: No data for: " << stInfo->toIdentString()
                                         << " obstime: "
                                         << pt::to_kvalobs_string(obstime));
    return false;
  }

  if (dit->obstime() != obstime) {
    LOGDEBUG("checkTypes: No data for obstime: "
             << pt::to_kvalobs_string(obstime)
             << " station: " << stInfo->toIdentString());
    return false;
  }

  LOGDEBUG("checkTypes: " << *dit);

  for (it = tids.begin(); it != tids.end(); it++) {
    if (dit->size(*it) == 0)
      break;
  }

  if (it == tids.end()) {
    mustHaveTypes = true;
    return true;
  }

  tids = stInfo->mustHaveTypes();
  ostringstream ost;

  // If we have no must have types defined, mustHaveTypes is set to true.
  if (tids.empty()) {
    mustHaveTypes = true;
    ost << "checkTypes: No must have types is defined, this is ok.";
  } else {
    ost << "checkTypes: mustHaveTypes:";

    for (it = tids.begin(); it != tids.end(); it++)
      ost << " " << *it;

    for (it = tids.begin(); it != tids.end(); it++) {
      if (dit->size(*it) == 0)
        break;
    }

    if (it == tids.end())
      mustHaveTypes = true;
  }

  LOGDEBUG(ost.str());
  return false;
}

bool
BufrWorker::saveTo(StationInfoPtr info,
                   BufrDataPtr bufr,
                   int ccx,
                   std::string* base64) const
{
  bool doSave = true;
  int nValues;

  try {
    BufrHelper bufrHelper(bufrParamValidater, info, bufr);
    bufrHelper.setSequenceNumber(ccx);
    encodeBufrManager->encode(bufrHelper);

    if (!bufrHelper.validBufr()) {
      LOGINFO("INVALID BUFR: " << bufrHelper.getErrorMessage());
      return false;
    }

    doSave = !bufrHelper.emptyBufr();
    nValues = bufrHelper.nValues();

    if (!doSave) {
      LOGINFO("No data values was written to the BUFR message, the BUFR "
              "message is NOT saved.");
      return false;
    }

    LOGINFO("BUFR with " << nValues << " real data values is saved.")
    bufrHelper.saveToFile();

    if (base64) {
      ostringstream ost;
      bufrHelper.writeToStream(ost);
      string buf = ost.str();
      encode64(buf.data(), buf.size(), *base64);
    }
    return true;
  } catch (const std::exception& ex) {
    LOGERROR("Failed to encode to BUFR: " << info->toIdentString()
                                          << " obstime: "
                                          << pt::to_kvalobs_string(bufr->time())
                                          << ". Reason: " << ex.what());
  }
  return false;
}

bool BufrWorker::saveTo(std::shared_ptr<BufrHelper> bufrHelper,
              int ccx,
              std::string *base64 ) const {
  bool doSave = true;
  int nValues;

  try {
    if (!bufrHelper->validBufr()) {
      LOGINFO("INVALID BUFR: " << bufrHelper->getErrorMessage());
      return false;
    }

    bufrHelper->setSequenceNumber(ccx);
    

    doSave = !bufrHelper->emptyBufr();
    nValues = bufrHelper->nValues();

    if (!doSave) {
      LOGINFO("No data values was written to the BUFR message, the BUFR "
              "message is NOT saved.");
      return false;
    }

    LOGINFO("BUFR with " << nValues << " real data values is saved.")
    bufrHelper->saveToFile();

    if (base64) {
      ostringstream ost;
      bufrHelper->writeToStream(ost);
      string buf = ost.str();
      encode64(buf.data(), buf.size(), *base64);
    }
    return true;
  } catch (const std::exception& ex) {
    LOGERROR("Failed to encode to BUFR: " << bufrHelper->getStationInfo()->toIdentString()
                                          << " obstime: "
                                          << pt::to_kvalobs_string(bufrHelper->getData()->time())
                                          << ". Reason: " << ex.what());
  }
  return false;                

}



bool
BufrWorker::checkContinuesTypes(ObsEvent& event,
                                const DataEntryList& data) const
{
  WaitingPtr w;
  StationInfoPtr info = event.stationInfo();

  if (event.waitingOnContinuesData()) {
    // We have waited on this event in the predefined time,
    // just generate a bufr for this event.
    swmsg << "Expired waiting on continues data!" << endl;
    return true;
  }

  if ((event.obstime().time_of_day().hours() % 3) != 0) {
    // Just interested in bufrtimes that use data from
    // multiple hours.

    return true;
  }

  w = app.getWaiting(event.obstime(), info);

  std::list<int> contTypes = info->continuesTypeids(app.continuesTypeID());

  if (contTypes.empty()) {
    // The station has only no continues type ids.
    return true;
  }

  if (!data.hasContinuesTimes(contTypes, 4)) {
    if (!w) {
      pt::ptime now(pt::second_clock::universal_time());
      now += pt::minutes(5);

      try {
        w = WaitingPtr(new Waiting(now, event.obstime(), info, "continues types", true));
      } catch (...) {
        LOGERROR("NO MEM");
        return true;
      }
    }

    w->waitingOnContinuesData(true);

    app.addWaiting(w, true);

    LOGINFO("Waiting on continues data: "
            << info->toIdentString()
            << " obstime: " << pt::to_kvalobs_string(event.obstime())
            << " delay: " << pt::to_kvalobs_string(w->delay()));

    swmsg << "Waiting on continues data until: "
          << pt::to_kvalobs_string(w->delay());

    return false;
  }

  return true;
}
