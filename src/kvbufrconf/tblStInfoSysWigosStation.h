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

#ifndef __tblStInfoSysWigosStation_h__
#define __tblStInfoSysWigosStation_h__

#include "boost/date_time/posix_time/ptime.hpp"
#include <kvalobs/kvDbBase.h>

class TblStInfoSysWigosStation : public kvalobs::kvDbBase
{
  /*
   wigosid    | text                        |           |          |
   stationid  | integer                     |           |          |
   lat        | double precision            |           |          |
   lon        | double precision            |           |          |
   hs         | integer                     |           |          |
   hv         | integer                     |           |          |
   hp         | double precision            |           |          |
   maxspeed   | double precision            |           |          |
   name       | text                        |           |          |
   short_name | character varying(32)       |           |          |
   wmono      | integer                     |           |          |
   fromtime   | timestamp without time zone |           |          |
  */

  std::string wigosid_;
  int stationid_;
  float lat_;
  float lon_;
  int hs_;
  int hv_;
  float hp_;
  float maxspeed_;
  std::string name_;
  std::string shortName_;
  int wmono_;
  boost::posix_time::ptime from_;

  void createSortIndex();

public:
  TblStInfoSysWigosStation() { clean(); }
  TblStInfoSysWigosStation(const TblStInfoSysWigosStation& station)
  {
    set(station);
  }
  TblStInfoSysWigosStation(const dnmi::db::DRow& r) { set(r); }

  bool set(const dnmi::db::DRow& r);
  bool set(const TblStInfoSysWigosStation& param);

  TblStInfoSysWigosStation& operator=(const TblStInfoSysWigosStation& info)
  {
    if (&info != this)
      set(info);
    return *this;
  }

  void clean();

  const char* tableName() const { return "wigos_station"; }
  // #ifdef __WITH_PUTOOLS__
  //   miutil::miString toSend() const { return ""; }   // NOT used
  //   miutil::miString toUpdate() const { return ""; } // NOT used
  //   miutil::miString uniqueKey() const;
  // #else
  std::string toSend() const { return ""; }   // NOT used
  std::string toUpdate() const { return ""; } // NOT used
  std::string uniqueKey() const;
  // #endif

  std::string wigosid() const { return wigosid_; }
  int stationid() const { return stationid_; }
  float lat() const { return lat_; }
  float lon() const { return lon_; }
  int hs() const { return hs_; }
  int hv() const { return hv_; }
  float hp() const { return hp_; }
  float maxspeed() const { return maxspeed_; }
  std::string name() const { return name_; }
  std::string shortName() const { return shortName_; }
  int wmono() const { return wmono_; }
  boost::posix_time::ptime fromTime() const { return from_; }
};

#endif
