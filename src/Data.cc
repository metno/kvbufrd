/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: Data.cc,v 1.3.6.3 2007/09/27 09:02:22 paule Exp $

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
#include <stdio.h>
#include <sstream>
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "Data.h"

namespace pt = boost::posix_time;
using namespace std;
using namespace dnmi;

void Data::createSortIndex() {
  stringstream s;
  s << stationid_ << paramid_ << sensor_ << level_ << obstime_;
  sortBy_ = s.str();
}

void Data::clean() {
  stationid_ = 0;
  obstime_ = pt::second_clock::universal_time();
  original_.erase();
  paramid_ = 0;
  typeid_ = 0;
  sensor_ = 0;
  level_ = 0;

  createSortIndex();
}

bool Data::set(const kvalobs::kvData &data) {
  char buf[100];
  tbtime_ = pt::second_clock::universal_time();
  sprintf(buf, "%.2f", data.original());

  set(data.stationID(), data.obstime(), buf, data.paramID(), data.typeID(), data.sensor(), data.level(), data.controlinfo().flagstring(),
      data.useinfo().flagstring());

  return true;
}

bool Data::set(const dnmi::db::DRow &r_) {
  db::DRow &r = const_cast<db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "stationid") {
        stationid_ = atoi(buf.c_str());
      } else if (*it == "obstime") {
        obstime_ = pt::time_from_string_nothrow(buf);
      } else if (*it == "tbtime") {
        tbtime_ = pt::time_from_string_nothrow(buf);
      } else if (*it == "original") {
        original_ = buf;
      } else if (*it == "paramid") {
        paramid_ = atoi(buf.c_str());
      } else if (*it == "typeid") {
        typeid_ = atoi(buf.c_str());
      } else if (*it == "sensor") {
        sensor_ = atoi(buf.c_str());
      } else if (*it == "level") {
        level_ = atoi(buf.c_str());
      } else if (*it == "controlinfo") {
        controlinfo_ = buf;
      } else if (*it == "useinfo") {
        useinfo_ = buf;
      } else {
        LOGWARN("Data::set .. unknown entry:" << *it << std::endl);
      }
    } catch (...) {
      LOGWARN("Data: unexpected exception ..... \n");
    }
  }

  createSortIndex();
  return true;
}

bool Data::set(int pos, const pt::ptime &obt, const std::string &org, int par, int typ, int sen, int lvl, const std::string &controlinfo,
               const std::string &useinfo) {
  tbtime_ = pt::second_clock::universal_time();
  stationid_ = pos;
  obstime_ = obt;
  original_ = org;
  paramid_ = par;
  typeid_ = typ;
  sensor_ = sen;
  level_ = lvl;
  controlinfo_ = controlinfo;
  useinfo_ = useinfo;

  createSortIndex();

  return true;
}

std::string
Data::toSend() const {
  ostringstream ost;
  pt::ptime now = pt::second_clock::universal_time();;

  ost << "(" << stationid_ << "," << quoted(pt::to_kvalobs_string(obstime_)) << "," << quoted(pt::to_kvalobs_string(now)) << "," << quoted(original_) << "," << paramid_ << "," << typeid_
      << "," << sensor_ << "," << level_ << "," << quoted(controlinfo_) << "," << quoted(useinfo_) << ")";

  return ost.str();
}

std::string
Data::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE stationid=" << stationid_ << " AND " << "       obstime=" << quoted(pt::to_kvalobs_string(obstime_)) << " AND " << "       paramid=" << paramid_ << " AND "
      << "       typeid=" << typeid_ << " AND " << "       sensor=" << sensor_ << " AND " << "       level=" << level_;

  return ost.str();
}

std::string
Data::toUpdate() const {
  ostringstream ost;
  pt::ptime now = pt::second_clock::universal_time();

  ost << "SET original=" << quoted(original_) << "    ,controlinfo=" << quoted(controlinfo_) << "    ,useinfo=" << quoted(useinfo_) << "    ,tbtime="
      << quoted(pt::to_kvalobs_string(now)) << " WHERE stationid=" << stationid_ << " AND " << "       obstime=" << quoted(pt::to_kvalobs_string(obstime_)) << " AND "
      << "       paramid=" << paramid_ << " AND " << "       typeid=" << typeid_ << " AND " << "       sensor=" << quoted(sensor_) << " AND " << "       level="
      << level_;

  return ost.str();
}

std::list<Data> kvDataToData( const std::list<kvalobs::kvData> &data){
  std::list<Data> ret;
  for( auto &d : data )
    ret.push_back(Data(d));
  return ret;
}

kvalobs::kvControlInfo Data::controlinfo() const {
  if (controlinfo_.length() != kvalobs::kvDataFlag::size)
    return kvalobs::kvControlInfo();

  return kvalobs::kvControlInfo(controlinfo_);
}

kvalobs::kvUseInfo Data::useinfo() const {
  if (useinfo_.length() != kvalobs::kvDataFlag::size)
    return kvalobs::kvUseInfo();

  return kvalobs::kvUseInfo(useinfo_);
}


DataInsertCommand::DataInsertCommand(const std::list<Data> &dl, const std::string &logid)
  : dl_(dl),logid_(logid) {
}

std::string DataInsertCommand::getError()const {
  return err_.str();
}

//CREATE TABLE data (stationid integer, obstime timestamp, tbtime timestamp 
//  DEFAULT CURRENT_TIMESTAMP, original text, paramid integer, typeid integer, 
//  sensor integer, level integer, controlinfo text, useinfo text, 
// UNIQUE(stationid, obstime, paramid, typeid, level, sensor)
//)

// INSERT INTO data 
// (stationid, obstime, tbtime,original, paramid, typeid, sensor, level, controlinfo, useinfo ) 
// VALUES(18269, '2020-03-11 16:00:00', '2020-03-11 16:37:46', '7.60', 211, 504, 0, 0, '0111100000100010', '7000000000000000') 
// ON CONFLICT(stationid, obstime, paramid, typeid, level, sensor) 
// DO UPDATE SET original=excluded.original, tbtime='2020-03-11 16:37:46' WHERE original<>excluded.original;



// (" << stationid_ << "," 
// << quoted(pt::to_kvalobs_string(obstime_)) << "," << quoted(pt::to_kvalobs_string(now)) 
// << "," << quoted(original_) << "," << paramid_ << "," << typeid_
// << "," << sensor_ << "," << level_ << "," << quoted(controlinfo_) << "," << quoted(useinfo_) << ")";

namespace {
  std::string Q(const pt::ptime &t) {
    return "'" + pt::to_kvalobs_string(t) + "'";
  }
  std::string Q(const std::string &s) {
    return "'" + s + "'";
  }
}


bool DataInsertCommand::doExec( dnmi::db::Connection *con) {
  if( ! con ) {
    std::cerr << "DataInsertCommand::doExec: No connection\n";
    err_<< "DataInsertCommand::doExec: No connection";
    LOGERROR("DataInsertCommand::doExec: No connection");
    if( !logid_.empty()) {
      IDLOGERROR(logid_,"NO CONNECTION.");
    }
    return false;
  }

  bool ok=true;
  ostringstream q;
  pt::ptime now=pt::second_clock::universal_time();
  ostringstream log;
  for( const auto &d : dl_ ) {
    q.str("");
    q <<"INSERT INTO data (stationid, obstime, tbtime,original, paramid, typeid, sensor, level, controlinfo, useinfo ) VALUES("
      << d.stationID() << ", " << Q(d.obstime()) << ", " << Q(now) << ", " << Q(d.original()) << ", " 
      << d.paramID() << ", " << d.typeID()<< ", " << d.sensor() << ", " << d.level() << ", " 
      << Q(d.controlinfo().flagstring()) << ", " << Q(d.useinfo().flagstring())<<")"
      << " ON CONFLICT(stationid, obstime, paramid, typeid, level, sensor) DO"
      << " UPDATE SET original=excluded.original, controlinfo=excluded.controlinfo, useinfo=excluded.useinfo, tbtime=" << Q(now)
      << " WHERE original<>excluded.original OR controlinfo<>excluded.controlinfo OR useinfo<>excluded.useinfo;";

    try {
      unique_ptr<dnmi::db::Result> r(con->execQuery(q.str()));
      log << d << endl;
      if( r.get() ) {
        ostringstream o;
        while( r->hasNext() ) {
          o.str("");
          auto v=r->next();
          auto n=v.fields();
          for( int i=0; i<n; ++i) {
            o << v.fieldName(i) <<"(" << v[i]<< "), ";
          }
          cerr << "doExec: res: "<<o.str() << "\n";
          LOGINFO("doExec: " << o.str());
          if(!logid_.empty()) {
            IDLOGINFO(logid_, "doExec: " << o.str());
          }
        }
      }
    }
    catch( const std::exception &e) {
      ok=false;
      err_ << e.what() << "\n";
      cerr << "doExec: exception: " << e.what() << "\n" << "  Q: '" << q.str() << "'\n";
      LOGERROR("doExec: exception: " << e.what());
      if(!logid_.empty()) {
        IDLOGINFO(logid_, "doExec: Exception: " << e.what());
      }
    } 
    catch ( ... ) {
      cerr << "doExec: UNKNOWN exception\n" << "  Q: '" << q.str() << "'\n";
      LOGERROR("doExec: UNKOWN exception." );
      if(!logid_.empty()) {
        IDLOGINFO(logid_, "doExec: UNKNOWN Exception.");
      }
    }
  }

  if(!logid_.empty()) {
    IDLOGINFO(logid_, "doExec: inserted/updated data:" << endl << log.str() );
  }
  return ok;
}


std::ostream&
operator<<(std::ostream& ost, const Data& data) {
  ost << "[" << data.stationid_ << ", " << data.typeid_ << ", " << pt::to_kvalobs_string(data.obstime_) << ", " << data.paramid_ <<  ", " << data.original_ 
      << ", " << data.sensor_ << ", " << data.level_ << ", " << data.controlinfo_ << ", " << data.useinfo_ << ", " << pt::to_kvalobs_string(data.tbtime_) << "]";
  return ost;
}

std::ostream& operator<<( std::ostream& ost,
						  const std::list<Data>& dl ) {
  for( auto &e : dl) {
    ost << e << std::endl;
  }
  return ost;
}


DataKey::DataKey(const kvalobs::kvData &data)
    : stationid_(data.stationID()),
      typeid_(data.typeID()),
      paramid_(data.paramID()),
      sensor_(data.sensor()),
      level_(data.level()),
      obstime_(data.obstime()) {

}

bool DataKey::operator<(const DataKey &rhs) const {
  return (stationid_ < rhs.stationid_) 
      || (stationid_ == rhs.stationid_ && typeid_ < rhs.typeid_)
      || (stationid_ == rhs.stationid_ && typeid_ == rhs.typeid_ && paramid_ < rhs.paramid_)
      || (stationid_ == rhs.stationid_ && typeid_ == rhs.typeid_ && paramid_ == rhs.paramid_ && sensor_ < rhs.sensor_)
      || (stationid_ == rhs.stationid_ && typeid_ == rhs.typeid_ && paramid_ == rhs.paramid_ && sensor_ == rhs.sensor_ && level_ < rhs.level_)
      || (stationid_ == rhs.stationid_ && typeid_ == rhs.typeid_ && paramid_ == rhs.paramid_ && sensor_ == rhs.sensor_ && level_ == rhs.level_
          && obstime_ < rhs.obstime_);
}

bool DataKeySet::add(const DataKey &key) {
  pair<set<value_type>::iterator, bool> ret;
  ret = insert(key);
  return ret.second;
}

