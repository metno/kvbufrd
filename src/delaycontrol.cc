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
#include "kvevents.h"
#include "delaycontrol.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;

namespace pt=boost::posix_time;

DelayControl::DelayControl(App &app_, 
                           std::shared_ptr<threadutil::CommandQueue> que_)
: app(app_), que(que_)
{
}

void 
DelayControl::operator()()
{
   bool    cachereload=true;
   time_t   nextTime;
   time_t   checkPoint;
   time_t   tNow;
   pt::ptime oldCheckpoint=app.checkpoint();
   ObsEvent *event;
   ostringstream ost;

   milog::LogContext context("DelayControl");

   IDLOGINFO("DelayCtl", "DelayControl: started");
   LOGINFO("DelayControl: started");

   time(&tNow);

   nextTime=tNow-tNow%60+60;  //Compute the nearest minute in the future.
   checkPoint=tNow-tNow%3600; //Compute the nearest hour in the future

   if(!oldCheckpoint.is_special()){
      if(oldCheckpoint<pt::from_time_t(checkPoint)){
         app.createCheckpoint(pt::from_time_t(checkPoint));
      }
   }else{
      app.createCheckpoint(pt::from_time_t(checkPoint));
   }

   checkPoint+=3600;    //Compute next checkpoint time

   IDLOGDEBUG("DelayCtl","checkPont: " << pt::to_kvalobs_string(pt::from_time_t(checkPoint)));
   IDLOGDEBUG("DelayCtl","nextTime:  " << pt::to_kvalobs_string(pt::from_time_t(nextTime)));

   while(!app.shutdown()){
      time(&tNow);

      if(checkPoint<tNow){
         app.createCheckpoint(pt::from_time_t(checkPoint));
         checkPoint+=3600;
      }

      if(nextTime>tNow){
        if( cachereload ) {
          cachereload = app.checkObsEventWaitingOnCacheReload(*que, "DelayCtl");
        }

        sleep(1);
        continue;
      }

      if(app.joinGetDataThreads(false, "DelayCtl")){
         IDLOGINFO("DelayCtl", "Joined at least one GetDataThread!");
      }

      nextTime+=60; //The next check time
      WaitingList waiting=app.getExpired();

      for(IWaitingList it=waiting.begin(); it!=waiting.end(); it++){
         try{
            event=new ObsEvent(*it);
         }
         catch(...){
            LOGERROR("NOMEM: cant allocate an ObsEvent!");
            break;
         }

         app.addObsEvent(event, *que);

         ost << (*it)->info()->toIdentString() << " obstime: " << pt::to_kvalobs_string((*it)->obstime())
	        << " delay to: " << pt::to_kvalobs_string((*it)->delay()) << endl;
      }

      if(!ost.str().empty()){
         LOGINFO("Expired stations: " << pt::to_kvalobs_string(pt::second_clock::universal_time()) << endl << ost.str());
         IDLOGINFO("DelayCtl",
                   "Expired stations: " << pt::to_kvalobs_string(pt::second_clock::universal_time()) << endl
                   << ost.str());
      }

      ost.str("");
      cachereload = app.checkObsEventWaitingOnCacheReload(*que, "DelayCtl");
   }

   IDLOGINFO("DelayCtl", "Join all getDataThreads!");

   if(app.joinGetDataThreads(true, "DelayCtl")){
      IDLOGINFO("DelayCtl","At least one getDataThread was joined!");
   }

   IDLOGINFO("DelayCtl", "All getDataThreads terminated!");


   LOGINFO("DelayControl: terminated");
   IDLOGINFO("DelayCtl", "DelayControl: terminated");
} 
