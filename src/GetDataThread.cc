/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $

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
/* $Header: /var/lib/cvs/kvalobs/src/kvbufferd/GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $ */
#include <milog/milog.h>
#include <sstream>
#include "boost/date_time/posix_time/ptime.hpp"
#include "miutil/timeconvert.h"
#include "getDataReceiver.h"
#include "GetDataThread.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;
namespace pt = boost::posix_time;

GetData::GetData(App &app_, const boost::posix_time::ptime &fromTime_, int wmono_, int hours_, std::shared_ptr<dnmi::thread::CommandQue> que_)
    : app(app_),
      que(que_),
      fromTime(fromTime_),
      joinable_(new bool(false)),
      wmono(wmono_),
      hours(hours_) {
}

void GetData::operator()() {
  pt::ptime now(pt::second_clock::universal_time());
  pt::ptime bufferFromTime(now);

  if (hours >= 0) {
    bufferFromTime -= pt::hours(6);

    if (fromTime >= bufferFromTime)
      bufferFromTime = fromTime;
  }

  LogContext lContext("GetDataKv(" + pt::to_kvalobs_string(now) + ")");

  //Generate buffer for maximum 7 hours back in time.

  LOGINFO("Started GetData thread!");
  IDLOGINFO("GetData", "Started GetData thread!");

  IDLOGINFO(
      "GetData",
      "fromTime: " << pt::to_kvalobs_string(fromTime) << " hours: " << hours << " wmono: " << wmono << endl << " bufferFromTime: " << pt::to_kvalobs_string(bufferFromTime));

  kvalobs::kvDbGateProxy gate(app.dbThread->dbQue);
  gate.busytimeout(300);

  if (wmono < 0) {
    reloadAll(gate, bufferFromTime);
  } else {
    reloadOne(gate, bufferFromTime);
  }

  LOGINFO("Exit GetData thread!");
  IDLOGINFO("GetData", "Exit GetData thread!");
  *joinable_ = true;
}

void GetData::reloadAll(kvalobs::kvDbGateProxy &gate, const pt::ptime &bufferFromTime) {
  //Mark all station to be reloaded.
  //StationList stationList = app.reloadCache(-1);

  StationList stationList=app.getStationList();
  for (IStationList it = stationList.begin(); it != stationList.end() && !app.shutdown(); it++) {
    reload(gate, *it, bufferFromTime);
  }
}

void GetData::reloadOne(kvalobs::kvDbGateProxy &gate, const pt::ptime &bufferFromTime) {
  StationList stationList = app.reloadCache(wmono);

  if (stationList.empty()) {
    LOGWARN("No station information for station: wmono:" << wmono <<"!");
    IDLOGWARN("GetData", "No station information for wmono: " << wmono << "!");
    return;
  }

  StationInfoPtr station = stationList.front();

  reload( gate, station, bufferFromTime);
}

void GetData::reload(kvalobs::kvDbGateProxy &gate, const StationInfoPtr station, const pt::ptime &bufferFromTime) {
  string logid;
  ostringstream ost;
  std::list<long> sids;
  std::list<long> tids;
  std::set<long> sidSet;
  std::set<long> tidSet;
  ost << "GetData-" << station->toIdentString();
  logid = ost.str();

  app.createGlobalLogger(logid);
  ost.str("");

  for( auto sid : station->definedStationID() ) {
    for( auto tid : station->typepriority()) {
      auto r = loadedSet.insert(GetData::Loaded(sid, tid));
      if( r.second ) {
        sidSet.insert(sid);
        tidSet.insert(tid);
      } else {
        IDLOGINFO(logid, "GetData: " << station->toIdentString() << " stationid: " << sid << " typeid: " << tid << " allready loaded");
        IDLOGINFO("GetData", "GetData: " << station->toIdentString() << " stationid: " << sid << " typeid: " << tid << " allready loaded");
      }
    }
  }

  if( sidSet.empty() ) {
    app.cacheReloaded(station);
    ost.str("");
    for (auto &s : station->definedStationID()) {
      ost << " " << s;
    }

    IDLOGINFO(logid, "GetData: " << station->toIdentString() << " all stationids: " << ost.str() <<  ", allready loaded");
    IDLOGINFO("GetData", "GetData: " << station->toIdentString() << " all stationids: " << ost.str() <<  ", allready loaded");
    return;  
  }

  for( auto sid : sidSet) {
    sids.push_back(sid);
  }
  for( auto tid : tidSet) {
    tids.push_back(tid);
  } 

  for (auto &s : sids)
    ost << " " << s;

  IDLOGINFO(logid, "Started GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));
  IDLOGINFO("GetData", "Started GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));

  try {
    app.cacheReload(station);
    GetKvDataReceiver myDataReceiver(app, bufferFromTime, que, gate, logid);

    if (!app.getKvData([&myDataReceiver]
    (long sid, long tid, const pt::ptime &obstime, const kvalobs::ObsDataElement &data) {
      myDataReceiver.next(sid, tid, obstime, data);
    },
                       sids, tids, fromTime, pt::ptime(), true)) {
      IDLOGERROR("GetData", "Failed GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));
      IDLOGERROR(logid, "Failed GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));
    } else {
      IDLOGINFO("GetData", "Success GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));
      IDLOGINFO(logid, "Success GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime));
    }
  } catch (const std::exception &ex) {
    IDLOGERROR("GetData",
               "Exception while loading data: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime) << " Reason: " << ex.what());
    IDLOGERROR(logid, "Exception: GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime)<< " Reason: " << ex.what());
  }
  catch( ... ) {
    IDLOGERROR("GetData",
                   "Exception while loading data: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime) << " Reason: Unknown.");
    IDLOGERROR(logid, "Exception: GetData: " << station->toIdentString() << " stationids:" << ost.str() << " FromTime: " << pt::to_kvalobs_string(fromTime)<< " Reason: Unknown.");
  }

  app.cacheReloaded(station);
}

