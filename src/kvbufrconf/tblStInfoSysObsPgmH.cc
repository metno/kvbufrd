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
#include "boost/algorithm/string/replace.hpp"
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "tblStInfoSysObsPgmH.h"

using namespace std;
using namespace dnmi;
using boost::posix_time::to_kvalobs_string;
using boost::posix_time::time_from_string_nothrow;


void
TblStInfoSysObsPgmH::
createSortIndex()
{
   ostringstream ost;

   ost << stationid_ << paramid_ << message_formatid_;
   sortBy_ = ost.str();
}


bool
TblStInfoSysObsPgmH::
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

         if(*it=="stationid"){
            stationid_ = atoi( buf.c_str() );
         }else if(*it=="paramid"){
            paramid_ = atoi( buf.c_str() );
         }else if(*it=="hlevel"){
            hlevel_ = atoi( buf.c_str() );
         }else if(*it=="message_formatid"){
            message_formatid_ = atoi( buf.c_str() );
         }else if(*it=="nsensor"){
            nsensor_ = atoi( buf.c_str() );
         }else if(*it=="priority_message"){
            priority_message_ = buf=="t";
         }else if(*it=="anytime"){
            anytime_ = buf == "t";
         }else if(*it=="hour"){
            string h=buf;
            boost::replace_all( h, "}", "");
            boost::replace_all( h, "{", "");
            boost::replace_all( h, ",", "");
            boost::replace_all( h, "t", "1");
            boost::replace_all( h, "f", "0");
//            miutil::replace( h, "}", "");
//            miutil::replace( h, "{", "");
//            miutil::replace( h, ",", "");
//            miutil::replace( h, "t", "1");
//            miutil::replace( h, "f", "0");

            if( h.size() > 24 )
               h.erase( 24 );

            hour_ = std::bitset<24>( h );
         }else if(*it=="test"){
            test_ = buf=="t";
         }else if(*it=="totime"){
            totime_ = (buf.empty()?pt::ptime():time_from_string_nothrow( buf ));
         }else if(*it=="fromtime"){
            fromtime_ = (buf.empty()?pt::ptime():time_from_string_nothrow( buf ));
         }
      }
      catch(...){
         LOGWARN("TblStInfoSysObsPgmH: unexpected exception ..... \n");
         error = true;
      }
   }

   createSortIndex();
   return !error;
}

void
TblStInfoSysObsPgmH::
set(const TblStInfoSysObsPgmH &obspgmh )
{
   stationid_ = obspgmh.stationid_;
   paramid_ = obspgmh.paramid_;
   hlevel_ = obspgmh.hlevel_;
   message_formatid_ = obspgmh.message_formatid_;
   nsensor_ = obspgmh.nsensor_;
   priority_message_ = obspgmh.priority_message_;
   anytime_ = obspgmh.anytime_;
   hour_ = obspgmh.hour_;
   test_ = obspgmh.test_;
   totime_ = obspgmh.totime_;
   fromtime_ = obspgmh.fromtime_;
}

void
TblStInfoSysObsPgmH::
clean()
{
   stationid_ = kvDbBase::INT_NULL;
   paramid_ = kvDbBase::INT_NULL;
   hlevel_ = kvDbBase::INT_NULL;
   message_formatid_ = kvDbBase::INT_NULL;
   nsensor_ = kvDbBase::INT_NULL;
   priority_message_ = false;
   anytime_ = false;
   hour_ = hour_.reset();
   test_ = false;
   totime_ = pt::ptime();
   fromtime_ = pt::ptime();
}

std::string
TblStInfoSysObsPgmH::
uniqueKey() const
{
   ostringstream ost;

   ost << " WHERE stationid=" << stationid_ << " AND "
       << " paramid=" << paramid_ << " AND "
       << " message_formatid=" << message_formatid_;
   return ost.str();
}

std::ostream&
operator<<(std::ostream &o, TblStInfoSysObsPgmH &op )
{
   o << "[" << op.stationid_ << ","
     << op.paramid_ << ","
     << op.hlevel_ << ","
     << op.message_formatid_ << ","
     << op.nsensor_ << ","
     << (op.priority_message_?"t":"f") << ","
     << (op.anytime_?"t":"t") << ","
     << "(" << op.hour_ << "),"
     << (op.test_?"t":"f") << ","
     << "'" << (op.totime_.is_special()?"(NULL)":to_kvalobs_string(op.totime_)) << "',"
     << "'" << (op.fromtime_.is_special()?"(NULL)":to_kvalobs_string(op.fromtime_)) << "']";
   return o;
}
