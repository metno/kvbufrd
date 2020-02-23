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
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "tblStInfoSysNetworkStation.h"

using namespace std;
using namespace dnmi;

using boost::posix_time::to_kvalobs_string;
using boost::posix_time::time_from_string_nothrow;

void
TblStInfoSysNetworkStation::
createSortIndex()
{
   ostringstream ost;

   ost << stationid_;
   sortBy_ = ost.str();
}


bool
TblStInfoSysNetworkStation::
set( const dnmi::db::DRow &r_)
{
   db::DRow      &r=const_cast<db::DRow&>(r_);
   bool error=false;
   string        buf;
   list<string>  names=r.getFieldNames();
   list<string>::iterator it=names.begin();

   for(;it!=names.end(); it++){
      try{
         buf=r[*it];

         if( *it=="stationid" ){
            stationid_ = atoi( buf.c_str() );
         }else if( *it=="networkid" ){
            networkid_ = atoi( buf.c_str() );
         }else if(*it=="name") {
            name_=buf;
         }else if( *it=="external_stationcode") {
            externalStationcode_ = buf;
         }else if( *it=="comment") {
            comment_=buf;
         }else if( *it=="totime") {
            if( ! buf.empty() )
               toTime_ = time_from_string_nothrow( buf );
         }else if( *it=="fromtime") {
            fromTime_ = time_from_string_nothrow( buf );
         }
      }
      catch(...){
         LOGWARN("TblStInfoSysNetworkStation: unexpected exception ..... \n");
         error = true;
      }
   }

   if(fromTime_.is_special())
     error=true;

   createSortIndex();
   return !error;
}

bool
TblStInfoSysNetworkStation::
set(const TblStInfoSysNetworkStation &ns )
{
   stationid_ = ns.stationid_;
   networkid_ = ns.networkid_;
   name_ = ns.name_;
   externalStationcode_ = ns.externalStationcode_;
   comment_ = ns.comment_;
   toTime_ = ns.toTime_;
   fromTime_ = ns.fromTime_;
}

void
TblStInfoSysNetworkStation::
clean()
{
   stationid_ = INT_MAX;
   networkid_ = INT_MAX;
   name_.erase();
   externalStationcode_.erase();
   comment_.erase();
   toTime_ = pt::ptime();
   fromTime_ = pt::ptime();
}
std::string
TblStInfoSysNetworkStation::
uniqueKey() const
{
   ostringstream ost;

   ost << " WHERE stationid=" << stationid_
       <<   " AND networkid=" << networkid_
       <<   " AND fromtime=" << quoted( to_kvalobs_string(fromTime_) );
   return ost.str();
}

std::ostream&
operator<<( std::ostream &o, const TblStInfoSysNetworkStation &nt)
{
  o << "[" << nt.stationid_ << ","<< nt.networkid_<< "," << nt.name_ << ","
    << nt.externalStationcode_ << ",'" << nt.comment_ << "',"
    << (nt.fromTime_.is_special()?"(NULL)":to_kvalobs_string(nt.fromTime_)) << ","
    << (nt.toTime_.is_special()?"(NULL)":to_kvalobs_string(nt.toTime_)) << "]";

   return o;
}

