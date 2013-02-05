/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: tblWaiting.cc,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include "tblWaiting.h"
#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi;

void 
TblWaiting::
createSortIndex() 
{
   using namespace boost;
   sortBy_=lexical_cast<string>(wmono_)+lexical_cast<string>(id_)+
         callsign_ + code_ +
         obstime_.isoTime()+delaytime_.isoTime();
}

void 
TblWaiting::
clean()
{
   wmono_     = 0;
   id_        = 0;
   callsign_.erase();
   code_.erase();
   obstime_   = miTime::nowTime();
   delaytime_ = miTime::nowTime();

   createSortIndex();
}


bool 
TblWaiting::
set(const dnmi::db::DRow &r_)
{
   db::DRow               &r=const_cast<db::DRow&>(r_);
   string                 buf;
   list<string>           names=r.getFieldNames();
   list<string>::iterator it=names.begin();

   for(;it!=names.end(); it++){
      try{
         buf=r[*it];

         if(*it=="wmono"){
            wmono_=atoi(buf.c_str());
         }else if(*it=="id"){
            id_=atoi(buf.c_str());
         }else if( *it=="callsign" ) {
            callsign_ = buf;
         }else if( *it=="code" ) {
            code_ = buf;
         }else if(*it=="obstime"){
            obstime_=miTime(buf);
         }else if(*it=="delaytime"){
            delaytime_=miTime(buf);
         }else{
            LOGWARN("TblWaiting::set .. unknown entry:" << *it << std::endl);
         }
      }
      catch(...){
         LOGWARN("TblWaiting: unexpected exception ..... \n");
      }
   }

   createSortIndex();
   return true;
}

bool 
TblWaiting::
set(const TblWaiting &s)
{
   wmono_      = s.wmono_;
   id_         = s.id_;
   callsign_   = s.callsign_;
   code_       = s.code_;
   obstime_    = s.obstime_;
   delaytime_ = s.delaytime_;

   createSortIndex();

   return true;
}



bool 
TblWaiting::
set(int                  wmono, 
    int                  id,
    const std::string    &callsign,
    const std::string    &code,
    const miutil::miTime &obtime,    
    const miutil::miTime &delaytime)
{
   wmono_      = wmono;
   id_         = id;
   callsign_   = callsign;
   code_       = code;
   obstime_    = obtime;
   delaytime_ = delaytime;

   createSortIndex();

   return true;
}

#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblWaiting::
toSend() const
{
   ostringstream ost;

   ost << "("
         << wmono_             << ","
         << id_                << ","
         << quoted( callsign_ )<< ","
         << quoted( code_ )    << ","
         << quoted( obstime_.isoTime() )   << ","
         << quoted( delaytime_.isoTime() )
         << ")";

   return ost.str();
}

#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblWaiting::
uniqueKey()const
{
   ostringstream ost;

   ost << " WHERE wmono=" << wmono_ << " AND "
       << "          id=" << id_ << " AND "
       << "    callsign=" << quoted(callsign_ ) << " AND "
       << "        code=" << quoted(code_ ) << " AND "
       << "   obstime=" << quoted( obstime_.isoTime() );

   return ost.str();
}


#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblWaiting::
toUpdate()const
{
   ostringstream ost;

   ost << "SET delaytime=" << quoted( delaytime_.isoTime() )
            << " WHERE   wmono=" << wmono_ << " AND "
            << "            id=" << id_ << " AND "
            << "      callsign=" << quoted(callsign_ ) << " AND "
            << "          code=" << quoted(code_ ) << " AND "
            << "       obstime=" << quoted( obstime_.isoTime());

   return ost.str();
}
