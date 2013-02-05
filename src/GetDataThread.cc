/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $                                                       

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
/* $Header: /var/lib/cvs/kvalobs/src/kvbufferd/GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $ */
#include <milog/milog.h>
#include <sstream>
#include <puTools/miTime.h>
#include "getDataReceiver.h"
#include "GetDataThread.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;


GetData::
GetData( App                      &app_,
         const miutil::miTime     &fromTime_,
         int                      wmono_,
         int                      hours_,
         dnmi::thread::CommandQue &que_):
  	app(app_), 
  	que(que_),
  	fromTime(fromTime_),
  	joinable_(new bool(false)),
  	wmono(wmono_),
  	hours(hours_)
{
}
  
void 
GetData::
operator()()
{
  miTime                   now(miTime::nowTime());
  miTime                   bufferFromTime(now);

  if(hours>=0){
    bufferFromTime.addHour(-6);

    if(fromTime>=bufferFromTime)
      bufferFromTime=fromTime;
  }

  LogContext lContext("GetDataKv("+now.isoTime()+")"); 

  //Generate buffer for maximum 7 hours back in time.

  LOGINFO("Started GetData thread!");
  IDLOGINFO("GetData", "Started GetData thread!");

  IDLOGINFO("GetData", 
	         "fromTime: " << fromTime << " hours: " << hours <<
	         " wmono: " << wmono << endl <<
	         " bufferFromTime: " << bufferFromTime);

  kvalobs::kvDbGateProxy gate( app.dbThread->dbQue );
  gate.busytimeout(120);

  if(wmono < 0){
    reloadAll(gate, bufferFromTime);
  }else{
    reloadOne(gate, bufferFromTime);
  }


  LOGINFO("Exit GetData thread!");
  IDLOGINFO("GetData", "Exit GetData thread!");
  *joinable_=true;
} 


void 
GetData::
reloadAll(kvalobs::kvDbGateProxy    &gate,
          const miutil::miTime &bufferFromTime)
{
   kvservice::KvObsDataList dl;
   ostringstream            ost;
   string logid;

   //Mark all station to be reloaded.
   StationList stationList=app.reloadCache(-1);

   for(IStationList it=stationList.begin();
         it!=stationList.end() && !app.shutdown();
         it++){
      ost.str("");
      ost << "GetData-" << (*it)->toIdentString();
      logid = ost.str();

      app.createGlobalLogger( logid, (*it)->loglevel() );

      StationInfo::TLongList idList=(*it)->definedStationID();
      kvservice::WhichDataHelper  which;

      ost.str("");

      for(StationInfo::ITLongList sit=idList.begin();
            sit!=idList.end(); sit++){
         ost << " " <<*sit;
         which.addStation(*sit, TO_PTIME( fromTime) );
      }

      IDLOGINFO( logid,
                 "Started GetData: " << (*it)->toIdentString() << " stationids:" <<
                 ost.str() << " FromTime: "  << fromTime);

      IDLOGINFO( "GetData",
                 "Started GetData: " << (*it)->toIdentString() << " stationids:" <<
                 ost.str() << " FromTime: "  << fromTime);

      GetKvDataReceiver myDataReceiver(app, bufferFromTime, que, gate, logid);

      if(!app.getKvData(myDataReceiver, which)){
         IDLOGERROR( "GetData",
                     "Failed GetData: " << (*it)->toIdentString()
                     << " stationids:" << ost.str() << " FromTime: "  << fromTime);
         IDLOGERROR( logid,
                     "Failed GetData: " << (*it)->toIdentString()
                     << " stationids:" << ost.str() << " FromTime: "  << fromTime);
      }

      IDLOGINFO( "GetData",
                 "Success GetData: " << (*it)->toIdentString() << " stationids:" <<
                 ost.str() << " FromTime: "  << fromTime);
      IDLOGINFO( logid,
                 "Success GetData: " << (*it)->toIdentString() << " stationids:" <<
                 ost.str() << " FromTime: "  << fromTime);


      app.cacheReloaded( *it );
   }
}

void 
GetData::
reloadOne(kvalobs::kvDbGateProxy    &gate,
          const miutil::miTime &bufferFromTime)
{
   string logid;
   ostringstream                ost;
   StationList stationList=app.reloadCache(wmono);

   if( stationList.empty() ){
      LOGWARN("No station information for station: wmono:" << wmono <<"!");
      IDLOGWARN("GetData", "No station information for wmono: " << wmono <<
                "!");
      return;
   }

   ost << "GetData-" << (*stationList.begin())->toIdentString();
   logid = ost.str();

   app.createGlobalLogger( logid );


   StationInfo::TLongList idList=(*stationList.begin())->definedStationID();
   kvservice::WhichDataHelper  which;

   ost.str("");

   for(StationInfo::ITLongList sit=idList.begin();
         sit!=idList.end(); sit++){
      ost << " " <<*sit;
      which.addStation(*sit, TO_PTIME( fromTime ) ) ;
   }

   IDLOGINFO( logid,
              "Started GetData: " << (*stationList.begin())->toIdentString()
              << " stationids:" << ost.str() << " FromTime: "  << fromTime);

   IDLOGINFO( "GetData",
              "Started GetData: wmono=" << (*stationList.begin())->toIdentString()
              << " stationids:" << ost.str() << " FromTime: "  << fromTime);



   GetKvDataReceiver myDataReceiver(app, bufferFromTime, que, gate, logid);

   if(!app.getKvData(myDataReceiver, which)){
      IDLOGERROR("GetData",
                 "Failed GetData: " << (*stationList.begin())->toIdentString()
                 << " stationids:" << ost.str() << " FromTime: "  << fromTime);
      IDLOGERROR( logid,
                  "Failed GetData: " << (*stationList.begin())->toIdentString()
                  << " stationids:" << ost.str() << " FromTime: "  << fromTime);
   } else {
      IDLOGINFO( "GetData",
                 "Success GetData: " << (*stationList.begin())->toIdentString()
                 << " stationids:" << ost.str() << " FromTime: "  << fromTime);
      IDLOGINFO( logid,
                 "Success GetData: " << (*stationList.begin())->toIdentString()
                 << " stationids:" << ost.str() << " FromTime: "  << fromTime);
   }

   app.cacheReloaded( *stationList.begin() );

}
