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

#ifndef __tblStInfoSysStation_h__
#define __tblStInfoSysStation_h__

#include <kvalobs/kvDbBase.h>

class TblStInfoSysStation : public kvalobs::kvDbBase {
   int stationid_;
   float lat_;
   float lon_;
   int   hs_;
   int   hv_;
   int   hp_;
   std::string name_;
   int   wmono_;

  void createSortIndex();

public:
  TblStInfoSysStation() {clean();}
  TblStInfoSysStation(const TblStInfoSysStation &station){ set( station );}
  TblStInfoSysStation(const dnmi::db::DRow &r){set(r);}

  bool set(const dnmi::db::DRow &r);
  bool set(const TblStInfoSysStation &param );

  TblStInfoSysStation& operator=(const TblStInfoSysStation &info ){
                  if( &info != this)
                     set(info);
                  return *this;
                }

  void clean();

  const char* tableName() const {return "station";}

  miutil::miString toSend()    const { return ""; } //NOT used
  miutil::miString toUpdate()  const{ return ""; }  //NOT used
  miutil::miString uniqueKey() const;

  int hp() const { return hp_; }
  int hs() const { return hs_; }
  int hv() const { return hv_; }
  float lat() const { return lat_; }
  float lon() const { return lon_; }
  std::string name() const { return name_; }
  int stationid() const { return stationid_; }
  int wmono() const { return wmono_; }

};

#endif
