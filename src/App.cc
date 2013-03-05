/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: App.cc,v 1.19.2.14 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include <ctype.h>
#include <list>
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include <milog/milog.h>
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvPath.h>
#include "kvDbGateProxy.h"
#include "obsevent.h"
//#include "ValidData.h"
#include "tblWaiting.h"
#include "Data.h"
#include "App.h"
#include "StationInfoParse.h"
#include "tblKeyVal.h"
#include "getDataReceiver.h"
#include "GetDataThread.h"
#include <kvalobs/kvPath.h>
#include "parseMilogLogLevel.h"

using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace milog;
using namespace miutil::conf;
using boost::mutex;
using namespace kvservice;
namespace fs = boost::filesystem;


namespace {

void createDirectory( const fs::path &path ) {
   try {
      if( ! fs::exists( path  ) ) {
         fs::create_directories( path );
      } else if ( ! fs::is_directory( path )  ) {
         LOGFATAL( "The path <" << path << "> exist, but is NOT a directory.");
         cerr << "The path <" << path << "> exist, but is NOT a directory.";
         exit( 1 );
      }
   }
   catch( const fs::filesystem_error &ex ) {
      LOGFATAL( "The path <" << path << "> does not exist and cant be created." <<
                "Reason: " << ex.what());
      cerr << "The path <" << path << "> does not exist and cant be created."
           <<  "Reason: " << ex.what() << endl;
      exit( 1 );
   }

}

class MyConnectionFactory : public kvalobs::ConnectionFactory
{
   App *app;

public:
   MyConnectionFactory( App *app_ )
      : app( app_ )
   {
   }

   virtual dnmi::db::Connection* newConnection()
      {
         return app->createDbConnection();
      }

   virtual void releaseConnection(dnmi::db::Connection *con )
      {
         app->releaseDbConnection( con );
      }
};
}

bool
App::
createGlobalLogger(const std::string &id, milog::LogLevel ll)
{
   try{

      if( ll == milog::NOTSET )
         ll = defaultLogLevel;

      /*FIXME: Remove the comments when the needed functionality is
       * in effect on an operational machin.
       *
       *if( LogManager::hasLogger(id) )
       *  return true;
       */
      FLogStream *logs=new FLogStream(2, 204800); //200k
      std::ostringstream ost;
      fs::path logpath = fs::path(kvPath("logdir")) / options.progname / (id + ".log");

      if(logs->open( logpath.string()) ){
         logs->loglevel( ll );
         if(!LogManager::createLogger(id, logs)){
            delete logs;
            return false;
         }

         return true;
      }else{
         LOGERROR("Cant open the logfile <" << logpath << ">!");
         delete logs;
         return false;
      }
   }
   catch(...){
      LOGERROR("Cant create a logstream for LOGID " << id);
      return false;
   }
}

App::
App(int argn, char **argv,   
    const std::string &confFile_, miutil::conf::ConfSection *conf):
    kvservice::corba::CorbaKvApp(argn, argv, conf),
    startTime_(miutil::miTime::nowTime()),
    confFile(confFile_),
    hasStationWaitingOnCacheReload(false),
    acceptAllTimes_(false),
    defaultLogLevel( milog::INFO )
{
   ValElementList valElem;
   string         val;
   string         bufr_tables( DATADIR );
   bool           bufr_tables_names( false );

   LogContext context("ApplicationInit");

   createDirectory( fs::path(kvPath("logdir")) / options.progname );

   valElem=conf->getValue("loglevel");

   if( !valElem.empty() ) {
      std::string slevel=valElem[0].valAsString();
      milog::LogLevel ll = parseMilogLogLevel( slevel );

      if( ll != milog::NOTSET )
         defaultLogLevel = ll;
   }

   milog::LogManager *manager = milog::LogManager::instance();

   if( manager ) {
      manager->loglevel( defaultLogLevel );
      milog::Logger &logger=milog::Logger::logger();
      logger.logLevel( defaultLogLevel );
   }

   createGlobalLogger("GetData");
   createGlobalLogger("DelayCtl");
   createGlobalLogger("main");
   createGlobalLogger("uinfo0");


   //If a station is set up with this types delay them if
   //they has not at least 4 hours with data.
   continuesTypeID_.push_back( 311 );
   continuesTypeID_.push_back( 310 );
   continuesTypeID_.push_back( 3 );
   continuesTypeID_.push_back( 330 );

   valElem=conf->getValue("bufr_tables");

   if( !valElem.empty() )
      bufr_tables=valElem[0].valAsString();

   if( ! bufr_tables.empty() ) {
      if( bufr_tables[0] != '/' ) {
         LOGERROR( "bufr_tables must be an absolute path '" << bufr_tables << "'.");
      }

      if( *bufr_tables.rbegin() != '/')
         bufr_tables += '/';
   }

   valElem=conf->getValue("bufr_tables_names");

   if( !valElem.empty() ) {
      string t=valElem[0].valAsString();

      if(!t.empty() && (t[0]=='t' || t[0]=='T'))
         bufr_tables_names = true;
   }


   IDLOGINFO("main", "BUFR_TABLES=" << bufr_tables );

   setenv( "BUFR_TABLES", bufr_tables.c_str(), 1 );

   if( getenv( "BUFR_TABLES" ) ) {
      LOGINFO("BUFR_TABLES='" << getenv( "BUFR_TABLES") << "'." );
   } else {
      LOGINFO("Failed to set BUFR_TABLES.");
   }

   if( bufr_tables_names ) {
      IDLOGINFO("main", "BUFR_TABLE_NAMES=true" );
      setenv( "PRINT_TABLE_NAMES", "true", 1 );
   } else {
      IDLOGINFO("main", "BUFR_TABLE_NAMES=false" );
      setenv( "PRINT_TABLE_NAMES", "false", 1 );
   }

   valElem=conf->getValue("accept_all_obstimes");

   if(!valElem.empty()){
      string t=valElem[0].valAsString();

      if(!t.empty() && (t[0]=='t' || t[0]=='T'))
         acceptAllTimes_=true;
   }

   if(acceptAllTimes_){
      IDLOGINFO("main", "Accepting all obstimes.");
   }else{
      IDLOGINFO("main", "Rejecting obstimes that is too old or to early.");
   }

   valElem=conf->getValue("database.driver");

   if( valElem.empty() ) {
      LOGFATAL("No <database.driver> in the configurationfile!");
      exit(1);
   }

   dbDriver=valElem[0].valAsString();

   LOGINFO("Loading driver for database engine <" << dbDriver << ">!\n");

   if(!dbMgr.loadDriver(dbDriver, dbDriverId)){
      LOGFATAL("Can't load driver <" << dbDriver << endl
               << dbMgr.getErr() << endl
               << "Check if the driver is in the directory $KVALOBS/lib/db???");

      exit(1);
   }

   valElem=conf->getValue("database.dbconnect");

   if(valElem.empty()){
      LOGFATAL("No <database.dbconnect> in the configurationfile!");
      exit(1);
   }

   dbConnect=valElem[0].valAsString();

   LOGINFO("Connect string <" << dbConnect << ">!\n");

   valElem=conf->getValue("corba.kvpath");

   if(valElem.empty() || valElem[0].valAsString().empty() ){
      mypathInCorbaNS=nameserverpath;
   }else{
      mypathInCorbaNS=valElem[0].valAsString();
   }

   LOGINFO("My path in the CORBA nameserver: " << mypathInCorbaNS );

   if(!mypathInCorbaNS.empty() ) {
      if( *mypathInCorbaNS.rbegin() != '/')
         mypathInCorbaNS+='/';
      if( *mypathInCorbaNS.begin() == '/')
         mypathInCorbaNS.erase(0,1);
   }

   if(!readStationInfo(conf)){
      LOGFATAL("Exit! No configuration!");
      exit(1);
   }

   dbThread = boost::shared_ptr<KvDbGateProxyThread>( new KvDbGateProxyThread( boost::shared_ptr<MyConnectionFactory>( new MyConnectionFactory( this ) ) ));
   dbThread->start();

   readWaitingElementsFromDb();

   //We dont need conf any more.
   delete conf;
}

App::
~App()
{
}


bool
App::
initKvBufrInterface(  dnmi::thread::CommandQue &newObsQue )
{
   kvBufrdImpl *bufrdImpl;

   try{
      bufrdImpl=new kvBufrdImpl( *this, newObsQue);
      PortableServer::ObjectId_var id = getPoa()->activate_object(bufrdImpl);

      bufrRef = bufrdImpl->_this();
      IDLOGINFO( "main", "CORBAREF: " << corbaRef(bufrRef) );
      std::string nsname = "/" + mypathInCorbaNameserver();
      nsname += options.progname;
      IDLOGINFO( "main", "CORBA NAMESERVER (register as): " << nsname );
      putObjInNS(bufrRef, nsname);
   }
   catch( const std::bad_alloc &ex ) {
      LOGFATAL("NOMEM: cant initialize the aplication!");
      return false;
   }
   catch(...){
      IDLOGFATAL("main","CORBA: cant initialize the aplication!");
      return false;
   }

   return true;
}


void 
App::
readWaitingElementsFromDb()
{
   list<TblWaiting> data;
   list<TblWaiting>::iterator it;


   kvDbGateProxy gate( dbThread->dbQue );
   gate.busytimeout(120);

   if(gate.select(data, " order by delaytime")){
      for(it=data.begin(); it!=data.end(); it++){
         StationInfoList info=findStationInfoWmono( it->wmono() );

         if( info.empty() ) {
            gate.remove(*it);
            continue;
         }

         for( StationInfoList::iterator itInfo = info.begin();
               itInfo != info.end(); ++itInfo ) {
            try{
               waitingList.push_back(WaitingPtr(new Waiting(it->delaytime(),
                                                            it->obstime(),
                                                            *itInfo)));
            }
            catch(...){
               LOGFATAL("NOMEM: while reading 'waiting' from database!");
               exit(1);
            }
         }
      }
   }else{
      LOGERROR("ERROR (Init): While reading 'waiting' from database!" << endl
               << gate.getErrorStr());
   }
}  

void           
App::
continuesTypeID(const std::list<int> &continuesTimes)
{
   mutex::scoped_lock lock(mutex);
   continuesTypeID_=continuesTimes;
}

std::list<int> 
App::
continuesTypeID()
{ 
   mutex::scoped_lock lock(mutex);

   return continuesTypeID_;
}


/**
* Return true if the station has only typeid that is NOT in the list
* continuesTypeID.
*/
bool           
App::
onlyNoContinuesTypeID(StationInfoPtr st)
{
   mutex::scoped_lock lock(mutex);

   StationInfo::TLongList tp=st->typepriority();
   StationInfo::ITLongList itp;

   for(list<int>::iterator it=continuesTypeID_.begin();
         it!=continuesTypeID_.end(); it++){

      for(itp=tp.begin(); itp!=tp.end(); itp++){
         if(*it==*itp)
            return false;
      }
   }

   return true;
}


bool 
App::
isContinuesType(int typeID)
{
   mutex::scoped_lock lock(mutex);

   for(list<int>::iterator it=continuesTypeID_.begin();
         it!=continuesTypeID_.end(); it++){

      if(*it==typeID){
         return true;
      }
   }

   return false;
}

bool
App::
readStationInfo( miconf::ConfSection *conf)
{
   StationInfoParse theParser;
   StationList tmpList;

   if(!theParser.parse(conf, tmpList)){
      LOGFATAL("Cant parse the SYNOP configuration.");
      return false;
   }

   mutex::scoped_lock lock(mutex);
   stationList=tmpList;

   return true;
}

bool
App::
readStationInfo(std::list<StationInfoPtr> &stList)const
{
   StationInfoParse theParser;

   LOGDEBUG2("Reading conf from file!" << endl <<
             "<"<<confFile<<">" <<endl);

   miutil::conf::ConfSection *conf=miutil::conf::ConfParser::parse( confFile );

   if(!conf)
      return false;

   stList.clear();

   bool ret=true;

   if(!theParser.parse(conf, stList)){
      LOGWARN("Cant parse the BUFFER configuration!" << endl
              << "File: <" << confFile << ">" << endl );
      ret = false;
   }

   delete conf;

   return ret;
}

StationList
App::
getStationList()const
{
   mutex::scoped_lock lock(mutex);

   return stationList;
}

bool
App::
listStations(kvbufrd::StationInfoList &list)
{
   ostringstream ost;

   mutex::scoped_lock lock(mutex);

   list.length(stationList.size());

   IStationList it=stationList.begin();

   for(CORBA::Long i=0; it!=stationList.end(); it++, i++){
      list[i].wmono=(*it)->wmono();
      list[i].id = (*it)->stationID();

      StationInfo::TLongList stid=(*it)->definedStationID();
      StationInfo::ITLongList itl=stid.begin();
      list[i].stationIDList.length(stid.size());

      for(int j=0; itl!=stid.end(); itl++, j++){
         list[i].stationIDList[j]=*itl;
      }

      ost.str("");
      ost << **it;

      list[i].info=CORBA::string_dup(ost.str().c_str());
   }

   return true;
}

dnmi::db::Connection*
App::
createDbConnection()
{
   dnmi::db::Connection *con;

   con=dbMgr.connect(dbDriverId, dbConnect);

   if(!con){
      LOGERROR("Can't create a database connection  ("
            << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!");
      return 0;
   }

   LOGDEBUG3("New database connection (" << dbDriverId
           << ") created!");
   return con;
}

void                  
App::
releaseDbConnection(dnmi::db::Connection *con)
{
   LOGDEBUG3("Database connection released.");
   dbMgr.releaseConnection( con );
}


StationInfoList
App::
findStationInfo(long stationid)
{
   mutex::scoped_lock lock(mutex);

   return stationList.findStation( stationid );
}


StationInfoList
App::
findStationInfoWmono(int wmono)
{
   mutex::scoped_lock lock(mutex);
   return stationList.findStationByWmono( wmono );
}




WaitingPtr
App::
addWaiting( WaitingPtr w, bool replace )
{
   IWaitingList it;

   mutex::scoped_lock lock(mutex);

   for(it=waitingList.begin(); it!=waitingList.end(); it++){
      if( *w->info() == *(*it)->info() &&
         w->obstime()==(*it)->obstime()){

         if(replace){
            if(w->delay()!=(*it)->delay()){
               LOGINFO("Replace delay element for: " << w->info()->toIdentString()
                       << " obstime: "  << w->obstime() << " delay to: "
                       << w->delay());
               *it=w;
               if( ! w->addToDb() ) {
                  LOGERROR("DBERROR while replaceing delay element for: " << w->info()->toIdentString()
                           << " obstime: "  << w->obstime() << " delay to: "
                           << w->delay());
               }
            }
         }
         return *it;
      }
   }

   LOGINFO("Add delay element for: " << w->info()->toIdentString()
           << " obstime: "  << w->obstime() << " delay to: "
           << w->delay());

   if( !w->addToDb() ) {
      LOGERROR("DBERROR while adding delay element for: " << w->info()->toIdentString()
                 << " obstime: "  << w->obstime() << " delay to: "
                 << w->delay());
   }
   //We have now record for this station and obstime in the waitingList.

   if(waitingList.empty()){
      waitingList.push_back(w);
      return WaitingPtr();
   }

   for(it=waitingList.begin(); it!=waitingList.end(); it++){
      if(w->delay()<=(*it)->delay()){
         break;
      }
   }

   waitingList.insert(it, w);

   return WaitingPtr();
}

WaitingPtr 
App::    
getWaiting( const miutil::miTime &obstime,
            StationInfoPtr info )
{
   mutex::scoped_lock lock(mutex);
   IWaitingList it;

   for(it=waitingList.begin(); it!=waitingList.end(); it++){
      if( *(*it)->info() == *info &&
         (*it)->obstime()==obstime) {
         (*it)->removeFrom();
         WaitingPtr w=*it;
         waitingList.erase(it);

         return w;
      }
   }

   return WaitingPtr();
}



WaitingList    
App::
getExpired()
{
   WaitingList  wl;
   IWaitingList it;
   IWaitingList itTmp;
   miTime       now;
   ostringstream ost;
   bool          msg=false;

   mutex::scoped_lock lock(mutex);

   milog::LogContext context("Delay");

   now=miTime::nowTime();
   it=waitingList.begin();

   while( it != waitingList.end() && (*it)->delay() <= now){
      LOGDEBUG("getExpired: loop");
      if(!msg){
         ost << "Expired delay for stations at time: " << now << endl;
         msg=true;
      }
      ost << "-- " << (*it)->info()->toIdentString() << " obstime: " << (*it)->obstime()
	       << " delay to: " << (*it)->delay() << endl;
      wl.push_back(*it);
      waitingList.erase(it);
      it=waitingList.begin();
   }

   if(msg){
      LOGINFO(ost.str());
   }

   if(!wl.empty()){
      for(it=wl.begin(); it!=wl.end(); it++){
         (*it)->removeFrom();
      }
   }

   return wl;
}

kvbufrd::DelayList*
App::
getDelayList(miutil::miTime &nowTime)
{
   kvbufrd::DelayList *dl;

   mutex::scoped_lock lock(mutex);

   nowTime=miTime::nowTime();

   try{
      dl=new kvbufrd::DelayList();
   }
   catch(...){
      return 0;
   }

   if(waitingList.empty())
      return dl;

   dl->length(waitingList.size());
   IWaitingList it=waitingList.begin();
   CORBA::Long  i;

   for(it=waitingList.begin(), i=0;
         it!=waitingList.end();
         it++, i++){

      try{
         (*dl)[i].wmono=(*it)->info()->wmono();
         (*dl)[i].id=(*it)->info()->stationID();
         (*dl)[i].obstime=(*it)->obstime().isoTime().c_str();
         (*dl)[i].delay=(*it)->delay().isoTime().c_str();
      }
      catch(...){
         break;
      }
   }

   return dl;
}


void           
App::
removeWaiting( StationInfoPtr info,
               const miutil::miTime &obstime )
{
   IWaitingList it;

   mutex::scoped_lock lock(mutex);

   for(it=waitingList.begin(); it!=waitingList.end(); it++){
      if( *info == *(*it)->info() &&
          obstime==(*it)->obstime()){

         if(!(*it)->removeFrom()){
            LOGWARN("Cant remove waiting element from database:  "
                  << (*it)->info()->toIdentString() << " obstime: " << obstime);

         }

         LOGINFO("Removed waiting element: "
               << (*it)->info()->toIdentString() << " obstime: " << obstime << endl
               << " with delay: " << (*it)->delay());

         waitingList.erase(it);

         return;
      }
   }
}

void           
App::
removeWaiting( WaitingPtr w )
{
   LOGINFO("remove: " << w->info()->toIdentString() << " obstime: " << w->obstime()
           << " delay to: " << w->delay() << " from database!");

   w->removeFrom();
}

miutil::miTime 
App::
checkpoint()
{
   list<TblKeyVal> data;
   list<TblKeyVal>::iterator it;

   kvDbGateProxy gate( dbThread->dbQue );

   gate.busytimeout(120);

   if(!gate.select(data, "WHERE key=\'checkpoint\'")){
      LOGERROR("DBERROR: cant obtain checkpoint!");
      return miTime();
   }

   if(data.empty()){
      LOGINFO("No checkpont!");
      return miTime();
   }

   return miTime(data.front().val());
}

void           
App::
createCheckpoint( const miutil::miTime &cpoint )
{
   if( cpoint.undef() ){
      LOGERROR("Checkpoint: undef checkpont time!");
      return;
   }

   kvDbGateProxy gate( dbThread->dbQue );
   gate.busytimeout(120);

   if(cpoint.undef()){
      LOGERROR("Checkpoint: undef checkpont time!");
      return;
   }

   if(!gate.insert(TblKeyVal("checkpoint", cpoint.isoTime()), true)){
      LOGERROR("Failed to create checkpoint! (" <<cpoint <<")");
   }else{
      LOGINFO("Checkpoint created at: " << cpoint);
   }
}


StationInfoPtr 
App::
replaceStationInfo(StationInfoPtr newInfoPtr)
{
   mutex::scoped_lock lock(mutex);

   IStationList it=stationList.begin();

   for(;it!=stationList.end(); it++){
      if( *(*it) == *newInfoPtr ){

         //Set the cacheReload of the new StationInfo to the same as
         //reload.
         newInfoPtr->cacheReloaded48((*it)->cacheReloaded48());
         StationInfoPtr info=*it;
         *it=newInfoPtr;
         return info;
      }
   }

   return StationInfoPtr();
}

bool
App:: 
addStationInfo(StationInfoPtr newInfoPtr)
{

   mutex::scoped_lock lock(mutex);
   IStationList it=stationList.begin();

   for(;it!=stationList.end(); it++){
      if( *(*it) == *newInfoPtr ){
         return false;
      }
   }

   stationList.push_back(newInfoPtr);

   return true;
}

/**
* Replace the current configuration with the new one.
* @param newConf The new configuration to replace the old.
*/
void
App::
replaceStationConf(const StationList &newConf )
{
   mutex::scoped_lock lock(mutex);

   stationList = newConf;
}


bool
App:: 
getSavedBufrData( StationInfoPtr info,
                  const miutil::miTime &obstime,
                  std::list<TblBufr> &tblBufr )
{
   kvDbGateProxy gate( dbThread->dbQue );
   ostringstream  ost;

   gate.busytimeout(120);

   ost << "WHERE wmono=" << info->wmono()
       << " AND id=" << info->stationID()
       << " AND callsign='" << info->callsign() << "'"
       << " AND obstime=\'" << obstime << "\'";

   if(!gate.select(tblBufr, ost.str())){
      LOGERROR("DBERROR: getSavedBufrData: " << gate.getErrorStr());
      return false;
   }

   return true;
}


bool 
App::
saveBufrData( const TblBufr &tblBufr )
{
   kvDbGateProxy gate( dbThread->dbQue );

   gate.busytimeout(120);

   if(!gate.insert(tblBufr, true)){
      LOGERROR("DBERROR: saveBufrData: " << gate.getErrorStr());
      return false;
   }

   return true;
}


bool 
App::          
getDataFrom(const miutil::miTime &t,
            int                  wmono,
            int                  hours,
            dnmi::thread::CommandQue &que)
{
   LogContext lContext("getDataFrom");

   LOGINFO("Get data from server, start time: " << t );

   GetData *getData;

   try{
      getData=new GetData(*this, t, wmono, hours, que);
   }
   catch(...){
      LOGERROR("NO MEM!");
      return false;
   }

   try{
      //Create and start a background thread to receive the
      //data from kvalobs.
      getData->setThread(new boost::thread(*getData));
   }
   catch(...){
      LOGERROR("NO MEM!");
      delete getData;
      return false;
   }

   mutex::scoped_lock lock(mutex);

   getDataThreads.push_back(getData);

   return true;
}


bool
App::
joinGetDataThreads(bool waitToAllIsJoined, const std::string &logid)
{
   mutex::scoped_lock lock(mutex);
   std::list<GetData*>::iterator it=getDataThreads.begin();
   bool   joined=false;

   IDLOGDEBUG(logid, "# " << getDataThreads.size() << " getDataThreads!");

   for(;it!=getDataThreads.end(); it++){
      if(waitToAllIsJoined){
         (*it)->join();
         delete *it;
         it=getDataThreads.erase(it);
         joined=true;
      }else{
         if((*it)->joinable()){
            (*it)->join();
            delete *it;
            it=getDataThreads.erase(it);
            joined=true;
         }
      }
   }

   return joined;
}


void 
App::
cacheReloaded( StationInfoPtr info)
{
   mutex::scoped_lock lock(mutex);

   IStationList it=stationList.begin();

   for(;it!=stationList.end(); it++){
      if( *(*it) == *info ){
         (*it)->cacheReloaded48(true);
         return;
      }
   }
}

StationList
App::
reloadCache(int wmono, int id)
{
   mutex::scoped_lock lock(mutex);
   StationList myStationList;

   IStationList it=stationList.begin();

   if(wmono < 0 ){
      for(;it!=stationList.end(); it++){
         (*it)->cacheReloaded48(false);
         myStationList.push_back(*it);
      }
   }else{
      for(;it!=stationList.end(); it++){
         if((*it)->wmono()==wmono &&
            (*it)->stationID() == id ){
            (*it)->cacheReloaded48(false);
            myStationList.push_back(*it);
            break;
         }
      }
   }

   if(!myStationList.empty()){
      hasStationWaitingOnCacheReload=true;
   }

   return myStationList;
}

kvbufrd::ReloadList*
App::
listCacheReload()
{
   mutex::scoped_lock lock(mutex);
   kvbufrd::ReloadList *retlist;

   StationList myStationList;

   try{
      retlist=new kvbufrd::ReloadList();
   }
   catch(...){
      return 0;
   }

   IStationList it=stationList.begin();

   for(;it!=stationList.end(); it++){
      if(!(*it)->cacheReloaded48()){
         myStationList.push_back(*it);
      }
   }

   if(!myStationList.empty()){
      int  count;
      retlist->length(myStationList.size());

      it=myStationList.begin();

      for(CORBA::Long i=0; it!=myStationList.end(); it++, i++){
         count=0;

         for(list<ObsEvent*>::iterator eit=obsEventWaitingOnCacheReload.begin();
               eit!=obsEventWaitingOnCacheReload.end();
               eit++){
            if( *(*eit)->stationInfo() == **it )
               count++;
         }

         (*retlist)[i].wmono=(*it)->wmono();
         (*retlist)[i].id=(*it)->stationID();
         (*retlist)[i].eventsWaiting=count;
      }
   }

   return retlist;
}


void 
App::
addObsEvent(ObsEvent *event,
            dnmi::thread::CommandQue &que)
{
   mutex::scoped_lock lock(mutex);

   if(!hasStationWaitingOnCacheReload){
      try{
         que.postAndBrodcast(event);
      }
      catch(...){
         delete event;
      }

      return;
   }

   for(IStationList it=stationList.begin();
         it!=stationList.end();
         it++){
      if( *event->stationInfo() == **it ){
         if((*it)->cacheReloaded48()){
            //The cache for the station is reloaded post the event to
            //the event que and return.
            try{
               que.postAndBrodcast(event);
            }
            catch(...){
               delete event;
            }
            return;
         }else{
            //Break out of the loop and add the event to
            //the obsEventWaitingOnCacheReload.
            break;
         }
      }
   }

   for(std::list<ObsEvent*>::iterator it=obsEventWaitingOnCacheReload.begin();
         it!=obsEventWaitingOnCacheReload.end();
         it++){
      if( (*it)->obstime()==event->obstime() &&
         *(*it)->stationInfo() == *event->stationInfo() ){
         delete *it;
         *it=event;
         return;
      }
   }

   obsEventWaitingOnCacheReload.push_back(event);
}


void 
App::
checkObsEventWaitingOnCacheReload(dnmi::thread::CommandQue &que,
                                  const std::string &logid)
{
   mutex::scoped_lock lock(mutex);

   IDLOGDEBUG(logid, "CheckObsEventWaitingOnCacheReload called!");

   if(!hasStationWaitingOnCacheReload){
      IDLOGDEBUG(logid, "No Station waiting on reload!!");
      return;
   }

   for(std::list<ObsEvent*>::iterator it=obsEventWaitingOnCacheReload.begin();
         it!=obsEventWaitingOnCacheReload.end();
         it++){

      for(IStationList sit=stationList.begin();
            sit!=stationList.end();
            sit++){
         if( *(*it)->stationInfo() == **sit ){
            if((*sit)->cacheReloaded48()){
               IDLOGINFO(logid,"The cache is reloaded for station: " <<
                         (*sit)->toIdentString() << " obstime: "  << (*it)->obstime());


               ObsEvent *event=*it;
               it=obsEventWaitingOnCacheReload.erase(it);
               //The cache for the station is reloaded post the event to
               //the event que.
               try{
                  que.postAndBrodcast(event);
               }
               catch(...){
                  LOGERROR("Cant post event to the eventque for the station: " <<
                           (*sit)->toIdentString());
                  IDLOGERROR(logid,
                             "Cant post event to the eventque for the station: " <<
                             (*sit)->toIdentString());

                  delete event;
               }
            }

            //Break out of the loop.
            break;
         }
      }
   }

   //If the list obsEventWaitingOnCacheReload is empty
   //check the stationList to see if all stations is reloaded. If so
   //set hasStationWaitingOnCacheReload to false.
   if(obsEventWaitingOnCacheReload.empty()){
      bool allReloaded=true;
      ostringstream ost;

      for(IStationList it=stationList.begin();
            it!=stationList.end();
            it++){
         if(!(*it)->cacheReloaded48()){
            ost << (*it)->toIdentString() << " ";
            allReloaded=false;
         }
      }

      if(!allReloaded){
         IDLOGDEBUG(logid, "This stations is not reloaded with new data:" << endl
                    << ost.str());
      }else{
         IDLOGINFO(logid, "All stations reloaded with data from kvalobs.");
         hasStationWaitingOnCacheReload=false;
      }
   }
}



void
decodeArgs( int argn, char **argv, Opt &opt )
{
   struct option long_options[]=
   {{"config-file", 1, 0, 'c'},
    {"pidfile", 1, 0, 'p'},
    {"fromtime", 1, 0, 'f'},
    {"help", 0, 0, 'h'},
    {0,0,0,0} };

   opt.progname = getProgNameFromArgv0( argv[0] );

   int c;
   int index;

   while(true){
      c=getopt_long(argn, argv, "hp:c:", long_options, &index);

      if(c==-1)
         break;

      switch(c){
         case 'h':
            usage( opt.progname, 1 );
            break;
         case 'p':
            opt.pidfile = optarg;
            break;
         case 'c':
            opt.conffile = optarg;
            break;
         case 'f': {
            string tmp(optarg);
            miTime fromTime;

            if( tmp.find_first_of("-:") == string::npos ) {
               int n = atoi( optarg );
               if( n > 0 ) {
                  fromTime = miTime::nowTime();
                  fromTime.addHour(-1 * n );
               }
            } else {
               fromTime = miTime( optarg );
            }

            if( fromTime.undef() ) {
               cerr << "Invalid from time '" << optarg << "'. "
                     << "Format YYYY-MM-DD hh:mm:ss or hours" << endl;
               usage( opt.progname, 1 );
            } else {
               opt.fromTime = fromTime;
            }
         }
         break;
         case '?':
            cout <<"Unknown option : <" << (char)optopt << ">!" << endl;
            cout << opt.progname << " -h for help.\n\n";
            usage( opt.progname, 1 );
            break;
         case ':':
            cout << optopt << " missing argument!" << endl;
            usage( opt.progname, 1 );
            break;
         default:
            cout << "?? option caharcter: <" << (char)optopt << "> unknown!" << endl;
            usage( opt.progname, 1 );
            break;
      }
   }

   if( opt.conffile.empty() ) {
      opt.conffile = kvPath("sysconfdir")+"/"+opt.progname +".conf";
   } else if( *opt.conffile.begin() != '/' ){
      opt.conffile = kvPath("sysconfdir")+"/"+opt.conffile;
   }

   if( opt.pidfile.empty() ) {
      opt.pidfile = dnmi::file::createPidFileName( kvPath("rundir"),
                                                   opt.progname );
   } else if( *opt.pidfile.begin() != '/') {
      opt.pidfile = kvPath("rundir") + "/" + opt.pidfile;
   }

}


void usage( const std::string &progname, int exitCode )
{
   cout << "\n " << progname << " is a program that creates BUFR message from kvalobs."
         << "\n\nUSAGE "
         << "\n" << progname << " [[--config-file|-c] conffile] [[--pidfile|-p] pidfile]"
         << "\n\t   [[--fromtime|-f] 'YYYY-MM-DD hh:mm:ss' or hours back"
         << "\n\n\t [--config-file|-c] configfile Use the configfile. If the name is not"
         << "\n\t       an absolute path the file is looked up relative to " << kvPath("sysconfdir")
         << "\n\t       Default value is set to " << kvPath("sysconfdir") << "/" << progname << ".conf"
         << "\n\t [--pidfile|-p] pidfile Use this as the pid file. "
         << "\n\t       Default value " << kvPath("rundir") << "/" << progname <<"-node.pid"
         << "\n\t       Where node is determined by the hostname."
         << "\n\n";
   exit(  exitCode );
}


string
getProgNameFromArgv0( const std::string &cmd )
{
   fs::path myname( cmd );
   return  myname.filename().string();
}





