/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiver.cc,v 1.14.2.20 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <puTools/miTime.h>
#include <milog/milog.h>
#include <boost/algorithm/string/replace.hpp>
//#include <miutil/replace.h>
#include <kvalobs/kvPath.h>

#include "App.h"
#include "Data.h"
#include "kvDbGateProxy.h"
#include "DataReceiver.h"
#include "tblBufr.h"
#include "defines.h"

using namespace std;
using namespace kvservice;
using namespace kvalobs;
using namespace milog;

DataReceiver::DataReceiver(App &app_,
                           dnmi::thread::CommandQue &inputQue_,
                           dnmi::thread::CommandQue &outputQue_):
                                 app(app_),
                                 inputQue(inputQue_),
                                 outputQue(outputQue_)
{
}

void 
DataReceiver::
operator()()
{
   dnmi::thread::CommandBase *com;
   DataEvent                 *event;

   milog::LogContext context("DataReceiver");

   while(!app.shutdown()){
      com=inputQue.get(1);

      if(!com)
         continue;

      event=dynamic_cast<DataEvent*>(com);

      if(!event){
         //Is this an event from the delaycontrol thread or
         //getDataThread. getDataThread is started at program startup
         //to get "old" data from kvalobs. It is terminated when
         //all "old" data is received.

         ObsEvent *obsevent=dynamic_cast<ObsEvent*>(com);

         if(!obsevent){
            LOGERROR("Unexpected event!");
            delete com;
            continue;
         }

         doCheckReceivedData(obsevent);

         continue;
      }

      newData(event->data());

      delete event;
   }
}

void 
DataReceiver::doCheckReceivedData(ObsEvent *obsevent)
{
   milog::LogContext context("delaycontrol");

   setDefaultLogger(obsevent->stationInfo());

   if( !obsevent->stationInfo()->msgForTime( obsevent->obstime() ) ){
      LOGINFO("Skip BUFFER for this hour: " << obsevent->obstime() <<
              " " << obsevent->stationInfo()->toIdentString() );
      delete obsevent;
      Logger::resetDefaultLogger();
      return;
   }

   if(!typeidReceived(*obsevent)){
      LOGERROR("Cant get information about received stationid/typeid!"<<
               "Deleting event: " << obsevent->stationInfo()->toIdentString() <<
               " obstime: " << obsevent->obstime());
      delete obsevent;
      obsevent = 0;
   }else if(obsevent->nTypeidReceived()==0){
      LOGWARN("No stationid/typeid received!"<<
              "Deleting event: " << obsevent->stationInfo()->toIdentString() <<
              " obstime: " << obsevent->obstime());
      delete obsevent;
      obsevent = 0;
   }

   LOGDEBUG3("Resend the event on outputque (newObsQue)");

   if( obsevent )
      outputQue.postAndBrodcast(obsevent);

   Logger::resetDefaultLogger();
}


void 
DataReceiver::newData(kvservice::KvObsDataListPtr data)
{
   kvalobs::kvDbGateProxy gate( app.dbThread->dbQue );
   IKvObsDataList    it;
   //StationInfoPtr    station;
   StationInfoList    stations;
   miutil::miTime    toTime;
   miutil::miTime    fromTime;
   DataKeySet        dataInserted;

   //data, er en liste av lister <KvData> til observasjoner
   //med samme stationid, typeid og obstime.

   gate.busytimeout(120);
   milog::LogContext context("newdata");

   toTime=miutil::miTime::nowTime();
   fromTime=toTime;
   toTime.addHour(3);
   fromTime.addDay(-3);

   LOGINFO("Accepting data in the time interval: " << fromTime << " - " <<
           toTime);

   for(it=data->begin();
         it!=data->end();
         it++){

      KvObsData::kvDataList::iterator dit=it->dataList().begin();

      if(dit==it->dataList().end()){
         LOGWARN("Data received from kvalobs: Unexpected, NO Data!");
         continue;
      }


      stations=app.findStationInfo(dit->stationID());

      if( stations.empty() ){
         LOGWARN("NO StationInfo: " << dit->stationID());
         continue;
      }

      for( StationList::iterator itStation=stations.begin();
            itStation != stations.end(); ++itStation ) {
         std::list<Data> dataList;
         LOGINFO("Data received from kvalobs: stationID: "
               << it->stationid()
               << " (" << (*itStation)->toIdentString() << ")" << endl
               << "       obstime: " << dit->obstime()  << endl
               << "        typeid: " << dit->typeID()  << endl
               << "   #parameters: " << it->dataList().size() << endl);

         if(!app.acceptAllTimes() &&
               (dit->obstime()< TO_PTIME( fromTime ) || dit->obstime()> TO_PTIME( toTime ) )){
            LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
                    "-- Valid interval: " << fromTime << " - " << toTime);

            //No 'continue' here! We reevaluate after we have set up to log
            //to a station specific file and write the message to the logfile
            //before we continue.
            //See  **1**

         }


         if(!(*itStation)->hasTypeId(dit->typeID())){
            LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");

            //No 'continue' here! We reevaluate after we have set up to log
            //to a station specific file and write the message to the logfile
            //before we continue.
            //See  **2**
         }

         //Setup to log to a station specific file.
         setDefaultLogger(*itStation);

         //This is repeated here to get it out on the log file for the wmono.
         LOGINFO("Data received from kvalobs: stationID: "
               << it->stationid()<< endl
               << " (" << (*itStation)->toIdentString() << ")" << endl
               << "       obstime: " << dit->obstime() << endl
               << "        typeid: " << dit->typeID()  << endl
               << "   #parameters: " << it->dataList().size() << endl
               << "Accepting data in time interval: " << fromTime << " - " <<
               toTime);

         //**1**
         if(!app.acceptAllTimes() &&
               (dit->obstime()< TO_PTIME( fromTime ) || dit->obstime() > TO_PTIME( toTime ) )){
            LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
                    "-- Valid interval: " << fromTime << " - " << toTime);
            Logger::resetDefaultLogger();
            LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
                    "-- Valid interval: " << fromTime << " - " << toTime);
            continue;
         }

         //**2**
         if(!(*itStation)->hasTypeId(dit->typeID())){
            LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
            Logger::resetDefaultLogger();
            LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
            continue;
         }

         for(dit=it->dataList().begin(); dit!=it->dataList().end(); dit++) {
            if( dataInserted.add( DataKey(*dit) ) )
               dataList.push_back(Data(*dit));
         }

         dit=it->dataList().begin();

         if(dataList.size()==0 && dataInserted.size() == 0 ) {
            LOGDEBUG("No new data to insert in database:" << endl
                     << "  stationid: " << dit->stationID()
                     << " obstime: " <<  dit->obstime()
                     << " typeid: " << dit->typeID() << endl);
            Logger::resetDefaultLogger();
            LOGDEBUG("No new data to insert in database:" << endl
                     << "  stationid: " << dit->stationID()
                     << " obstime: " <<  dit->obstime()
                     << " typeid: " << dit->typeID() << endl);
            continue;
         }

         if( ! dataList.empty() && !gate.insert(dataList, true)){
            LOGERROR("Cant insert data: \n  stationid: " << dit->stationID()
                     << " obstime: " << dit->obstime()
                     << "  reason: " << gate.getErrorStr() << endl);
            Logger::resetDefaultLogger();
            LOGERROR("Cant insert data: \n  stationid: " << dit->stationID()
                     << " obstime: " << dit->obstime()
                     << "  reason: " << gate.getErrorStr() << endl);

            continue;
         }

         if( !(*itStation)->msgForTime( TO_MITIME( dit->obstime() ) ) ) {
            LOGINFO("Skip BUFR for this hour: " << dit->obstime() << endl
                    <<  " stationid: " << dit->stationID() << endl
                    <<  " typeid: " << dit->typeID());
            Logger::resetDefaultLogger();
            LOGINFO("Skip BUFR for this hour: " << dit->obstime() << endl
                    <<  " stationid: " << dit->stationID() << endl
                    <<  " typeid: " << dit->typeID());
            continue;
         }

         std::list<Data>::iterator dataIt=dataList.begin();

         ObsEvent *event;

         ostringstream errs;

         errs << "Inserted data # " << dataList.size() << " :" << endl
               << "  stationid: " << dataIt->stationID()
               << " obstime: " <<  dataIt->obstime()
               << " typeid: " << dataIt->typeID() << endl;

         LOGINFO(errs.str());

         try{
            event=new ObsEvent(dataIt->obstime(),
                               *itStation);

            if(!typeidReceived(*event)){
               errs << "FAILED: Cant get informatiom about (stationid/typeid) " <<
                     "received for station <" << event->stationInfo()->toIdentString() <<
                     "> at obstime: " << event->obstime() << endl;
               LOGWARN("FAILED: Cant get informatiom about (stationid/typeid) " <<
                       "received for station <" << event->stationInfo()->toIdentString() <<
                       "> at obstime: " << event->obstime());
            }else if(event->nTypeidReceived()==0){
               errs << "No data received (stationid/typeid)!" << endl <<
                     "Station: " << event->stationInfo()->toIdentString() <<
                     " obstime: " << event->obstime() << endl;

               LOGWARN("No data received (stationid/typeid)!" << endl <<
                       "Station: " << event->stationInfo()->toIdentString() <<
                       " obstime: " << event->obstime());
               delete event;
            }else{
               app.addObsEvent(event, outputQue);
            }
         }
         catch(...){
            errs << "NOMEM: cant send a ObsEvent!";
            LOGERROR("NOMEM: cant send a ObsEvent!");
         }

         prepareToProcessAnyBufrBasedOnThisObs(dataIt->obstime(),
                                               *itStation);

         Logger::resetDefaultLogger();

         LOGINFO( errs.str() );
      }
   }
}

void
DataReceiver::
prepareToProcessAnyBufrBasedOnThisObs(const miutil::miTime &obstime,
                                      StationInfoPtr station)
{
   //TODO: This function only works correct for observation
   //on whole hours. It must be looked through again to be implemented
   //for observations with better resolution than one hour. It only
   //regenerates BUFR that is on WMO standard times ie. 0,3,6,..,21.

   std::list<TblBufr> bufrData;
   miutil::miTime      now(miutil::miTime::nowTime());
   miutil::miTime      maxTime;
   miutil::miTime      time=obstime;

   maxTime=miutil::miTime(obstime.year(), obstime.month(), obstime.day(),
                          obstime.hour(), 0, 0);
   maxTime.addHour(24);

   milog::LogContext context("regenerate");

   if(maxTime>now)
      maxTime=miutil::miTime(now.year(), now.month(), now.day(),
                             now.hour(), 0, 0);

   int hour=obstime.hour();
   int r=hour%3;

   //Nearest BUFR time larger than obstime;
   if(r==0)
      time.addHour(3);
   else
      time.addHour(3-r);

   while(time<=maxTime){

      if( ! station->msgForTime( time ) ) {
         time.addHour(3);
         continue;
      }

      LOGINFO("Possibly regenerating BUFR for: <" <<
              station->toIdentString() << "> obstime: " << time<<endl);

      try{
         ObsEvent *event=new ObsEvent(time, station, true);

         if(!typeidReceived(*event)){
            LOGWARN("FAILED: Can't get information about (stationid/typeid) " <<
                    "received for station <"<< event->stationInfo()->toIdentString() <<
                    "> at obstime " << event->obstime() << ".");
            delete event;
            return;
         }else if(event->nTypeidReceived()==0){
            LOGWARN("No data received (stationid/typeid)!" << endl <<
                    "Station: " << event->stationInfo()->toIdentString() <<
                    " obstime: " << event->obstime());
            delete event;
         }else{
            app.addObsEvent(event, outputQue);
         }
      }
      catch(...){
         LOGERROR("NOMEM: can't send a ObsEvent!");
      }

      time.addHour(3);
   }

}

bool
DataReceiver::
typeidReceived(ObsEvent &event)
{
   StationInfo::TLongList stations=event.stationInfo()->definedStationID();
   StationInfo::TLongList::iterator it=stations.begin();
   ostringstream ost;


   kvDbGateProxy gate( app.dbThread->dbQue  );
   gate.busytimeout( 120 );

   if(it==stations.end())
      return false;

   ost << "SELECT distinct stationid,typeid FROM data WHERE "
         << "obstime='" << event.obstime().isoTime() << "' AND "
         << "stationid IN (" << *it;

   it++;
   for(;it!=stations.end();it++){
      ost << "," << *it;
   }

   ost << ")";

   LOGDEBUG3("typeidReceived: query:" << endl << ost.str());

   KvDbGateResult dbResult;
   LOGDEBUG("gate.exec dbResult called.");
   if( ! gate.exec( dbResult, ost.str() ) ) {
      LOGERROR("Cant read the stationid/typeid for " << endl <<
               "event: " << ost.str() << " obstime: " <<
               event.obstime() << endl << "Reason: " << gate.getErrorStr() );
      return false;
   }
   LOGDEBUG("RETURN gate.exec dbResult called.");

   if( dbResult.size() == 0 )
      return false;

   int ssid;
   int stid;

   try{
      ost.str("");
      ost << "typeidReceived for station: " << event.stationInfo()->toIdentString() <<
            " obstime: " << event.obstime() << endl << " (stationid/typeid):";

      for( KvDbGateResult::const_iterator it = dbResult.begin(); it!=dbResult.end(); ++it ) {

         ssid = it->at( 0 ).asInt();
         stid= it->at( 1 ).asInt();

         if( ssid == INT_MAX || stid == INT_MAX )
            continue;

         ost << " (" << ssid << "/" << stid << ")";
         event.addTypeidReceived( ssid, stid );
      }

      LOGINFO(ost.str());
   }
   catch( const std::exception &ex){
      LOGERROR("Fetch error: stationid/typeid for " << endl <<
               "event: " << ost.str() << " obstime: " <<
               event.obstime() << endl << "Reason: " << ex.what() );

      //We clear the list. An empty list means "accept all", when the data
      //is used i BUFFER generation!
      event.clearTypeidReceived();
      return false;
   }

   return true;
}

void 
DataReceiver::setDefaultLogger(StationInfoPtr station)
{
   try{
      FLogStream *logs=new FLogStream(1, 204800); //200k
      std::ostringstream ost;
      string code( station->codeToString() );

      boost::replace_all( code, " ", "_" );
      //miutil::replaceStr( code," ", "_" );
      ost << kvPath("logdir") << "/" << options.progname << "/dr-"
          << station->toIdentString() <<"_code_" << code << ".log";

      if(logs->open(ost.str())){
         Logger::setDefaultLogger(logs);
         milog::LogLevel ll = station->loglevel();
         if( ll == milog::NOTSET )
            ll = milog::DEBUG;

         Logger::logger().logLevel( ll );
      }else{
         LOGERROR("Cant open the logfile <" << ost.str() << ">!");
         delete logs;
      }
   }
   catch(...){
      LOGERROR("Cant create a logstream for: " <<
               station->toIdentString() << " codes: " << station->codeToString() );
   }
}
