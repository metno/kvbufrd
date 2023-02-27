/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblKeyVal.h,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $

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
#include "tblStInfoSysWigosStation.h"
#include "../dbhelper.h"
#include "miutil/timeconvert.h"
#include <boost/lexical_cast.hpp>
#include <float.h>
#include <limits.h>
#include <milog/milog.h>
#include <sstream>
#include <stdlib.h>

using namespace std;
using namespace dnmi;
namespace pt=boost::posix_time;

void
TblStInfoSysWigosStation::createSortIndex()
{
  ostringstream ost;

  ost << stationid_;
  sortBy_ = ost.str();
}

bool
TblStInfoSysWigosStation::set(const dnmi::db::DRow& r_)
{
  db::DRow& r = const_cast<db::DRow&>(r_);
  bool error = false;
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];
      if (*it == "wigosid") {
        wigosid_ = buf;
      } else if (*it == "stationid") {
        stationid_ = dbhelper::toInt(buf);
      } else if (*it == "lat") {
        lat_ = dbhelper::toFloat(buf);
      } else if (*it == "lon") {
        lon_ = dbhelper::toFloat(buf);
      } else if (*it == "hs") {
        hs_ = dbhelper::toInt(buf);
      } else if (*it == "hv") {
        hv_ = dbhelper::toInt(buf);
      } else if (*it == "hp") {
        hp_ = dbhelper::toFloat(buf);
      } else if (*it == "maxspeed") {
        maxspeed_ = dbhelper::toFloat(buf);
      } else if (*it == "name") {
        name_ = buf;
      } else if (*it == "short_name") {
        shortName_ = buf;
      } else if (*it == "wmono") {
        wmono_ = dbhelper::toInt(buf);
      } else if (*it == "fromtime") {
        from_ = (buf.empty() ? pt::ptime() : pt::time_from_string_nothrow(buf));
      }
    } catch (...) {
      LOGWARN("TblStInfoSysWigosStation: unexpected exception ..... \n");
      error = true;
    }
  }

  createSortIndex();
  return !error;
}

bool
TblStInfoSysWigosStation::set(const TblStInfoSysWigosStation& station)
{
  wigosid_ = station.wigosid_;
  stationid_ = station.stationid_;
  lat_ = station.lat_;
  lon_ = station.lon_;
  hs_ = station.hs_;
  hv_ = station.hv_;
  hp_ = station.hp_;
  maxspeed_ = station.maxspeed_;
  name_ = station.name_;
  shortName_ = station.shortName_;
  wmono_ = station.wmono_;
  from_ = station.from_;
  return true;
}

void
TblStInfoSysWigosStation::clean()
{
  wigosid_.erase();
  stationid_ = INT_NULL;
  lat_ = FLT_NULL;
  lon_ = FLT_NULL;
  hs_ = INT_NULL;
  hv_ = INT_NULL;
  hp_ = FLT_NULL;
  maxspeed_ = FLT_NULL;
  name_.erase();
  shortName_.erase();
  wmono_ = INT_NULL;
  from_ = pt::ptime();
}

// #ifdef __WITH_PUTOOLS__
// miutil::miString
// #else
std::string
//#endif
TblStInfoSysWigosStation::uniqueKey() const
{
  ostringstream ost;

  ost << " WHERE stationid=" << stationid_;
  return ost.str();
}
