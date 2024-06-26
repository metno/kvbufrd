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

#include "tblStInfoSysStationOutmessage.h"
#include "miutil/timeconvert.h"
#include <float.h>
#include <limits.h>
#include <milog/milog.h>
#include <sstream>
#include <stdlib.h>

using namespace std;
using namespace dnmi;

namespace pt = boost::posix_time;

void
TblStInfoSysStationOutmessage::createSortIndex()
{
  ostringstream ost;

  ost << stationid_;
  sortBy_ = ost.str();
}

bool
TblStInfoSysStationOutmessage::set(const dnmi::db::DRow& r_)
{
  db::DRow& r = const_cast<db::DRow&>(r_);
  bool error = false;
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "stationid") {
        stationid_ = atoi(buf.c_str());
      } else if (*it == "productcoupling") {
        productcoupling_ = buf;
      } else if (*it == "coupling_delay") {
        coupling_delay_ = buf;
      } else if (*it == "priority_precip") {
        priority_precip_ = buf;
      } else if (*it == "fromtime") {
        fromTime_ =
          (buf.empty() ? pt::ptime() : pt::time_from_string_nothrow(buf));
      }
    } catch (...) {
      LOGWARN("TblStInfoSysStation: unexpected exception ..... \n");
      error = true;
    }
  }

  createSortIndex();
  return !error;
}

bool
TblStInfoSysStationOutmessage::set(const TblStInfoSysStationOutmessage& station)
{
  stationid_ = station.stationid_;
  productcoupling_ = station.productcoupling_;
  coupling_delay_ = station.coupling_delay_;
  priority_precip_ = station.priority_precip_;
  fromTime_ = station.fromTime_;
  return true;
}

void
TblStInfoSysStationOutmessage::clean()
{
  stationid_ = INT_MAX;
  productcoupling_.erase();
  coupling_delay_.erase();
  priority_precip_.erase();
  fromTime_ = pt::ptime();
}

#ifdef __WITH_PUTOOLS__
miutil::miString
#else
std::string
#endif
TblStInfoSysStationOutmessage::uniqueKey() const
{
  ostringstream ost;

  ost << " WHERE stationid=" << stationid_;
  return ost.str();
}
