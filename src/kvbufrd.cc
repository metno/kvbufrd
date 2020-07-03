/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufrd.cc,v 1.12.2.11 2007/09/27 09:02:23 paule Exp $

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
#include <stdlib.h>
#include <fstream>
#include <thread>
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "milog/milog.h"
#include "fileutil/pidfileutil.h"
#include "miutil/timeconvert.h"
#include "kvalobs/kvPath.h"
#include "BufrWorker.h"
#include "DataReceiver.h"
#include "App.h"
#include "delaycontrol.h"
#include "InitLogger.h"
#include "kvDbGateProxyThread.h"
#include "cachedb.h"
#include "cachedbcleaner.h"
#include "CommandPriorityQueue.h"
#include "CommandPriority2Queue.h"

//using namespace kvservice;
using namespace std;
using namespace miutil;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

void checkPidfile(const Opt &opts);

Opt options;

int
main(int argn, char **argv)
{
   miutil::conf::ConfSection *conf;
   dnmi::file::PidFileHelper pidFile;
// checkAndInitCacheDB("/build/kvbufrd/kvbufrd.sqlite");
// return 0;
   std::string confFile;
   decodeArgs( argn, argv, options );
   InitLogger(argn, argv, options.progname );

   cerr << "conffile: '" << options.conffile << "'" << endl;
   cerr << "pidfile:  '" << options.pidfile << "'" << endl;
   cerr << "progname: '" << options.progname << "'" << endl;

   if( ! fs::exists( fs::path( options.conffile ) ) ) {
      LOGFATAL("The configuration file '" << options.conffile
               << "' does not exist!");
      cerr << "The configuration file '" << options.conffile
           << "' does not exist!";
      exit( 1 );
   }

   try{
      conf=miutil::conf::ConfParser::parse(options.conffile, true);
   }
   catch( const logic_error &ex ){
      LOGFATAL( ex.what() );
      return 1;
   }

   //exit if an instance is allready running.
   checkPidfile(options);
   
   App  app(argn, argv, options, conf );
   std::shared_ptr<threadutil::CommandQueue> newDataQue(new threadutil::CommandQueue());
   std::shared_ptr<threadutil::CommandPriority2Queue> newObsQue(new threadutil::CommandPriority2Queue());

   IDLOGINFO("main", "main Priority loglevel: " << milog::Logger::logger("priqueue").logLevel());

   newDataQue->setName("newDataQue");
   newObsQue->setName("newObsQue");

   DataReceiver dataReceiver(app, newDataQue, newObsQue);
   BufrWorker   bufrWorker(app, newObsQue);
   DelayControl delayControl(app, newDataQue);
   CacheDbCleaner cacheDbCleaner(app);
   
   pidFile.createPidFile(options.pidfile);
   
   pt::ptime startTime;

   //COMMENT:
   //For debugging. At the momment a time spec
   //on the form "YYYY-MM-DD hh:mm:ss" as the first
   //argument on the command line is taken to mean that
   //we shall get data from the server from this data
   //until now.

   if( ! options.fromTime.is_special() ){
      startTime=options.fromTime;
   }else{
      startTime=app.checkpoint();

      if(!startTime.is_special()){
         IDLOGINFO("main", "checkpoint at: " << pt::to_kvalobs_string(startTime));
      }
   }

   std::thread bufrWorkerThread(bufrWorker);
   IDLOGDEBUG("main","Started <BufrWorkerThread>!");

   if(startTime.is_special()){
     startTime=pt::second_clock::universal_time();
     startTime -= pt::hours(6);
   }

   auto notOlderThan=pt::second_clock::universal_time()-pt::hours(24);

   if( startTime < notOlderThan ) {
      auto newStartTime=pt::second_clock::universal_time() - pt::hours(6);
      IDLOGINFO("main", "checkpoint to old: " << pt::to_kvalobs_string(startTime) << " setting it to " <<
         pt::to_kvalobs_string(newStartTime));
      LOGINFO("checkpoint to old: " << pt::to_kvalobs_string(startTime) << " setting it to " <<
         pt::to_kvalobs_string(newStartTime));
      std::cerr << "checkpoint to old: " << pt::to_kvalobs_string(startTime) << " setting it to " <<
         pt::to_kvalobs_string(newStartTime) << endl << endl;
      startTime=newStartTime;
   }

   IDLOGINFO("main","Getting data from kvalobs from time: " << pt::to_kvalobs_string(startTime));
   //app.getDataFrom(startTime, 1044, 0, newObsQue);
   app.getDataFrom(startTime, -1, 0, newObsQue);
   IDLOGDEBUG("main","Return from app.getDataFrom!");


   std::thread dataReceiverThread(dataReceiver);
   IDLOGDEBUG("main","Started <dataReceiverThread>!");

   std::thread delayThread(delayControl);
   IDLOGDEBUG("main","Started <delayControlThread>!");

   std::thread cacheDbCleanerThread(cacheDbCleaner);

   app.run(newDataQue);

   LOGINFO(" --- SHUTDOWN --- ");

   dataReceiverThread.join();
   IDLOGINFO("main","Joined <dataReceiverThread>!");

   bufrWorkerThread.join();
   IDLOGINFO("main","Joined <bufrWorkerThread>!");

   //The dbThread must run until the bufrworker has completed
   app.dbThread->dbQue->suspend();
   app.dbThread->join();

   delayThread.join();
   IDLOGINFO("main","Joined <delayControlThread>!");

   cacheDbCleanerThread.join();
   IDLOGINFO("main","Joined <cacheDbCleanerThread>!");
   //app.doShutdown();

   return 0;
}

void checkPidfile(const Opt &opts) {
   bool error;
   
   if(dnmi::file::isRunningPidFile(opts.pidfile, error)){
      if(error){
         LOGFATAL("An error occured while reading the pidfile:" << endl
                  << opts.pidfile << " remove the file if it exist and"
                  << endl << opts.progname << " is not running. " <<
                  "If it is running and there is problems. Kill " << opts.progname <<
                  " and " << endl << "restart it." << endl << endl);
         exit(1);
      }else{
         LOGFATAL("Is " << opts.progname << " allready running?" << endl
                  << "If not remove the pidfile: " << opts.pidfile);
         exit(1);
      }
   }

}