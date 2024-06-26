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

#include <limits.h>
#include <sstream>
#include <stdlib.h>
#include <milog/milog.h>
#include "../dbhelper.h"
#include "tblStInfoSysSensorInfo.h"

using namespace std;
using namespace dnmi;

/*class TblStInfoSysSensorInfo : public kvalobs::kvDbBase {
   int  equipmentid_;
   int  paramgroupid_;
   int  stationid_;
   int  sensor_;
   int  hlevel_;
   bool operational_;
   int  physical_height_;
   int  measurement_methodid_;
*/
void
TblStInfoSysSensorInfo::
createSortIndex()
{
   ostringstream ost;

   ost << equipmentid_ <<  paramgroupid_<< stationid_
       << sensor_<<   hlevel_ ;
   sortBy_ = ost.str();

}


bool
TblStInfoSysSensorInfo::
set(const dnmi::db::DRow &r_)
{
   db::DRow      &r=const_cast<db::DRow&>(r_);
   bool error=false;
   string        buf;
   list<string>  names=r.getFieldNames();
   list<string>::iterator it=names.begin();

   for(;it!=names.end(); it++){
      try{
         buf=r[*it];

         if(*it=="equipmentid"){
            equipmentid_ = dbhelper::toInt( buf );
         }else if(*it=="paramgroupid"){
            paramgroupid_ = dbhelper::toInt( buf );
         }else if(*it=="stationid"){
            stationid_ = dbhelper::toInt( buf );
         }else if(*it=="sensor"){
            sensor_ = dbhelper::toInt( buf );
         }else if(*it=="hlevel"){
            hlevel_ = dbhelper::toInt( buf );
         }else if(*it=="operational"){
            operational_= (!buf.empty() && ( buf[0]=='t' || buf[0]=='T'))?true:false;
         }else if(*it=="physical_height"){
            physical_height_ = dbhelper::toInt( buf );
         }else if(*it=="measurement_methodid"){
            measurement_methodid_ = dbhelper::toInt( buf );
         }
      }
      catch(...){
         LOGWARN("TblKeyVal: unexpected exception ..... \n");
         error = true;
      }
   }

   createSortIndex();
   return !error;

}
 bool
 TblStInfoSysSensorInfo::
 set(const TblStInfoSysSensorInfo &info )
 {
    equipmentid_ = info.equipmentid_;
    paramgroupid_ = info.paramgroupid_;
    stationid_ = info.stationid_;
    sensor_ = info.sensor_;
    hlevel_ = info.hlevel_;
    operational_ = info.operational_;
    physical_height_ = info.physical_height_;
    measurement_methodid_ = info.measurement_methodid_;
   return true;
 }

 void
 TblStInfoSysSensorInfo::
 clean()
 {
    equipmentid_ = INT_NULL;
    paramgroupid_ = INT_NULL;
    stationid_ = INT_NULL;
    sensor_ = INT_NULL;
    hlevel_ = INT_NULL;
    operational_ = false;
    physical_height_ = INT_NULL;
    measurement_methodid_ = INT_NULL;
 }

#ifdef __WITH_PUTOOLS__
 miutil::miString
#else
 std::string
#endif
 TblStInfoSysSensorInfo::
 uniqueKey() const
 {
    ostringstream ost;

    ost << " WHERE paramgroupid=" << paramgroupid_
        << " AND equipmentid=" << equipmentid_
        << " AND stationid=" << stationid_
        << " AND sensor=" << sensor_
        << " AND hlevel=" << hlevel_;
    return ost.str();
 }

