/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufrCltApp.cc,v 1.6.2.3 2007/09/27 09:02:23 paule Exp $

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
#include <dnmithread/mtcout.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <iostream>
#include "kvbufrd.hh"
#include <list>
#include <stdio.h>
#include "kvbufrCltApp.h"
#include "kvbufrCorbaThread.h"
#include "kvbufrCltBufrcbImp.h"

using namespace std;
using namespace CorbaHelper;

namespace{
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();

  omni_mutex mutex;
}


BufrCltApp *BufrCltApp::app =0;

BufrCltApp::BufrCltApp(int argn, char **argv, miutil::conf::ConfSection *conf )
  :corbaThread(0),bufrcb(kvbufrd::bufrcb::_nil()), shutdown_(false),capp(0),
   bufr(kvbufrd::bufr::_nil())
{
  BufrcbImpl *bufrImpl;
  struct timespec spin;

  spin.tv_sec=0;
  spin.tv_nsec=10000000;

  if(!getOptions(argn, argv, conf, opt)){
    cerr << "Inavlid or missing option!";
    use(1);
  }

  if(!app)
    app=this;

  setSigHandlers();
  
  try{
    corbaThread = new BufrCltCorbaThread(argn, argv);
  }
  catch(...){
    CERR("FATAL: failed to initialize KVALOBS service interface!!");
    exit(2);
  }
  
  while(!corbaThread->isInitialized())
    nanosleep(&spin, 0);

  capp=CorbaApp::getCorbaApp();
 
  try{
    bufrImpl=new BufrcbImpl(*this);
  }
  catch(...){
    CERR("FATAL: NOMEM, failling to initialize the application!");
    exit(2);
  }

  try{
    PortableServer::ObjectId_var id; 
    id=CorbaApp::getCorbaApp()->getPoa()->activate_object(bufrImpl);
    bufrcb=bufrImpl->_this();
  }
  catch(...){
    CERR("FATAL: Failling to initialize the application! (CORBA)");
    exit(2);
  }

  capp->setNameservice(opt.nshost);

  string name=opt.kvserver;
  
  if(name.empty()){
    CERR("FATAL: No kvserver given!");
    exit(1);
  }

  if(name[name.length()-1]!='/')
    name+="/";

  name+="kvbufrd";

  CORBA::Object_ptr obj=capp->getObjFromNS(name);

  CERR("INFO: Looking up <" << name << "> in nameserver at <" << 
       capp->getNameservice() << ">!\n");
  
  if(CORBA::is_nil(obj)){
    CERR("FATAL: Failed too look up <" << name << "> in CORBA nameserver!\n");
    doShutdown();
    exit(2);
  }

  bufr=kvbufrd::bufr::_narrow(obj);
  
  if(CORBA::is_nil(bufr)){
    CERR("FATAL: Cant downcast the object from the CORBA nameserver!\n");
    exit(2);
  }

  try{
    bufr->ping();
  }
  catch(...){
    CERR("FATAL: kvbufrd is down!\n");
    doShutdown();
    exit(2);
  }
  
}
 
BufrCltApp::~BufrCltApp()
{
  doShutdown();
}


void
BufrCltApp::
doShutdown()
{
  omni_mutex_lock lock(mutex);

  CorbaHelper::CorbaApp *capp;
  
  shutdown_=true;

  if(!corbaThread){
    //CERR("The CORBA subsystem is allready shutdown!" <<endl );
    return;
  }

  capp=CorbaHelper::CorbaApp::getCorbaApp();
  
  capp->getOrb()->shutdown(true);
  corbaThread->join(0);
  //delete corbaThread_; //This cause a segmentation fault
  corbaThread=0;
  //CERR("AFTER: join\n");
}

bool 
BufrCltApp::
shutdown()const
{
  omni_mutex_lock lock(mutex);

  return shutdown_ || sigTerm;
}

bool 
BufrCltApp::
wait()const
{
  omni_mutex_lock lock(mutex);

  return wait_;
}

void 
BufrCltApp::
wait(bool w)
{
 omni_mutex_lock lock(mutex);
 
 wait_=w;
}

void
BufrCltApp::
run()
{
  while(!shutdown())
    sleep(1);
}


bool 
BufrCltApp::
uptime(miutil::miTime &startTime, long &uptime_)
{
  CORBA::String_var startTime_;
  CORBA::Long       tmpTime;

  try{
    if(!bufr->uptime(startTime_, tmpTime))
      return false;
  }
  catch(...){
    return false;
  }
   
  cerr << "tmpTime: " << tmpTime << endl; 

  startTime=miutil::miTime(startTime_);
  uptime_=static_cast<long>(tmpTime);
  return true;
}

bool
BufrCltApp::
stationsList(kvbufrd::StationInfoList &infoList)
{
  kvbufrd::StationInfoList_var list;

  try{
    if(!bufr->stations(list))
      return false;
  }
  catch(...){
    return false;
  }

  infoList=list;
  return true;
}

bool
BufrCltApp::
delayList(kvbufrd::DelayList &delayList, miutil::miTime &nowTime)
{
  kvbufrd::DelayList_var list;
  CORBA::String_var       t;
  
  try{
    if(!bufr->delays(t, list))
      return false;
    
    nowTime=miutil::miTime(t);
  }
  catch(...){
    return false;
  }

  delayList=list;
  return true;
}

kvbufrd::ReloadList*
BufrCltApp::
cacheReloadList(std::string &msg)
{
  CORBA::String_var        msg_;
  kvbufrd::ReloadList_var ll;

  try{
    if(!bufr->reloadCache("", ll, msg_)){
      msg=msg_;
      return 0;
    }

    msg=msg_;
  }
  catch(...){
    COUT("EXCEPTION: cacheReloadList!"); 
    return false;
  }
    
  return ll._retn();
}

bool
BufrCltApp::
createBufr(int wmono,
	    const miutil::miTime &obstime,
	    const TKeyVal &keyvals,
	    int timeoutInSeconds,
	    kvbufrd::BufrData &res)
{
  time_t t;
  time_t tt;
  struct timespec spin;
  micutil::KeyValList keyVals;
  CITKeyVal it=keyvals.begin();

  spin.tv_sec=0;
  spin.tv_nsec=10000000;
  
  wait(false);

  keyVals.length(keyvals.size());
  
  for(CORBA::Long i=0; it!=keyvals.end(); it++, i++){
    keyVals[i].key=CORBA::string_dup(it->first.c_str());
    keyVals[i].val=CORBA::string_dup(it->second.c_str());
  }

  try{
    if(!bufr->createBufr(wmono, obstime.isoTime().c_str(),
			   keyVals, bufrcb)){
      CERR("Cant create SYNOP for station <" << wmono << ">!");
      return false;
    }
  }
  catch(...){
    CERR("Cant connect to <kvbufrd>!");
    return false;
  }

  wait(true);

  time(&t);
  tt=t;

  while((tt-t)<timeoutInSeconds){
    if(wait() && !shutdown()){
      nanosleep(&spin, 0);
      time(&tt);
    }else
      break;
  }

  if(wait())
    return false;

  res=result;
  
  return true;
}



bool 
BufrCltApp::reloadConf()
{
  CORBA::String_var msg;

  try{
    if(!bufr->reloadConf(msg)){
      CERR(msg << endl);
      return false;
    }
  }
  catch(...){
    CERR("Cant connect to kvbufrd!" << endl);
    return false;
  }
   
  CERR(msg<<endl); 
  return true; 
}

bool
BufrCltApp::getOptions(int argn, char **argv, miutil::conf::ConfSection *conf, Options &opt)
{
	struct option long_options[]={{"list-stations", 0, 0, 0},
			{"uptime", 0, 0, 0},
			{"help", 0, 0, 0},
			{"bufr", 0, 0, 0},
			{"delay-list", 0, 0, 0},
			{"reload", 0, 0, 0},
			{"cachereload", 0, 0, 0},
			{0,0,0,0}};
	int c;
	int index;
	bool hasTime=false;
	std::string sWmo;

	miutil::conf::ValElementList valElem;

	valElem=conf->getValue("corba.nameserver");

	if(valElem.empty()){
		CERR("No nameserver <corba.nameserver> in the configurationfile!");
		exit(1);
	}

	opt.nshost=valElem[0].valAsString();

	if(opt.nshost.empty()){
	  CERR("The key <corba.nameserver> in the configurationfile has no value!");
	  exit(1);
	}
  
	valElem=conf->getValue("corba.kvpath");

	if(valElem.empty())
		valElem=conf->getValue("corba.path");
	
	if( valElem.empty() ) {
		CERR("Either the <corba.kvpath> or <corba.path> has a value in the configuration file.");
		exit(1);
	}

	opt.kvserver = valElem[0].valAsString();

	if( opt.kvserver.empty() ) {
		CERR("Either the <corba.kvpath> or <corba.path> has a value in the configuration file.");
		exit(1);
	}
  
	while(true){
		c=getopt_long(argn, argv, "s:n:t:", long_options, &index);

		if(c==-1)
			break;

		switch(c){
		case 's':
			opt.kvserver=optarg;
			break;
		case 'n':
			opt.nshost=optarg;
			break;
		case 't':
			int y, m, d, h;
			if(sscanf(optarg, "%d-%d-%d %d", &y, &m, &d, &h)!=4){
				CERR("Invalid timespec: " << optarg<< endl);
				use(1);
			}
      
			opt.time=miutil::miTime(y, m, d, h);
			hasTime=true;
			break;
		case 0:
			if(strcmp(long_options[index].name,"list-stations")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::StationList;
				CERR("list-stations!" << endl);
			}else if(strcmp(long_options[index].name,"uptime")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Uptime;
				CERR( "uptime!" << endl );
			}else if(strcmp(long_options[index].name,"help")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Help;
				use(0);
			}else if(strcmp(long_options[index].name,"bufr")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Bufr;

				CERR("bufr!" << endl);
			}else if(strcmp(long_options[index].name,"delay-list")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Delays;
				CERR("delay-list!" << endl );

			}else if(strcmp(long_options[index].name,"reload")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Reload;
				CERR( "Reload!" << endl );
			}else if(strcmp(long_options[index].name, "cachereload")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::CacheReload;
				CERR( "CacheReload!" << endl );
			}

			break;
		case '?':
			CERR( "Unknown option : <" << (char)optopt << "> unknown!" << endl);
			return false;
			break;
		case ':':
			CERR( optopt << " missing argument!" << endl);
			return false;
			break;
		default:
			CERR("?? option caharcter: <" << (char)optopt << "> unknown!" << endl);
			return false;
		}
	}

	if(optind<argn){
		while(optind<argn){
			sWmo=argv[optind++];
      
			for(std::string::size_type i=0; i<sWmo.length(); i++){
				if(!isdigit(sWmo[i])){
					return false;
				}
			}
      
			opt.wmonoList.push_back(atoi(sWmo.c_str()));
		}
	}

	if(opt.cmd==Options::Undef){
		opt.cmd=Options::Bufr;
	}

	if(opt.cmd==Options::Bufr){
		if(opt.wmonoList.empty()){
			CERR("No wmono to create SYNOP for!\n");
			return false;
		}
    
		if(!hasTime){
			CERR("No time specified!\n");
			return false;
		}
	}
   
	return true;
}
      

/**
 * kvbufrclt [options] wmono, wmono, ....
 * options:
 *      -s kvalobsserver, ex kvtest, kvalobs 
 *      -n nameserver, the host name to the nameserver ex localhost, corbans
 *      -t 'YYYYY-MM-DD HH', timespec in iso format, ex 2004-01-25 15
 *      -h print the help screen and exit.
 */
void
use(int exitcode)
{
  cerr << "Use\n\n"
       << "    kvbufrclt [-n host] [-s kvserver] [CMDS]  \n\n"
       << "       -s kvserver : use the kvalobs server 'kvserver'.\n"
       << "          'kvserver' is the name of the kvalobsserver as it is\n"
       << "          known in the CORBA nameserver.\n"
       << "          Default kvserver is 'kvalobs'.\n"
       << "       -n host :    use the CORBA nameserver at host.\n"
       << "          Default host is 'corbans'!\n"
       << "\n"    
       << "    CMDS\n\n"
       << "     --list-stations: list the stations that is configured in\n"
       << "                      the kvbufrd.\n"
       << "     --delay-list:  list the stations in the dely que.\n"
       << "     --uptime: returns when kvbufrd was started.\n"
       << "     --help: print this help screen!\n"
       << "     --bufr [OPTIONS] wmono wmono .... wmono\n\n"
       << "       OPTIONS\n\n"
       << "       -t 'YYYY-MM-DD HH': Create the bufr for this time.\n\n"
       << "       wmono: wmo number of the station we shall generate a bufr\n"
       << "              for. There can be multiple wmono's.\n"
       << "     --reload: Update the station configrations from the\n"
       << "               configuration file."
       << "     --cachereload: list all stations marked for reload."
       << "\n\n";
  exit(exitcode);
}


namespace{
  void 
  sig_term(int)
  {
    sigTerm=1;
  }
  
  
  void
  setSigHandlers()
  {
    struct sigaction act, oldact;
    
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      CERR("Can't install signal handler for SIGTERM\n");
      exit(2);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      CERR("Can't install signal handler for SIGTERM\n");
      exit(2);
    }
  }
  
}

