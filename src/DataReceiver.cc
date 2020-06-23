/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: DataReceiver.cc,v 1.14.2.20 2007/09/27 09:02:22 paule Exp $

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
#include <list>
#include <stdlib.h>
#include <exception>
#include "boost/algorithm/string/replace.hpp"
#include "kvalobs/kvPath.h"
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "App.h"
#include "Data.h"
#include "kvDbGateProxy.h"
#include "DataReceiver.h"
#include "tblBufr.h"
#include "Waiting.h"

using namespace std;
using namespace kvservice;
using namespace kvalobs;
using namespace milog;

namespace pt = boost::posix_time;
using std::terminate;
using std::get;
using std::list;
using std::endl;
using kvalobs::kvData;

DataReceiver::DataReceiver(App &app_, std::shared_ptr<dnmi::thread::CommandQue> inputQue_, std::shared_ptr<dnmi::thread::CommandQue> outputQue_)
    : app(app_),
      inputQue(inputQue_),
      outputQue(outputQue_) {
}

void DataReceiver::operator()() {
  dnmi::thread::CommandBase *com;
  DataEvent *event;

  milog::LogContext context("DataReceiver");

  while (!app.shutdown()) {
    com = inputQue->get(1);

    if (!com)
      continue;

    event = dynamic_cast<DataEvent*>(com);

    if (!event) {
      //Is this an event from the delaycontrol thread or
      //getDataThread. getDataThread is started at program startup
      //to get "old" data from kvalobs. It is terminated when
      //all "old" data is received.

      ObsEvent *obsevent = dynamic_cast<ObsEvent*>(com);

      if (!obsevent) {
        LOGERROR("Unexpected event!");
        delete com;
        continue;
      }

      doCheckReceivedData(obsevent);

      continue;
    }

    try {
      newData(*event);
    } catch (const std::exception &ex) {
      LOGERROR("newData: Exception: " << ex.what() << "\nabort()");
      terminate();
    }

    delete event;
  }
}

void DataReceiver::doCheckReceivedData(ObsEvent *obsevent) {
  milog::LogContext context("delaycontrol");

  setDefaultLogger(obsevent->stationInfo());

  if (!obsevent->stationInfo()->msgForTime(obsevent->obstime())) {
    LOGINFO("Skip BUFFER for this hour: " << obsevent->obstime() << " " << obsevent->stationInfo()->toIdentString());
    delete obsevent;
    Logger::resetDefaultLogger();
    return;
  }

  if (!typeidReceived(*obsevent)) {
    LOGERROR(
        "Cant get information about received stationid/typeid!"<< "Deleting event: " << obsevent->stationInfo()->toIdentString() << " obstime: " << obsevent->obstime());
    delete obsevent;
    obsevent = 0;
  } else if (obsevent->nTypeidReceived() == 0) {
    LOGWARN("No stationid/typeid received!"<< "Deleting event: " << obsevent->stationInfo()->toIdentString() << " obstime: " << obsevent->obstime());
    delete obsevent;
    obsevent = 0;
  }

  LOGDEBUG3("Resend the event on outputque (newObsQue)");

  if (obsevent)
    outputQue->postAndBrodcast(obsevent);

  Logger::resetDefaultLogger();
}

namespace {
void logObservation( ){
} 
std::ostream& 
print(std::ostream &o, const kvalobs::kvData &d) {
  o << d.stationID() <<", " << d.typeID() << ", " << pt::to_kvalobs_string(d.obstime()) << ", " << d.paramID() 
    << ", " << d.original() << ", " << d.sensor() << ", " << d.level() << ", " << d.controlinfo().flagstring() 
    << ", " << d.useinfo().flagstring() << ", " << pt::to_kvalobs_string(d.tbtime());

  return o;
} 
}

void DataReceiver::newData(const DataEvent &event) {
  kvalobs::kvDbGateProxy gate(app.dbThread->dbQue);
  StationInfoList stations;
  pt::ptime toTime;
  pt::ptime fromTime;
  DataKeySet dataInserted;
  auto producer = event.producer();
  auto data = event.data();


  //data, er en liste av lister <KvData> til observasjoner
  //med samme stationid, typeid og obstime.

  gate.busytimeout(600);
  milog::LogContext context("newdata");

  toTime = pt::second_clock::universal_time();
  fromTime = toTime;
  toTime += pt::hours(3);
  
  //7 days ago
  fromTime -= pt::hours(7*24);

  LOGINFO("Accepting data in the time interval: " << pt::to_kvalobs_string(fromTime)<< " - " << pt::to_kvalobs_string(toTime));

  

  for (auto &index : data->getAllIndex()) {
    std::string insertedBy;
    auto &it = data->get(index);    
    
    list<kvData>::iterator dit = get<0>(it).begin();

    if (dit == get<0>(it).end()) {
      LOGWARN("Data received from kvalobs: Unexpected, NO Data!");
      IDLOGERROR("DataReceiver", "Data received from kvalobs: Unexpected, NO Data!\nDataReceived\n----- BEGIN -----\n"<<
        event.inCommingMessage()<<"\n----- END -----");
      continue;
    }

    stations = app.findStationInfo(dit->stationID());

    if (stations.empty()) {
      LOGWARN("NO StationInfo: " << dit->stationID());
      continue;
    }

    for ( auto station : stations ) {
      std::list<Data> dataList;
      dit = get<0>(it).begin();
      LOGINFO(
          "Data received from kvalobs: stationID: " << dit->stationID() << " (" << station->toIdentString() << ")" << endl 
          << "       obstime: " << pt::to_kvalobs_string(dit->obstime()) << endl 
          << "        typeid: " << dit->typeID() << endl 
          << "   #parameters: " << get<0>(it).size() );

      if (!app.acceptAllTimes() && (dit->obstime() < fromTime || dit->obstime() > toTime)) {
        LOGWARN(
            "obstime to old or to early: " << pt::to_kvalobs_string(dit->obstime()) << endl 
            << "-- Valid interval: " << pt::to_kvalobs_string(fromTime) << " - " << pt::to_kvalobs_string(toTime));

        //No 'continue' here! We reevaluate after we have set up to log
        //to a station specific file and write the message to the logfile
        //before we continue.
        //See  **1**

      }

      if (!station->hasTypeId(dit->typeID())) {
        LOGWARN("StationInfo: typeid: " << dit->typeID() << " not used!");

        //No 'continue' here! We reevaluate after we have set up to log
        //to a station specific file and write the message to the logfile
        //before we continue.
        //See  **2**
      }

      //Setup to log to a station specific file.
      setDefaultLogger(station);

      //This is repeated here to get it out on the log file for the wmono.
      std::ostringstream o;
      for(auto &d: get<0>(it)) {
        print(o,d) << endl;
      }

      LOGINFO(
          "Data received from kvalobs: stationID: " << dit->stationID() << endl
           << " (" << station->toIdentString() << ")" << endl 
           << "       obstime: " << dit->obstime() << endl 
           << "        typeid: " << dit->typeID() << endl 
           << "   #parameters: " << get<0>(it).size() << endl
           <<"       producer: " << producer << endl
           << "Accepting data in time interval: " << pt::to_kvalobs_string(fromTime) << " - " << pt::to_kvalobs_string(toTime) << endl
           << "Data: " << endl
           << o.str());
      
      //**1**
      if (!app.acceptAllTimes() && (dit->obstime() < fromTime || dit->obstime() > toTime)) {
        LOGWARN(
            "obstime to old or to early: " << pt::to_kvalobs_string(dit->obstime()) << endl 
            << "-- Valid interval: " << pt::to_kvalobs_string(fromTime) << " - " << pt::to_kvalobs_string(toTime));
        Logger::resetDefaultLogger();
        LOGWARN(
            "obstime to old or to early: " << pt::to_kvalobs_string(dit->obstime()) << endl 
            << "-- Valid interval: " << pt::to_kvalobs_string(fromTime) << " - " << pt::to_kvalobs_string(toTime));
        continue;
      }

      //**2**
      if (!station->hasTypeId(dit->typeID())) {
        LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
        Logger::resetDefaultLogger();
        LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
        continue;
      }

      dataList.clear();
      for (auto &d : get<0>(it)) {
        if (dataInserted.add(DataKey(d)))
          dataList.push_back(Data(d));
      }

      if( !dataList.empty() ) {
        insertedBy="(" + station->toIdentString() + ")";
      }
      
      dit = get<0>(it).begin();

      if (dataList.size() == 0 && insertedBy.empty() ) {
        LOGDEBUG(
            "No new data to insert into the database:" << endl << "  stationid: " << dit->stationID() << " obstime: " << pt::to_kvalobs_string(dit->obstime()) << " typeid: " << dit->typeID() << endl);
        Logger::resetDefaultLogger();
        LOGDEBUG(
            "No new data to insert in database:" << endl << "  stationid: " << dit->stationID() << " obstime: " << pt::to_kvalobs_string(dit->obstime()) << " typeid: " << dit->typeID() << endl);
        continue;
      }

      if( ! dataList.empty() ) {
        o.str("");
        for( auto &d: dataList) {
          o << d << endl;
        }
        LOGINFO("Data to insert/update" << endl << o.str());

        DataInsertCommand toInsert(dataList, "datareceiver_saved");
        if (!gate.doExec(&toInsert)) {
          LOGERROR(
           "Cant insert data: \n  stationid: " << dit->stationID() << " obstime: " << pt::to_kvalobs_string(dit->obstime()) << "  reason: " << gate.getErrorStr() << endl);
          Logger::resetDefaultLogger();
          LOGERROR(
            "Cant insert data: \n  stationid: " << dit->stationID() << " obstime: " << pt::to_kvalobs_string(dit->obstime()) << "  reason: " << gate.getErrorStr() << endl);

          continue;
        }
      } else {
        LOGINFO("Data for: " << dit->stationID() <<"/" << dit->typeID() << "/" << pt::to_kvalobs_string(dit->obstime()) << " allready inserted by:  " << insertedBy);
      }

      if (!station->msgForTime(dit->obstime())) {
        LOGINFO(
            "Skip BUFR for this hour: " << pt::to_kvalobs_string(dit->obstime()) << endl << " stationid: " << dit->stationID() << endl << " typeid: " << dit->typeID());
        Logger::resetDefaultLogger();
        LOGINFO(
            "Skip BUFR for this hour: " << pt::to_kvalobs_string(dit->obstime()) << endl << " stationid: " << dit->stationID() << endl << " typeid: " << dit->typeID());
        continue;
      }

      ObsEvent *event;

      try {
        event = new ObsEvent(dit->obstime(), station);

        o.str("");
        if (!typeidReceived(*event)) {
          o << "FAILED: Cant get informatiom about (stationid/typeid) " << "received for station <" << event->stationInfo()->toIdentString()
            << "> at obstime: " << pt::to_kvalobs_string(event->obstime()) << endl;
          LOGWARN(
              "FAILED: Cant get informatiom about (stationid/typeid) " << "received for station <" << event->stationInfo()->toIdentString() << "> at obstime: " << pt::to_kvalobs_string(event->obstime()));
        } else if (event->nTypeidReceived() == 0) {
          o << "No data received (stationid/typeid)!" << endl << "Station: " << event->stationInfo()->toIdentString() << " obstime: "
            << pt::to_kvalobs_string(event->obstime()) << endl;

          LOGWARN(
              "No data received (stationid/typeid)!" << endl << "Station: " << event->stationInfo()->toIdentString() << " obstime: " << pt::to_kvalobs_string(event->obstime()));
          delete event;
        } else {
          app.addObsEvent(event, *outputQue);
        }
      } catch (...) {
        o << "NOMEM: cant send a ObsEvent!";
        LOGERROR("NOMEM: cant send a ObsEvent!");
      }

      prepareToProcessAnyBufrBasedOnThisObs(event->obstime(), station);

      Logger::resetDefaultLogger();

      LOGINFO(o.str());
    }
  }
}

void DataReceiver::prepareToProcessAnyBufrBasedOnThisObs(const pt::ptime &obstime, StationInfoPtr station) {
  //TODO: This function only works correct for observation
  //on whole hours. It must be looked through again to be implemented
  //for observations with better resolution than one hour. It only
  //regenerates BUFR that is on WMO standard times ie. 0,3,6,..,21.

  std::list<TblBufr> bufrData;
  pt::ptime now(pt::second_clock::universal_time());
  pt::ptime maxTime;
  pt::ptime time = obstime;
  pt::ptime delayBefore(now.date(), pt::time_duration(now.time_of_day().hours(), 0, 0));
  
  delayBefore -= pt::hours(1);

  maxTime = pt::ptime(obstime.date(), pt::time_duration(obstime.time_of_day().hours(), 0, 0));
  maxTime += pt::hours(station->numberOfHourToRegenerate());

  milog::LogContext context("regenerate");

  if (maxTime > now)
    maxTime = pt::ptime(now.date(), pt::time_duration(now.time_of_day().hours(), 0, 0));

  int hour = obstime.time_of_day().hours();
  int r = hour % 3;

  //Nearest BUFR time larger than obstime;
  if (r == 0)
    time += pt::hours(3);
  else
    time += pt::hours(3 - r);

  while (time <= maxTime) {

    if (!station->msgForTime(time)) {
      time += pt::hours(3);
      continue;
    }

    LOGINFO("Possibly regenerating BUFR for: <" << station->toIdentString() << "> obstime: " << pt::to_kvalobs_string(time)<<endl);

    try {
      //if( time < delayBefore )
      ObsEvent *event = new ObsEvent(time, station, true);

      if (!typeidReceived(*event)) {
        LOGWARN(
            "FAILED: Can't get information about (stationid/typeid) " << "received for station <"<< event->stationInfo()->toIdentString() << "> at obstime " << pt::to_kvalobs_string(event->obstime()) << ".");
        delete event;
        return;
      } else if (event->nTypeidReceived() == 0) {
        LOGWARN(
            "No data received (stationid/typeid)!" << endl << "Station: " << event->stationInfo()->toIdentString() << " obstime: " << pt::to_kvalobs_string(event->obstime()));
        delete event;
      } else if ( time < delayBefore ) {
        regenerateToWaiting( event, 2 ); //Add the event to the waiting que, delay 2 minutes
      } else {
        app.addObsEvent(event, *outputQue);
      }
    } catch (...) {
      LOGERROR("NOMEM: can't send a ObsEvent!");
    }

    time += pt::hours(3);
  }

}

void
DataReceiver::regenerateToWaiting(ObsEvent *event, int minutesToDelay){
  pt::ptime delay(pt::second_clock::universal_time());
  delay += pt::minutes(minutesToDelay);

  auto w = WaitingPtr(new Waiting(delay, event->obstime(), event->stationInfo(),"regenerate",false));
  delete event;

  w=app.addWaiting(w, true);
  if( w ) {
    LOGINFO("Replacing waiting element: obstime: " << pt::to_kvalobs_string(w->obstime()) << " note: '" << w->note() << "'.");
  }
}

bool DataReceiver::typeidReceived(ObsEvent &event) {
  StationInfo::TLongList stations = event.stationInfo()->definedStationID();
  StationInfo::TLongList::iterator it = stations.begin();
  ostringstream ost;

  kvDbGateProxy gate(app.dbThread->dbQue);
  gate.busytimeout(300);

  if (it == stations.end())
    return false;

  ost << "SELECT distinct stationid,typeid FROM data WHERE " << "obstime='" << pt::to_kvalobs_string(event.obstime()) << "' AND " << "stationid IN (" << *it;

  it++;
  for (; it != stations.end(); it++) {
    ost << "," << *it;
  }

  ost << ")";

  LOGDEBUG3("typeidReceived: query:" << endl << ost.str());

  KvDbGateResult dbResult;
  LOGDEBUG("gate.exec dbResult called.");
  if (!gate.exec(dbResult, ost.str())) {
    LOGERROR(
        "Cant read the stationid/typeid for " << endl << "event: " << ost.str() << " obstime: " << pt::to_kvalobs_string(event.obstime()) << endl << "Reason: " << gate.getErrorStr());
    return false;
  }
  LOGDEBUG("RETURN gate.exec dbResult called.");

  if (dbResult.size() == 0)
    return false;

  int ssid;
  int stid;

  try {
    ost.str("");
    ost << "typeidReceived for station: " << event.stationInfo()->toIdentString() << " obstime: " << pt::to_kvalobs_string(event.obstime()) << endl
        << " (stationid/typeid):";

    for (KvDbGateResult::const_iterator it = dbResult.begin(); it != dbResult.end(); ++it) {

      ssid = it->at(0).asInt();
      stid = it->at(1).asInt();

      if (ssid == INT_MAX || stid == INT_MAX)
        continue;

      ost << " (" << ssid << "/" << stid << ")";
      event.addTypeidReceived(ssid, stid);
    }

    LOGINFO(ost.str());
  } catch (const std::exception &ex) {
    LOGERROR(
        "Fetch error: stationid/typeid for " << endl << "event: " << ost.str() << " obstime: " << pt::to_kvalobs_string(event.obstime()) << endl << "Reason: " << ex.what());

    //We clear the list. An empty list means "accept all", when the data
    //is used i BUFFER generation!
    event.clearTypeidReceived();
    return false;
  }

  return true;
}

void DataReceiver::setDefaultLogger(StationInfoPtr station) {
  try {
    FLogStream *logs = new FLogStream(1, 204800);  //200k
    std::ostringstream ost;
    string code(station->codeToString());

    boost::replace_all(code, " ", "_");
    //miutil::replaceStr( code," ", "_" );
    ost << kvPath("logdir") << "/" << app.options.progname << "/dr-" << station->toIdentString() << ".log";

    if (logs->open(ost.str())) {
      Logger::setDefaultLogger(logs);
      milog::LogLevel ll = station->loglevel();
      if (ll == milog::NOTSET)
        ll = milog::DEBUG;

      Logger::logger().logLevel(ll);
    } else {
      LOGERROR("Cant open the logfile <" << ost.str() << ">!");
      delete logs;
    }
  } catch (...) {
    LOGERROR("Cant create a logstream for: " << station->toIdentString() << " codes: " << station->codeToString());
  }
}
