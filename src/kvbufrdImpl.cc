/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufrdImpl.cc,v 1.9.2.3 2007/09/27 09:02:23 paule Exp $

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
#include <milog/milog.h>
#include <iostream>
#include "App.h"
#include "StationInfo.h"
#include "kvbufrdImpl.h"

using namespace std;


kvBufrdImpl::
kvBufrdImpl(App &app_, dnmi::thread::CommandQue &que_)
  :app(app_), que(que_)
{
}

kvBufrdImpl::
~kvBufrdImpl()
{
}


CORBA::Boolean 
kvBufrdImpl::
createBufr(CORBA::Short wmono,
	    const char* obstime, 
	    const micutil::KeyValList& keyVals,
	    kvbufrd::bufrcb_ptr callback)
{
  StationInfoPtr info=app.findStationInfoWmono(wmono);
  StationInfo    *pInfo;
  ObsEvent       *event;
  miutil::miTime time(obstime);

  milog::LogContext context("BufrdImpl");

  if(time.undef()){
    LOGERROR("obstime: " << obstime << " Invalid!");
    return false;
  }
    

  if(!info){
    LOGINFO("No configuration entry for <" << wmono << ">!");
    return false;
  }

  
  try{
    pInfo=new StationInfo(*info);
  }
  catch(...){
    LOGFATAL("NOMEM: copy of StationInfo!");
    return false;
  }

  //CODE:
  //Use keyVals to overide values in StationInfo 

  try{
    event=new ObsEvent(time, 
		       StationInfoPtr(pInfo), 
		       kvbufrd::bufrcb::_duplicate(callback));
  }
  catch(...){
    LOGFATAL("NOMEM!");
    return false;
  }

  que.postAndBrodcast(event);

  return true;
}

CORBA::Boolean 
kvBufrdImpl::
stations(kvbufrd::StationInfoList_out infoList)
{
  try{
    infoList=new kvbufrd::StationInfoList();
  }
  catch(...){
    return false;
  }

  return app.listStations(*infoList);

}

CORBA::Boolean 
kvBufrdImpl::
uptime(CORBA::String_out startTime, 
       CORBA::Long& uptimeInSeconds)
{
  milog::LogContext context("BufrdImpl");
  std::string t=app.startTime().isoTime();
  startTime=CORBA::string_dup(t.c_str());
  miutil::miTime now=miutil::miTime::nowTime();

  uptimeInSeconds=miutil::miTime::secDiff(now, app.startTime());

  return true;
}

CORBA::Boolean
kvBufrdImpl::
delays(CORBA::String_out nowTime, 
       kvbufrd::DelayList_out dl)
{
  milog::LogContext context("BufrdImpl");
  miutil::miTime      now;
  
  dl=app.getDelayList(now);
  
  if(!dl)
    return false;

  nowTime=CORBA::string_dup(now.isoTime().c_str());
  return true;
}

CORBA::Boolean 
kvBufrdImpl::
reloadConf(CORBA::String_out message)
{
  std::list<StationInfoPtr>                 stList;
  std::list<StationInfoPtr>::const_iterator it;
  std::ostringstream                        ost;
  int                                       newStations=0;
  int                                       updatedStations=0;

  milog::LogContext context("BufrdImpl");

  LOGDEBUG1("reloadConf called! " << endl);

  if(!app.readStationInfo(stList)){
    message=CORBA::string_dup("ERROR: Cant read the configuration file!");
    return false;
  }

  for(it=stList.begin(); it!=stList.end(); it++){
    StationInfoPtr st=app.findStationInfoWmono((*it)->wmono());
    
    if(!st){
      if(app.addStationInfo(*it)){
	newStations++;

	LOGINFO("Adding station <"<<(*it)->wmono() << ">!" << endl);
	ost << "Adding station <" << (*it)->wmono() << ">!" << endl;
      }else{
	LOGINFO("FAILED: Adding station <"<<(*it)->wmono() << ">!" << endl);
	ost << "FAILED: Adding station <" << (*it)->wmono() << ">!" << endl;
      }

    }else{
      if(!st->equalTo(*(*it))){
	(*it)->delayUntil(st->delayUntil());
	StationInfoPtr oldInfo=app.replaceStationInfo(*it);

	if(!oldInfo){
	  LOGINFO("FAILED: Updatting station <" << (*it)->wmono() << ">!" 
		  << endl);
	  ost <<"FAILED: Updatting station <" << (*it)->wmono() << ">!" 
	      << endl;
	}else{
	  updatedStations++;
	  
	  LOGINFO("Updatting station <" << (*it)->wmono() << ">!" << endl);
	  ost <<"Updatting station <" << (*it)->wmono() << ">!" << endl;
	  ost << "Old stationinfo: " << endl << *oldInfo << endl;
	  ost << "New stationinfo: " << endl << **it << endl;
	}
      }
    }
  }
  
  if(updatedStations>0 || newStations>0){
    ost << "-----------------------------------------" << endl;
    ost << "newStations: " << newStations << endl;
    ost << "updatedStations: " << updatedStations << endl;
  }else{
    ost << endl << "No changes!" << endl << endl;
  }

  message=CORBA::string_dup(ost.str().c_str());
  return true;
}


CORBA::Boolean 
kvBufrdImpl::
reloadCache(const char* fromTime, 
	    kvbufrd::ReloadList_out wmonolist,
	    CORBA::String_out message)
{
  milog::LogContext context("BufrdImpl");
  LOGINFO("reloadCache called!"<<endl);

  wmonolist=app.listCacheReload();
 
  if(!wmonolist){
    wmonolist=new kvbufrd::ReloadList();
    message=CORBA::string_dup("kvBufrdImpl::reloadCache Failed");
    return false;
  }

  message=CORBA::string_dup("OK!");
 
  return true;
}


CORBA::Boolean 
kvBufrdImpl::
getbufr(const kvbufrd::WmoNoList& wmoList,
	 const char* fromtime, const char* totime, 
	 kvbufrd::BufrList_out bufrs,
	 CORBA::String_out message)
{

  bufrs=new kvbufrd::BufrList();
  message=CORBA::string_dup("kvBufrdImpl::getbufr: Not implemented");
  return false;
 
}
 

CORBA::Boolean 
kvBufrdImpl::
getdata(CORBA::Short wmono, const char* obstime, 
	kvbufrd::DataElementList_out datalist,
	CORBA::String_out message)
{
  datalist=new kvbufrd::DataElementList();
  message=CORBA::string_dup("kvBufrdImpl::getdata: Not implemented");
  return false;
}
