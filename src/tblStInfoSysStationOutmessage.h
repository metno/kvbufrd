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

#ifndef __tblStInfoSysStationOutmessage_h__
#define __tblStInfoSysStationOutmessage_h__

#include <puTools/miTime.h>
#include <kvalobs/kvDbBase.h>

class TblStInfoSysStationOutmessage : public kvalobs::kvDbBase {
   int stationid_;
   std::string productcoupling_;
   std::string coupling_delay_;
   std::string priority_precip_;
   miutil::miTime fromTime_;

  void createSortIndex();

public:
  TblStInfoSysStationOutmessage() {clean();}
  TblStInfoSysStationOutmessage(const TblStInfoSysStationOutmessage &station){ set( station );}
  TblStInfoSysStationOutmessage(const dnmi::db::DRow &r){set(r);}

  bool set(const dnmi::db::DRow &r);
  bool set(const TblStInfoSysStationOutmessage &station );

  TblStInfoSysStationOutmessage& operator=(const TblStInfoSysStationOutmessage &station ){
                  if( &station != this)
                     set( station );
                  return *this;
                }

  void clean();

  const char* tableName() const {return "station_outmessage";}

  miutil::miString toSend()    const { return ""; } //NOT used
  miutil::miString toUpdate()  const{ return ""; }  //NOT used
  miutil::miString uniqueKey() const;

  std::string couplingDelay() const { return coupling_delay_; }
  std::string priorityPrecip() const { return priority_precip_; }
  std::string productcoupling() const { return productcoupling_; }
  int stationid() const { return stationid_; }
  miutil::miTime fromTime()const{ return fromTime_; }
};

#endif
