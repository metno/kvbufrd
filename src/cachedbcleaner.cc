/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: delaycontrol.cc,v 1.3.6.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <time.h>
#include <sstream>
#include "boost/date_time/posix_time/ptime.hpp"
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "obsevent.h"
#include "cachedbcleaner.h"
#include "kvDbGateProxy.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;

namespace pt=boost::posix_time;

namespace {
  const char* cleanSql[] = {
    "delete from data where tbtime<date('now', '-8 day');",
    "delete from bufr where tbtime<date('now', '-31 day');",
    "VACUUM;",
    0 
  };
}


bool CacheDbCleaner::cleanCache() {
  ostringstream ost;
  kvDbGateProxy gate( app.dbThread->dbQue );
     
  gate.busytimeout(300);
  int errCnt=0;

  for ( int i=0; cleanSql[i]; i++) {
    if( ! gate.exec(cleanSql[i]) ) {
      IDLOGERROR("cachedb", "query '" << cleanSql[i] << "' failed. Reason: " << gate.getErrorStr());
      LOGERROR("query '" << cleanSql[i] << "' failed. Reason: " << gate.getErrorStr());
      cerr << "query '" << cleanSql[i] << "' failed. Reason: " << gate.getErrorStr() << endl << endl;
      errCnt++;
    } 
  }

  if (errCnt == 0 ) {
    return true;
  }
  return false;
}


CacheDbCleaner::CacheDbCleaner(App &app_)
: app(app_)
{
}

void
CacheDbCleaner::operator()()
{
  time_t   nextTime;
  time_t   tNow;
   
  ostringstream ost;
  IDLOGINFO("cachedb", "CacheDbCleaner: started");
  LOGINFO("CacheDbCleaner: started");

   time(&tNow);

   nextTime=tNow-(tNow%3600)+1800; //Compute the nearest XX:30 hour from current time
   
   while(!app.shutdown()){
      time(&tNow);

      if(nextTime>tNow){
         sleep(1);
         continue;    
      }

      IDLOGINFO("cachedb","Start cleaning cachedb.");
      time_t start=tNow;
      
      if( cleanCache() ) {
        time(&tNow);
        IDLOGINFO("cachedb", "CacheDB cleaned in " << tNow - start << " seconds");
      } else {
        IDLOGERROR("cachedb", "Cleaning CacheDB failed. Duration: " << tNow - start << " seconds");
      }
      
      time(&tNow);
      
      do {
        nextTime += 3600;
      } while( nextTime < tNow);
   }

      
   LOGINFO("CacheDbCleaner: terminated");
   IDLOGINFO("cachedb", "CacheDbCleaner: terminated");
} 
