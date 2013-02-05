/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: tblBufr.cc,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $

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
#include "tblBufr.h"
#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi;

void 
TblBufr::
createSortIndex() 
{
   using namespace boost;
   sortBy_=lexical_cast<string>(wmono_)+lexical_cast<string>(id_)
         + callsign_ + code_
         + obstime_.isoTime()+createtime_.isoTime()
         + lexical_cast<string>(ccx_);
}

void 
TblBufr::
clean()
{
   wmono_      = 0;
   id_         = 0;
   callsign_.erase();
   code_.erase();
   obstime_    = miTime::nowTime();
   createtime_ = miTime::nowTime();
   crc_        = 0;
   ccx_        = 0;
   data_.erase();
   bufrBase64_.erase();

   createSortIndex();
}


bool 
TblBufr::
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
         } else if( *it=="id" ) {
            id_ = atoi(buf.c_str());
         } else if( *it=="callsign" ) {
            callsign_ = buf;
         }else if( *it=="code" ) {
            code_ = buf;
         }else if(*it=="obstime"){
            obstime_=miTime(buf);
         }else if(*it=="createtime"){
            createtime_=miTime(buf);
         }else if(*it=="crc"){
            crc_=atoi(buf.c_str());
         }else if(*it=="ccx"){
            ccx_=atoi(buf.c_str());
         }else if(*it=="data"){
            data_=buf;
         }else if(*it=="bufrBase64"){
            bufrBase64_=buf;
         }else{
            LOGWARN("TblBufr::set .. unknown entry:" << *it << std::endl);
         }
      }
      catch(...){
         LOGWARN("TblBufr: unexpected exception ..... \n");
      }
   }

   createSortIndex();
   return true;
}

bool 
TblBufr::
set(const TblBufr &s)
{
   wmono_      = s.wmono_;
   id_         = s.id_;
   callsign_   = s.callsign_;
   code_       = s.code_;
   obstime_    = s.obstime_;
   createtime_ = s.createtime_;
   crc_        = s.crc_;
   ccx_        = s.ccx_;
   data_       = s.data_;
   bufrBase64_ = s.bufrBase64_;
   createSortIndex();

   return true;
}



bool 
TblBufr::
set(int                  wmono,
    int                  id,
    const std::string    &callsign,
    const std::string    &code,
    const miutil::miTime &obtime,    
    const miutil::miTime &createtime,    
    int                  crc,
    int                  ccx,
    const std::string    &data,
    const std::string    &bufrBase64)
{
   wmono_      = wmono;
   id_         = id;
   callsign_   = callsign;
   code_       = code;
   obstime_    = obtime;
   createtime_ = createtime;
   crc_        = crc;
   ccx_        = ccx;
   data_       = data;
   bufrBase64_ = bufrBase64;

   createSortIndex();

   return true;
}

#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblBufr::
toSend() const
{
   ostringstream ost;

   ost << "("
         << wmono_             << ","
         << id_                << ","
         << quoted( callsign_) << ","
         << quoted( code_) << ","
         << quoted( obstime_.isoTime() )    << ","
         << quoted( createtime_.isoTime() ) << ","
         << crc_               << ","
         << ccx_               << ","
         << quoted(data_)      << ","
         << quoted( bufrBase64_ )
         << ")";

   return ost.str();
}

#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblBufr::
uniqueKey()const
{
   ostringstream ost;

   ost << " WHERE wmono="   << wmono_ << " AND "
       << "       id=" << id_ << " AND "
       << "       callsign=" << quoted( callsign_ ) << " AND "
       << "       code=" << quoted( code_ ) << " AND "
       << "       obstime=" << quoted(obstime_.isoTime());

   return ost.str();
}


#ifdef __WITH_PUTOOLS__
miutil::miString 
#else
std::string
#endif
TblBufr::
toUpdate()const
{
   ostringstream ost;

   ost << "SET createtime=" << quoted( createtime_.isoTime() ) << ","
       <<            "crc=" << crc_                << ","
       <<            "ccx=" << ccx_                << ","
       <<           "data=" << quoted(data_)       << ","
       <<     "bufrBase64=" << quoted( bufrBase64_ )
       << " WHERE   wmono=" << wmono_ << " AND "
       << "            id=" << id_ << " AND "
       << "      callsign=" << quoted( callsign_ ) << " AND "
       << "          code=" << quoted( code_ ) << " AND "
       << "       obstime=" << quoted(obstime_.isoTime());

   return ost.str();
}
