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
#include "tblStInfoSysParam.h"

using namespace std;
using namespace dnmi;

void
TblStInfoSysParam::
createSortIndex()
{
   ostringstream ost;

   ost << paramid_;
   sortBy_ = ost.str();
}


bool
TblStInfoSysParam::
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

         if(*it=="paramid"){
            paramid_ = atoi( buf.c_str() );
         }else if(*it=="name"){
            name_=buf;
         }else if(*it=="hlevel_scale"){
            hlevel_scale_ = atoi( buf.c_str() );
         }else if(*it=="standard_hlevel"){
            standard_hlevel_ = atoi( buf.c_str() );
         }else if(*it=="standard_physical_height"){
            standard_physical_height_ = atoi( buf.c_str() );
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
TblStInfoSysParam::
set(const TblStInfoSysParam &param )
{
   paramid_ = param.paramid_;
   name_ = param.name_;
   hlevel_scale_ = param.hlevel_scale_;
   standard_hlevel_ = param.standard_hlevel_;
   standard_physical_height_ = param.standard_physical_height_;
   return true;
}

void
TblStInfoSysParam::
clean()
{
   paramid_ = INT_MAX;
   name_.erase();
   hlevel_scale_ = INT_MAX;
   standard_hlevel_ = INT_MAX;
   standard_physical_height_ = INT_MAX;
}

#ifdef __WITH_PUTOOLS__
miutil::miString
#else
std::string
#endif
TblStInfoSysParam::
uniqueKey() const
{
   ostringstream ost;

   ost << " WHERE paramid=" << paramid_;
   return ost.str();
}
