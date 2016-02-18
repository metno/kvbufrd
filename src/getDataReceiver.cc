/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: getDataReceiver.cc,v 1.1.2.6 2007/09/27 09:02:23 paule Exp $

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
#include <sstream>
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "StationInfo.h"
#include "getDataReceiver.h"
#include "Data.h"
#include "obsevent.h"

using namespace std;
using namespace milog;
using namespace kvservice;
namespace pt=boost::posix_time;


void GetKvDataReceiver::Current::setCurrent(long stationid, const pt::ptime &obstime )
{
  if( stationid_==-1 || ! (stationid_ == stationid && obstime_ == obstime )){
    postEvents();
    stationid_ = stationid;
    obstime_ = obstime;
  }
}

void GetKvDataReceiver::Current::postEvents(){
  for( auto &e : obsEvents_ ){
    if (parent_->logid.empty()) {
      LOGDEBUG("Postevent: station_id=" << e.first << " obstime=" << pt::to_kvalobs_string(e.second->obstime()));
    } else {
      IDLOGDEBUG(parent_->logid, "Postevent: station_id=" << e.first << " obstime=" << pt::to_kvalobs_string(e.second->obstime()));
    }
    parent_->app.addObsEvent( e.second, *parent_->que);
  }
  obsEvents_.clear();
}


ObsEvent* GetKvDataReceiver::Current::getObsEvent( StationInfoPtr station){
  string id=station->toIdentString();
  auto it=obsEvents_.find(id);

  if(it == obsEvents_.end())
    obsEvents_[id]=new ObsEvent(obstime_,station);

  return obsEvents_[id];
}

GetKvDataReceiver::Current::~Current(){
  postEvents();
}

bool validate(const std::string &logid, long stationid, long typeId, const boost::posix_time::ptime &obstime_, const kvalobs::ObsDataElement &data_){
  const auto &d = get<0>(data_);
  set<string> e;
  ostringstream ost;
  ostringstream tmp;

  ost << "Validate: " << stationid << ", " << typeId << ", " << pt::to_kvalobs_string(obstime_) << endl;

  for( auto &it: d ){
    if( !(it.stationID() ==stationid && it.typeID()==typeId && it.obstime()==obstime_)) {
      tmp.str("");
      tmp << it.stationID() <<", " << it.typeID() << ", " << pt::to_kvalobs_string(it.obstime());
      auto in = e.insert(tmp.str());
      if( in.second )
        ost << "   " << tmp.str() << endl;
    }
  }

  if( ! e.empty() ) {
    IDLOGERROR(logid, ost.str());
    return false;
  }

  return true;
}

void GetKvDataReceiver::next(long stationid, long typeId, const boost::posix_time::ptime &obstime_, const kvalobs::ObsDataElement &data_) {
  int nObs = 0;
  LogContext lContext("GetKvDataReceiver");
  const auto &data = kvDataToData(get<0>(data_));

  IDLOGDEBUG(logid,"next: " << stationid << ", " << typeId << ", " << pt::to_kvalobs_string(obstime_) << " d#" << get<0>(data_).size() << " td#" << get<1>(data_).size());

  if( !validate(logid, stationid, typeId, obstime_, data_)) {
    return;
  }

  current.setCurrent(stationid, obstime_);

  StationInfoList stations = app.findStationInfo(stationid, typeId);

  if( stations.empty() || data.empty() )
    return;


  if (!gate.insert(data, true)) {
    LOGERROR("Cant insert data: \n  stationid: " << stationid << " typeid: " << typeId << " obstime: " << pt::to_kvalobs_string(obstime_) << "  reason: " << gate.getErrorStr() << endl);
    IDLOGERROR(logid, "Cant insert data: \n  stationid: " << stationid<< " typeid: " << typeId << " obstime: " << pt::to_kvalobs_string(obstime_) << "  reason: " << gate.getErrorStr() << endl);
    return;
  } else {
    IDLOGDEBUG(logid, "stationid: " << stationid << " typeid: " <<typeId << " obstime: " << pt::to_kvalobs_string(obstime_) << "  inserted: #" << data.size() << " params!" <<endl);
  }

  for( auto station : stations ){
    ObsEvent *event = current.getObsEvent( station);

    if( !event ) {
      if (!logid.empty()) {
        IDLOGERROR(logid, "Failed to create a ObsEvent. Stationid: " << station->stationID() << " obstime: " << pt::to_kvalobs_string(obstime_) );
      }
      continue;
    }

    event->addTypeidReceived(stationid, typeId);
  }
}
