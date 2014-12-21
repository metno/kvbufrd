/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: BufrWorker.cc,v 1.27.2.21 2007/09/27 09:02:23 paule Exp $

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <fstream>
#include <list>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include "App.h"
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "kvDbGateProxy.h"
#include "Data.h"
#include "StationInfo.h"
#include "BufrWorker.h"
#include "obsevent.h"
#include "bufr.h"
#include "LoadBufrData.h"
#include "base64.h"
#include "SemiUniqueName.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;

namespace fs = boost::filesystem;



BufrWorker::BufrWorker(App &app_,
                       dnmi::thread::CommandQue &que_,
                       dnmi::thread::CommandQue &replayQue_)
: app(app_), que(que_), replayQue(replayQue_),
  swmsg(*(new std::ostringstream())),
  encodeBufrManager( new EncodeBufrManager() )
{
   string filename=string( DATADIR ) + "/B0000000000000019000.TXT";
   bufrParamValidater = BufrParamValidater::loadTable( filename );
}

void 
BufrWorker::operator()()
{
   dnmi::thread::CommandBase *com;
   ObsEvent                  *event;

   milog::LogContext context("BufrWorker");

   while(!app.shutdown()){
      try{
         com=que.get(1);
      }
      catch(dnmi::thread::QueSuspended &e){
         LOGDEBUG6("EXCEPTION(QueSuspended): Input que is susspended!" << endl);
         continue;
      }
      catch(...){
         LOGDEBUG("EXCEPTION(Unknown): Uknown exception from input que!" << endl);
         continue;
      }

      if(!com)
         continue;

      event=dynamic_cast<ObsEvent*>( com );

      if(!event){
         LOGERROR("Unexpected event!");
         delete com;
         continue;
      }

      LOGINFO("New observation: ("<<event->stationInfo()->toIdentString() << ") "
              << event->obstime() << " regenerate: "
              << (event->regenerate()? "T":"F") << " client: "
              << (event->hasCallback()?"T":"F"));

      try{
         FLogStream *logs=new FLogStream(2, 307200); //300k
         std::ostringstream ost;

         ost << kvPath("logdir") << "/" << options.progname << "/"
               << event->stationInfo()->toIdentString() << ".log";

         if(logs->open(ost.str())){
            Logger::setDefaultLogger(logs);
            Logger::logger().logLevel(event->stationInfo()->loglevel());

            //Log to stationspecific logfile also.
            LOGINFO("+++++++ Start processing observation +++++++" << endl
                    << "New observation: ("<<event->stationInfo()->toIdentString() << ") "
                    << event->obstime() << " regenerate: "
                    << (event->regenerate()? "T":"F") << " client: "
                    << (event->hasCallback()?"T":"F"));
         }else{
            LOGERROR("Cant open the logfile <" << ost.str() << ">!");
            delete logs;
         }
      }
      catch(...){
         LOGERROR("Cant create a logstream for station: " <<
                  event->stationInfo()->toIdentString() );
      }

      swmsg.str("");

      try{
         newObs(*event);
      }
      catch(...){
         LOGERROR("EXCEPTION(Unknown): Unexpected exception from " <<
                  "BufrWorker::newObs" << endl);
      }

      if(event->hasCallback()){
         replayQue.postAndBrodcast(event);
      }else{
         delete event;
      }

      LOGINFO("------- End processing observation -------");

      Logger::resetDefaultLogger();

      LOGINFO(swmsg.str());

   }
} 


bool
BufrWorker::
readyForBufr( const DataEntryList &data,
              ObsEvent      &e   )const
{
   bool haveAllTypes;
   bool mustHaveTypes;
   bool force;
   int  delayMin;
   bool relativToFirst;

   miTime obstime=e.obstime();
   StationInfoPtr info=e.stationInfo();

   milog::LogContext context("readyForBufr");

   miTime delayTime;
   miTime nowTime(miTime::nowTime());

   haveAllTypes=checkTypes(data, info, obstime, mustHaveTypes);
   delayMin = info->delay( obstime, force, relativToFirst);

   LOGDEBUG3("haveAllTypes:  " << (haveAllTypes?"TRUE":"FALSE") << endl <<
             "mustHaveTypes: " << (mustHaveTypes?"TRUE":"FALSE") << endl <<
             "delay: " << (delayMin!=0?"TRUE":"FALSE") << " min: " << delayMin <<
             " force: " << (force?"TRUE":"FALSE") << " relativToFirst: " <<
             (relativToFirst?"TRUE":"FALSE") << endl <<
             " nowTime: " << nowTime << endl <<
             " #ContinuesTimes: " << data.nContinuesTimes()<< endl);

   if(!haveAllTypes && !mustHaveTypes){
      //We do not have all types we need, we are also missing
      //the types we need to make an incomplete bufr. Just
      //drop this event, dont make a waiting element for it, it is
      //useless;

      swmsg << "Missing mustHaveTypes!";
      return false;
   }


   if( delayMin > 0 ){
      if(relativToFirst){
         //If we allready have a registred waiting element dont replace it.
         //This ensures that we only register an waiting element for
         //the first data received.

         if(haveAllTypes)
            return true;


         delayTime=nowTime;
         delayTime.addMin( delayMin );
         WaitingPtr wp=e.waiting();

         if(!wp){
            LOGDEBUG1("Delaying (relativeToFirst): " << delayTime);
            app.addWaiting(WaitingPtr(new Waiting(delayTime,
                                                  obstime, info)),
                                                  false );

            swmsg << "Delay (relativToFirst): " << delayTime;
            return false;
         }else{
            LOGDEBUG1("Is delayed (relativeToFirst) to: " << wp->delay());
            if(wp->delay()<=nowTime){
               LOGDEBUG1("Is delayed (relativeToFirst): expired!");
               return true;
            }else{
               LOGDEBUG1("Is delayed (relativeToFirst): Not expired!");
               //Reinsert in the delaylist.
               app.addWaiting(wp,false );

               swmsg << "Delay (relativToFirst) Not expired: " << delayTime;

               return false;
            }
         }
      }else{ //Delay relative to obstime.
         delayTime=miTime(obstime.date(),miClock(obstime.hour(), delayMin, 0));
      }
   }

   if(haveAllTypes){
      if( delayMin > 0 ){
         if(!force){
            return true;
         }else{
            if(delayTime<nowTime)
               return true;

            try{
               app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)),
                              true );
            }
            catch(...){
               LOGFATAL("NOMEM: cant allocate delay element!");
            }

            swmsg << "Delay: " << delayTime;

            return false;
         }
      }else{
         return true;
      }
   }

   if( delayMin > 0 ){
      if(delayTime<nowTime){
         if(mustHaveTypes){
            return true;
         }else{
            return false;
         }
      }else{
         try{
            app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)), true );
         }
         catch(...){
            LOGFATAL("NOMEM: cant allocate delay element!");
         }
      }
   }else{
      if(mustHaveTypes)
         return true;
   }

   swmsg << "Not enough data!";
   return false;
}



/*
  If the event has a callback registred we write some error
  message that can be returned to the celler.
 */
void 
BufrWorker::
newObs(ObsEvent &event)
{
   EReadData       dataRes;
   DataEntryList   data;
   DataElementList bufrData;
   Bufr            bufrEncoder;
   BufrDataPtr     bufr;
   StationInfoPtr  info;
   ostringstream   ost;
   boost::uint16_t oldcrc=0;
   int             ccx=0;
   list<TblBufr>   tblBufrList;

   info=event.stationInfo();

   if( !info->msgForTime( event.obstime() ) ){


      LOGINFO("Skip BUFR for time: " << event.obstime() << " " <<
              info->toIdentString() );
      swmsg << "Skip BUFR for time: " << event.obstime() << " " <<
            info->toIdentString();


      if(event.hasCallback()){
         event.msg() << "Skip BUFR for time: " << event.obstime() <<
               " " << info->toIdentString();
         event.bufr("");
         event.isOk(false);
      }

      return;
   }

   //We check if this is a event for regeneraiting a BUFR
   //due to changed data. If it is, the bufr for this time
   //must allready exist. If it don't exist we could generate
   //a BUFR that is incomplete because of incomplete data.

   if( event.regenerate() ){
      list<TblBufr> tblBufrList;

      LOGINFO("Regenerate event: " << info->toIdentString() << ", obstime " <<
              event.obstime());

      if(app.getSavedBufrData(info,
                              event.obstime(),
                              tblBufrList)){
         if(tblBufrList.size()>0){
            LOGINFO("Regenerate event: Regenerate BUFR.");
         }else{
            LOGINFO("Regenerate event: No BUFR exist, don't regenerate!");
            swmsg << "Regenerate: No bufr exist.";

            return;
         }
      }else{
         LOGERROR("DBERROR: Regenerate event: Cant look up the bufr!");
         swmsg << "Regenerate: DB error!";

         return;
      }
   }


   dataRes=readData( event, data);

   if(dataRes!=RdOK){
      if(event.hasCallback()){
         event.isOk(false);

         switch(dataRes){
         case RdNoStation:
            event.msg() << "NOSTATION: No station configured!";
            break;
         case RdNoData:
            event.msg() << "NODATA: No data!";
            break;
         case RdMissingObstime:
            event.msg() << "MISSING OBSTIME: No data for the obstime!";
            break;
         default:
            event.msg() << "READERROR: cant read data!";
            break;
         }
      }

      swmsg << event.msg().str();

      return;
   }


   //If this event comes from the DelayControll and
   //is for data waiting on continues types don't run
   //it through the readyForBufr test. We know that
   //the readyForBufr has previously returned true for
   //this event.

   if(!event.waitingOnContinuesData()){
      //Don't delay a observation that is explicit asked for.
      //A BUFR that is explicit asked for has a callback.

      if(!event.hasCallback() && !readyForBufr(data, event)){
         return;
      }
   }

   //Check if shall test for continuesTypes. We do that
   //if we have now Waiting pointer or we have a Waiting pointer
   //but it has not been tested for waiting on continues data.
   if(!checkContinuesTypes(event, data)){
      return;
   }

   app.removeWaiting(info, event.obstime() );

   LOGINFO("ReadData: " << info->toIdentString() << " obstime: " <<
           event.obstime() << " # " << data.size());

   try{
      loadBufrData( data, bufrData, event.stationInfo());
   }
   catch(...){
      LOGDEBUG("EXCEPTION(Unknown): Unexpected exception from "<< endl <<
               "BufrWorker::loadBufrData" << endl);
   }

   CIDataElementList it=bufrData.begin();
   ost.str("");

   for(int i=0;it!=bufrData.end(); i++, it++)
      ost << it->time() << "  [" << i << "] #params " << bufrData[i].numberOfValidParams() <<endl;

   LOGINFO("# number of bufrdata: " << bufrData.size() << endl<<
           "Continues: " << bufrData.nContinuesTimes() << endl <<
           "Time(s): " << endl << ost.str());

   if( bufrData.firstTime() != event.obstime() ) {
	   LOGWARN( "NO data at '" << event.obstime() << "' passed the valid checks. Skipping BUFR generation.");
	   return;
   }


   if( app.getSavedBufrData( info, event.obstime(), tblBufrList ) ){
      if(tblBufrList.size()>0){
         ccx=tblBufrList.front().ccx();
         oldcrc=tblBufrList.front().crc();
         ++ccx;
         LOGDEBUG("A BUFR for: " << info->toIdentString() << " obstime: " << event.obstime()
                  << " exist. ccx=" << ccx-1 << " crc: " << oldcrc );
      }
   }

   LOGDEBUG6(bufrData);

   try{
      bufr = bufrEncoder.doBufr( info, bufrData );
   }
   catch( const IdException &e ) {
      LOGWARN("EXCEPTION: Cant resolve for BUFR id: " << info->toIdentString() <<
                             " obstime: "  <<
                             ((bufrData.begin()!=bufrData.end())?
                                   bufrData.begin()->time().isoTime():"(NULL)") << endl <<
                                   "what: " << e.what() << endl);
      swmsg << "Cant create a bufr!" << endl;
   }
   catch( const NotImplementedException &e ) {
      LOGWARN("EXCEPTION: No template implemmented for: " << info->toIdentString() <<
                       " obstime: "  <<
                       ((bufrData.begin()!=bufrData.end())?
                             bufrData.begin()->time().isoTime():"(NULL)") << endl <<
                             "what: " << e.what() << endl);
               swmsg << "Cant create a bufr!" << endl;
   }
   catch(std::out_of_range &e){
      LOGWARN("EXCEPTION: out_of_range: " << info->toIdentString() <<
              " obstime: "  <<
              ((bufrData.begin()!=bufrData.end())?
                    bufrData.begin()->time().isoTime():"(NULL)") << endl <<
                    "what: " << e.what() << endl);
      swmsg << "Cant create a bufr!" << endl;
   }
   catch(DataListEntry::TimeError &e){
      LOGWARN("Exception: TimeError: " << info->toIdentString() << " obstime: "  <<
              ((bufrData.begin()!=bufrData.end())?
                    bufrData.begin()->time().isoTime():"(NULL)") << endl<<
                    "what: " << e.what() << endl);
      swmsg << "Cant create a bufr!" << endl;
   }
   catch( std::logic_error &e ) {
         LOGWARN("EXCEPTION: logic_error: " << info->toIdentString() <<
                     " obstime: "  <<
                     ((bufrData.begin()!=bufrData.end())?
                           bufrData.begin()->time().isoTime():"(NULL)") << endl <<
                           "what: " << e.what() << endl);
             swmsg << "Cant create a bufr!" << endl;
      }
   catch(...){
      LOGWARN("EXCEPTION(Unknown): Unexpected exception in Bufr::doBufr:" <<
              endl << " station: " << info->toIdentString() << " obstime: "  <<
              ((bufrData.begin()!=bufrData.end())?
                    bufrData.begin()->time().isoTime():"(NULL)") << endl);
      swmsg << "Cant create a bufr!" << endl;
   }

   if(! bufr ){
      if(event.hasCallback()){
         event.isOk(false);

         if(bufrData.size()==0)
            event.msg() << "NODATA:(" << event.obstime() <<") cant create bufr!";
         else
            event.msg() << "BUFR ERROR:(" << event.obstime() <<") cant create bufr!";
      }

      LOGERROR("Cant create BUFR for <"<< info->toIdentString()<<"> obstime: " <<
               event.obstime());
      swmsg << "Cant create a BUFR!" << endl;
   }else{
      boost::uint16_t crc=bufr->crc();
      string          base64;

      bool newBufr( crc != oldcrc );

      if(newBufr){
         miTime createTime(miTime::nowTime());
         ostringstream dataOst;

         bufrData.writeTo( dataOst );
         if( saveTo( info, bufr, ccx, &base64 ) ) {
             if(app.saveBufrData( TblBufr( info->wmono(), info->stationID(),
                                           info->callsign(), info->codeToString(),
                                           event.obstime(), createTime,
                                           crc, ccx, dataOst.str(), base64 ))) {
                 LOGINFO("BUFR information saved to database! (" << info->toIdentString() << ") ccx: "
                         << ccx << " crc: " << crc );
             } else {
                 LOGERROR("FAILED to save BUFR information to the database! (" << info->toIdentString() << ") ccx: "
                         << ccx << " crc: " << crc );
             }

             swmsg << "New BUFR created. (" << info->toIdentString() << ") " << event.obstime() << endl;
         }
      }else{
         LOGINFO("DUPLICATE: (" << info->toIdentString() << ") "
                 << event.obstime());

         swmsg << "Duplicate BUFR created. (" <<  info->toIdentString() << ") "
               << event.obstime() << endl;

         if(event.hasCallback())
            event.msg() << "DUPLICATE: (" << info->toIdentString() << ") "
            << event.obstime();

      }

      ost.str("");

      if(event.hasCallback()){
         //If we have a callback registred. Return the bufr
         event.bufr( base64 );
         event.isOk(true);
      }
   }
}

BufrWorker::EReadData
BufrWorker::readData( ObsEvent             &event,
                      DataEntryList        &data)const
{
   ostringstream                   ost;
   kvDbGateProxy                   gate( app.dbThread->dbQue );
   list<Data>                      dataList;
   list<Data>::iterator            dit;
   DataEntryList::CITDataEntryList it;
   StationInfo::TLongList          stIDs;
   StationInfo::RITLongList        itStId;
   StationInfoPtr                  station=event.stationInfo();
   miutil::miTime                  from(event.obstime());
   miutil::miTime                  to(event.obstime());
   bool                            hasObstime=false;
   kvdatacheck::Validate validate( kvdatacheck::Validate::UseOnlyUseInfo );

   data.clear();

   if(!station)
      return RdERROR;

   gate.busytimeout(120);

   from.addHour( -24 );

   stIDs=station->definedStationID();
   itStId=stIDs.rbegin();

   if(itStId==stIDs.rend()){
      LOGERROR("No stationid's for station <" << station->toIdentString() << ">!");
      return RdNoStation;
   }

   for(;itStId!=stIDs.rend(); itStId++){
      ost.str("");
      ost << " where stationid=" << *itStId
            << " AND "
            << " obstime>=\'"      << from.isoTime() << "\' AND "
            << " obstime<=\'"      << to.isoTime()   << "\'"
            << " order by obstime, typeid;";

      LOGDEBUG("query: " << ost.str());

      if(!gate.select(dataList, ost.str())){
         LOGERROR("Cant get data from the database!\nQuerry: " << ost.str()
                  << endl << "Reason: " << gate.getErrorStr());
         return RdERROR;
      }

      bool doLogTypeidInfo=true;

      for(dit=dataList.begin(); dit!=dataList.end(); dit++){
         try{
            if(event.hasReceivedTypeid(dit->stationID(),
                                       dit->typeID(),
                                       doLogTypeidInfo)){

               if( validate( *dit ) ) {
                  data.insert(*dit);

                  if(!hasObstime && dit->obstime()==event.obstime())
                     hasObstime=true;
               }
            }

            doLogTypeidInfo=false;
         }
         catch(DataListEntry::TimeError &e){
            LOGDEBUG("EXCEPTION(DataListEntry::TimeError): Hmmm... " <<
                     "This should not happend!!" << endl);
         }
      }
   }

   string rejected = validate.getLog();

   if( ! rejected.empty() ) {
      LOGWARN("RJECTED data." << endl << rejected );
   }

   it=data.begin();

   if(it!=data.end()){
      ostringstream ost;
      ost << "First obstime: " << it->obstime() << " - ";
      it=data.end();
      it--;
      ost << it->obstime();
      LOGDEBUG(ost.str());
   }else{
      LOGWARN("No data in the cache for the station!");
      return RdNoData;
   }

   if(!hasObstime){
      LOGERROR("No data for the obstime: " << event.obstime());
      return RdMissingObstime;
   }

   return RdOK;
}

void 
BufrWorker::loadBufrData(const DataEntryList &dl,
                         DataElementList       &sd,
                         StationInfoPtr      info)const
{
   kvdatacheck::Validate validate( kvdatacheck::Validate::NoCheck );
   ::loadBufrData( dl, sd, info, validate );
}


bool
BufrWorker::
checkTypes(const DataEntryList  &data, 
           StationInfoPtr stInfo,
           const miutil::miTime obstime,
           bool           &mustHaveTypes)const
{
   StationInfo::TLongList          tids=stInfo->typepriority();
   DataEntryList::CITDataEntryList dit=data.find(obstime);
   StationInfo::CITLongList        it;

   mustHaveTypes=false;

   if(dit==data.end()){
      LOGDEBUG("checkTypes: No data for: " << stInfo->toIdentString() << " obstime: " << obstime);
      return false;
   }

   if(dit->obstime()!=obstime){
      LOGDEBUG("checkTypes: No data for obstime: " << obstime <<  " station: " << stInfo->toIdentString());
      return false;
   }

   LOGDEBUG("checkTypes: " << *dit);

   for(it=tids.begin();it!=tids.end(); it++){
      if(dit->size(*it)==0)
         break;
   }

   if(it==tids.end()){
      mustHaveTypes=true;
      return true;
   }

   tids=stInfo->mustHaveTypes();
   ostringstream ost;

   //If we have no must have types defined, mustHaveTypes is set to true.
   if( tids.empty() )  {
	   mustHaveTypes = true;
	   ost << "checkTypes: No must have types is defined, this is ok.";
   } else {
	   ost << "checkTypes: mustHaveTypes:";

	   for(it=tids.begin();it!=tids.end(); it++)
		   ost << " " << *it;

	   for(it=tids.begin();it!=tids.end(); it++){
		   if(dit->size(*it)==0)
			   break;
	   }

	   if(it==tids.end())
		   mustHaveTypes=true;
   }

   LOGDEBUG(ost.str());


   return false;
}




bool
BufrWorker::
saveTo( StationInfoPtr info,
        BufrDataPtr  bufr,
        int ccx,
        std::string *base64 ) const
{
    bool doSave=true;
    int nValues;

   try {
      BufrHelper bufrHelper( bufrParamValidater, info, bufr );
      bufrHelper.setSequenceNumber( ccx );
      encodeBufrManager->encode( bufrHelper );

      if( ! bufrHelper.validBufr() ) {
          LOGINFO("INVALID BUFR: " << bufrHelper.getErrorMessage() );
          return false;
      }

      doSave = ! bufrHelper.emptyBufr();
      nValues = bufrHelper.nValues();

      if( !doSave ) {
          LOGINFO("No data values was written to the BUFR message, the BUFR message is NOT saved.");
          return false;
      }

      LOGINFO("BUFR with " << nValues << " real data values is saved.")
      bufrHelper.saveToFile();

      if( base64 ) {
         ostringstream ost;
         bufrHelper.writeToStream( ost );
         string buf = ost.str();
         encode64( buf.data(), buf.size(), *base64 );
      }
      return true;
   }
   catch( const std::exception &ex ) {
      LOGERROR("Failed to encode to BUFR: " << info->toIdentString() << " obstime: " << bufr->time() << ". Reason: " << ex.what());
      return false;
   }
   return false;
}

bool 
BufrWorker::
checkContinuesTypes(ObsEvent &event, 
                    const DataEntryList &data)const
{
   WaitingPtr w;
   StationInfoPtr info=event.stationInfo();

   if(event.waitingOnContinuesData()){
      //We have waited on this event in the predefined time,
      //just generate a bufr for this event.
      swmsg << "Expired waiting on continues data!" << endl;
      return true;
   }

   if((event.obstime().hour()%3)!=0){
      //Just interested in bufrtimes that use data from
      //multiple hours.

      return true;
   }

   w=app.getWaiting( event.obstime(), info );


   std::list<int> contTypes=info->continuesTypeids(app.continuesTypeID());

   if(contTypes.empty()){
      //The station has only no continues type ids.
      return true;
   }

   if(!data.hasContinuesTimes(contTypes, 4)){
      if(!w){
         miutil::miTime now(miutil::miTime::nowTime());
         now.addMin(5);

         try{
            w=WaitingPtr(new Waiting(now, event.obstime(), info, true));
         }
         catch(...){
            LOGERROR("NO MEM");
            return true;
         }
      }

      w->waitingOnContinuesData(true);

      app.addWaiting(w, true );

      LOGINFO("Waiting on continues data: " << info->toIdentString() <<
              " obstime: " << event.obstime() << " delay: " << w->delay());

      swmsg << "Waiting on continues data until: " << w->delay();

      return false;
   }

   return true;
}
