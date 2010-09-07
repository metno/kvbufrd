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
#include <kvalobs/kvDbGate.h>
#include <list>
#include <iomanip>
#include "Data.h"
#include <milog/milog.h>
#include <fstream>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include "StationInfo.h"
#include "BufrWorker.h"
#include "obsevent.h"
#include "bufr.h"
#include <kvalobs/kvPath.h>
#include "LoadBufrData.h"
#include "encodebufr.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;



BufrWorker::BufrWorker(App &app_,
			 dnmi::thread::CommandQue &que_,
			 dnmi::thread::CommandQue &replayQue_)
  : app(app_), que(que_), replayQue(replayQue_), con(0), 
    swmsg(*(new std::ostringstream()))
{
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
    
    	event=dynamic_cast<ObsEvent*>(com);

    	if(!event){
      	LOGERROR("Unexpected event!");
      	delete com;
      	continue;
    	}

    	if(!con){
      	con=app.getNewDbConnection();

      	if(!con){
				LOGFATAL("Cant create a connection to the database!\n");
				delete event;
				continue;
      	}
    	}

    	LOGINFO("New observation: ("<<event->stationInfo()->wmono() << ") "
	    		  << event->obstime() << " regenerate: " 
	    		  << (event->regenerate()? "T":"F") << " client: " 
	    		  << (event->hasCallback()?"T":"F"));

    	try{
      	FLogStream *logs=new FLogStream(2, 307200); //300k
      	std::ostringstream ost;

      	ost << kvPath("logdir") << "/kvbufr/"
	  			 << event->stationInfo()->wmono() << ".log";

      	if(logs->open(ost.str())){
				Logger::setDefaultLogger(logs);
				Logger::logger().logLevel(event->stationInfo()->loglevel());
				
				//Log to stationspecific logfile also.
				LOGINFO("+++++++ Start processing observation +++++++" << endl 
						  << "New observation: ("<<event->stationInfo()->wmono() << ") "
	    		  		  << event->obstime() << " regenerate: " 
	    		        << (event->regenerate()? "T":"F") << " client: " 
	    		        << (event->hasCallback()?"T":"F"));
      	}else{
				LOGERROR("Cant open the logfile <" << ost.str() << ">!");
				delete logs;
      	}
    	}
    	catch(...){
      	LOGERROR("Cant create a logstream for wmono: " << 
	      		   event->stationInfo()->wmono() );
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
  
  	if(con)
   	app.releaseDbConnection(con);
} 


bool
BufrWorker::
readyForBufr(const DataEntryList &data,
	                  ObsEvent      &e   )const
{
  	bool haveAllTypes;
  	bool mustHaveTypes;
  	bool force;
  	int  min;
  	bool delay;
  	bool relativToFirst;

  	miTime obstime=e.obstime();
  	StationInfoPtr info=e.stationInfo();

  	milog::LogContext context("readyForBufr");

  	miTime delayTime;
  	miTime nowTime(miTime::nowTime());
 
  	haveAllTypes=checkTypes(data, info, obstime, mustHaveTypes);
  	delay=info->delay(obstime.hour(), min, force, relativToFirst);

  	LOGDEBUG3("haveAllTypes:  " << (haveAllTypes?"TRUE":"FALSE") << endl <<
  				 "mustHaveTypes: " << (mustHaveTypes?"TRUE":"FALSE") << endl <<
	    		 "delay: " << (delay?"TRUE":"FALSE") << " min: " << min <<
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
    
  
  	if(delay){
    	if(relativToFirst){
      	//If we allready have a registred waiting element dont replace it.
      	//This ensures that we only register an waiting element for
      	//the first data received.
    
      	if(haveAllTypes)
				return true;
      
  
      	delayTime=nowTime;
      	delayTime.addMin(min);
      	WaitingPtr wp=e.waiting();
      
      	if(!wp){
				LOGDEBUG1("Delaying (relativeToFirst): " << delayTime);
				app.addWaiting(WaitingPtr(new Waiting(delayTime, 
				      	      obstime, info)),
		      					false,
		       					con);

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
	  				app.addWaiting(wp,false, con);
	  
	  				swmsg << "Delay (relativToFirst) Not expired: " << delayTime;
	  
	  				return false;
				}
			}
    	}else{ //Delay relative to obstime.
      	delayTime=miTime(obstime.date(),miClock(obstime.hour(), min, 0));
    	}
  	}

  	if(haveAllTypes){
    	if(delay){
      	if(!force){
				return true;
      	}else{
				if(delayTime<nowTime)
	  				return true;
	
				try{
	  				app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)),
			 							true,
			 							con);
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
  
  	if(delay){
    	if(delayTime<nowTime){
      	if(mustHaveTypes){
				return true;
      	}else{
				return false;
      	}
    	}else{
       	try{
	 			app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)),
									true,
									con);
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
BufrWorker::newObs(ObsEvent &event)
{
	EReadData      dataRes;
  	DataEntryList  data;   
  	DataElementList  bufrData;
  	Bufr          bufrEncoder;
  	BufrData      bufr;
  	string         sBufr;
  	StationInfoPtr info;
  	ostringstream  ost;
   boost::uint16_t  oldcrc=0;
   int              ccx=0;
   list<TblBufr>   tblBufrList;

  	info=event.stationInfo();
  
  	if(!info->msgForTime(event.obstime().hour())){
  		LOGINFO("Skip SYNOP for time: " << event.obstime() << "  wmono: " << 
  				  info->wmono());
  		swmsg << "Skip SYNOP for time: " << event.obstime() << "  wmono: " << 
  				   info->wmono();
  				 
  		if(event.hasCallback()){
  			event.msg() << "Skip SYNOP for time: " << event.obstime() << 
  						   "  wmono: " << info->wmono();
  			event.bufr("");
      	event.isOk(false);
  		}
  		
  		return;
  	}

  	//We check if this is a event for regeneraiting a SYNOP
  	//due to changed data. If it is, the bufr for this time
  	//must allready exist. If it don't exist we could generate 
  	//a SYNOP that is incomplete because of incomplete data.

  	if(event.regenerate()){
    	list<TblBufr> tblBufrList;

    	LOGINFO("Regenerate event: wmono " << info->wmono() << ", obstime " <<
	    		  event.obstime());
    
    	if(app.getSavedBufrData(info->wmono(),
							          event.obstime(), 
			     				       tblBufrList, *con)){
			if(tblBufrList.size()>0){
				LOGINFO("Regenerate event: Regenerate SYNOP.");
      	}else{
				LOGINFO("Regenerate event: No SYNOP exist, don't regenerate!");
				swmsg << "Regenerate: No bufr exist.";

				return;
     		}
		}else{
      	LOGERROR("DBERROR: Regenerate event: Cant look up the bufr!");
      	swmsg << "Regenerate: DB error!";

      	return;
    	}
  	}


  	dataRes=readData(*con, event, data);

  	if(dataRes!=RdOK){
    	if(event.hasCallback()){
      	event.isOk(false);
      	
      	switch(dataRes){
      	case RdNoStation:
				event.msg() << "NOSTATION: No station congigured!";
				break;
      	case RdNoData:
				event.msg() << "NODATA: No data!";
				break;
      	case RdMissingObstime:
				event.msg() << "MISSING OBSTIME: No data for the obstime!";
				break;
      	default:
				event.msg() << "READERROR: cant read data!";
      	}
    	}

    	swmsg << event.msg().str();

    	return;
  	}
  

  	//If this event comes frrom the DelayControll and
  	//is for data waiting on continues types don't run
  	//it through the readyForBufr test. We know that
  	//the readyForBufr has previously returned true for
  	//this event.
 
  	if(!event.waitingOnContinuesData()){
    	//Don't delay a observation that is explicit asked for.
    	//A SYNOP that is explicit asked for has a callback.

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
    
  	app.removeWaiting(info->wmono(), event.obstime(), con);
  
  	LOGINFO("ReadData: wmono: " <<info->wmono() << " obstime: " <<
			  event.obstime() << " # " << data.size());
  
  	try{
    	loadBufrData(data, bufrData, event.stationInfo());
  	}
  	catch(...){
    	LOGDEBUG("EXCEPTION(Unknown): Unexpected exception from "<< endl <<
	     		   "BufrWorker::loadBufrData" << endl);
  	}
  
  	CIDataElementList it=bufrData.begin();
  	ost.str("");

  	for(int i=0;it!=bufrData.end(); i++, it++)
    	ost << it->time() << "  [" << i << "] " << bufrData[i].time() <<endl;
  
  	LOGINFO("# number of bufrdata: " << bufrData.size() << endl<<
   		  "Continues: " << bufrData.nContinuesTimes() << endl <<
	   	  "Time(s): " << endl << ost.str());
  
  	if( app.getSavedBufrData(info->wmono(), event.obstime(), tblBufrList, *con)){
  	   if(tblBufrList.size()>0){
  	      ccx=tblBufrList.front().ccx();
  	      oldcrc=tblBufrList.front().crc();
  	      ++ccx;
  	      LOGDEBUG("An BUFR messages for wmono: " << info->wmono() << " obstime: " << event.obstime()
  	               << " allready exist. ccx=" << ccx-1 << " crc: " << oldcrc );
  	   }
  	}

  	LOGDEBUG6(bufrData);

  	bool bufrOk;

  	try{

    	bufrOk=bufrEncoder.doBufr( info,
    	                           bufrData,
    	                           bufr,
    	                           true);
  	}
  	catch(std::out_of_range &e){
    	LOGWARN("EXCEPTION: out_of_range: wmono: " << info->wmono() <<
	    		  " obstime: "  <<
	    		  ((bufrData.begin()!=bufrData.end())?
					bufrData.begin()->time():"(NULL)") << endl <<
	    		  "what: " << e.what() << endl);
    	bufrOk=false;
    	swmsg << "Cant create a bufr!" << endl;
  	}
  	catch(DataListEntry::TimeError &e){
     	LOGWARN("Exception: TimeError: wmono: " << info->wmono() << " obstime: "  <<
    			  ((bufrData.begin()!=bufrData.end())?
				  bufrData.begin()->time():"(NULL)") << endl<<
				  "what: " << e.what() << endl);
    	bufrOk=false;
    	swmsg << "Cant create a bufr!" << endl;
  	}
  	catch(...){
    	LOGWARN("EXCEPTION(Unknown): Unexpected exception in Bufr::doBufr:" <<
	    		  endl << "wmono: " << info->wmono() << " obstime: "  <<
	   		  ((bufrData.begin()!=bufrData.end())?
				  bufrData.begin()->time():"(NULL)") << endl);
    	bufrOk=false;
    	swmsg << "Cant create a bufr!" << endl;
  	}
  
  	if(!bufrOk){
    	if(event.hasCallback()){
      	event.isOk(false);
      
      	if(bufrData.size()==0)
				event.msg() << "NODATA:(" << event.obstime() <<") cant create bufr!";
      	else
				event.msg() << "BUFR ERROR:(" << event.obstime() <<") cant create bufr!";
    	}

    	LOGERROR("Cant create SYNOP for <"<< info->wmono()<<"> obstime: " <<
	     		   event.obstime());
    	swmsg << "Cant create a bufr!" << endl;
  	}else{
    	boost::uint16_t       crc=bufr.crc();
    	bool newBufr( crc != oldcrc );
    

    	//Bufr::replaceCCCXXX(sBufr, ccx);

    	if(newBufr){
      	miTime createTime(miTime::nowTime());
      	ostringstream dataOst;

      	bufrData.writeTo( dataOst );
      
      	if(app.saveBufrData( TblBufr( info->wmono(), event.obstime(),
				 				 		         createTime, crc, ccx, dataOst.str() ),
			   					 	*con)) {
				LOGINFO("BUFR information saved to database! ccx: " << ccx << " crc: " << crc );
      	} else {
      	   LOGERROR("FAILED to svae BUFR information to the database! ccx: " << ccx << " crc: " << crc );
      	}

      	saveTo(info, bufr, ccx);
      
      	swmsg << "New bufr created!" << endl;

    	}else{
     		LOGINFO("DUPLICATE: wmono=" << info->wmono() << " obstime: " 
       			 << event.obstime());
      
     		swmsg << "Duplicate bufr created!" << endl;
      
     		if(event.hasCallback())
				event.msg() << "DUPLICATE: wmono=" << info->wmono() << " obstime: " 
		    					<< event.obstime();
      		
    	}

    	ost.str("");
    	
    	if(event.hasCallback()){
      	//If we have a callback registred. Return the bufr
      	event.bufr(sBufr);
      	event.isOk(true);
    	}
  	}
}

BufrWorker::EReadData
BufrWorker::readData(dnmi::db::Connection &con,
		      			 ObsEvent             &event,
		      			 DataEntryList        &data)const
{
  	ostringstream                   ost;
  	kvDbGate                        gate(&con);
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

  	from.addHour(-25);

  	stIDs=station->stationID();
  	itStId=stIDs.rbegin();

  	if(itStId==stIDs.rend()){
    	LOGERROR("No stationid's for station <" << station->wmono() << ">!");
    	return RdNoStation;
  	}
  
  	for(;itStId!=stIDs.rend(); itStId++){
   	ost.str("");
    	ost << " where stationid=" << *itStId
			 << " AND " 
			 << " obstime>=\'"      << from.isoTime() << "\' AND "
	  		 << " obstime<=\'"      << to.isoTime()   << "\'"
			 << " order by obstime;";

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
  		LOGDEBUG("checkTypes: No data for: wmono: " << stInfo->wmono() << " obstime: " << obstime);
    	return false;
  	}

  	if(dit->obstime()!=obstime){
  		LOGDEBUG("checkTypes: No data for obstime: " << obstime <<  " wmono: " << stInfo->wmono());
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
  	
  	ost << "checkTypes: mustHaveTypes:";
  	
  	for(it=tids.begin();it!=tids.end(); it++)
  		ost << " " << *it; 		
  	
  	LOGDEBUG(ost.str());
  
  	for(it=tids.begin();it!=tids.end(); it++){
    	if(dit->size(*it)==0)
      	break;
  	}

  	if(it==tids.end()){
    	mustHaveTypes=true;
  	}

  	return false;
}



void
BufrWorker::
saveTo( StationInfoPtr info,
        BufrData  &bufr,
        int ccx) const
{
   BufrEncoder encodeBufr( info );

   try {
      encodeBufr.encodeBufr( bufr, ccx );
      encodeBufr.saveToFile();
   }
   catch( const exception &ex ) {
      LOGERROR("Failed to encode to BUFR: wmono: " << info->wmono() << " obstime: " << bufr.time() << ". Reason: " << ex.what());
   }
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

  w=app.getWaiting(event.obstime(), info->wmono(), con);
  
  
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

    if(!app.addWaiting(w, true, con)){
      LOGERROR("Cant add a waiting element to the Waiting database.");
      return true;
    }
      
    LOGINFO("Waiting on continues data: wmono: " << info->wmono() <<
	    " obstime: " << event.obstime() << " delay: " << w->delay());

    swmsg << "Waiting on continues data until: " << w->delay();

    return false;
  }

  return true;
}
