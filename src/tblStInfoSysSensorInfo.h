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
#ifndef __tblStInfoSysSensorInfo_h__
#define __tblStInfoSysSensorInfo_h__

#include <kvalobs/kvDbBase.h>

class TblStInfoSysSensorInfo : public kvalobs::kvDbBase {
   int equipmentid_;
   int paramid_;
   int stationid_;
   int sensor_;
   int hlevel_;
   bool operational_;
   int physical_height_;
   int measurement_methodid_;

  void createSortIndex();

public:
  TblStInfoSysSensorInfo() {clean();}
  TblStInfoSysSensorInfo(const TblStInfoSysSensorInfo &info){ set( info );}
  TblStInfoSysSensorInfo(const dnmi::db::DRow &r){set(r);}

  bool set(const dnmi::db::DRow&);
  bool set(const TblStInfoSysSensorInfo &param );

  TblStInfoSysSensorInfo& operator=(const TblStInfoSysSensorInfo &info ){
                  if( &info != this)
                     set(info);
                  return *this;
                }

  void clean();

  const char* tableName() const {return "sensor_info";}

  miutil::miString toSend()    const { return ""; } //NOT used
  miutil::miString toUpdate()  const{ return ""; }  //NOT used
  miutil::miString uniqueKey() const;

  int equipmentid()const { return equipmentid_; }
  int paramid() const { return paramid_; }
  int stationid() const { return stationid_; }
  int sensor() const { return sensor_; }
  int hlevel() const { return hlevel_; }
  bool operational() const { return operational_; }
  int physical_height() const { return physical_height_; }
  int measurement_methodid() const { return measurement_methodid_; }
};

#endif
