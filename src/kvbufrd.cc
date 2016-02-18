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
#include <fstream>
#include "boost/thread/thread.hpp"
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "milog/milog.h"
#include "fileutil/pidfileutil.h"
#include "miutil/timeconvert.h"
#include "kvalobs/kvapp.h"
#include "kvalobs/kvPath.h"
#include "BufrWorker.h"
#include "DataReceiver.h"
#include "App.h"
#include "delaycontrol.h"
#include "InitLogger.h"
#include "kvDbGateProxyThread.h"

//using namespace kvservice;
using namespace std;
using namespace miutil;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

Opt options;

int
main(int argn, char **argv)
{
   bool error;
   std::string pidfile;
   std::string confFile;

   decodeArgs( argn, argv, options );
   InitLogger(argn, argv, options.progname );


   confFile = options.conffile;
   pidfile = options.pidfile;

   dnmi::file::PidFileHelper pidFile;
   miutil::conf::ConfSection *conf;

   cerr << "conffile: '" << confFile << "'" << endl;
   cerr << "pidfile:  '" << pidfile << "'" << endl;
   cerr << "progname: '" << options.progname << "'" << endl;

   if( ! fs::exists( fs::path( confFile ) ) ) {
      LOGFATAL("The configuration file '" << confFile
               << "' does not exist!");
      cerr << "The configuration file '" << pidfile
           << "' does not exist!";
      exit( 1 );
   }

   try{
      conf=miutil::conf::ConfParser::parse(confFile, true);
   }
   catch( const logic_error &ex ){
      LOGFATAL( ex.what() );
      return 1;
   }

   App  app(argn, argv, confFile, conf );
   std::shared_ptr<dnmi::thread::CommandQue> newDataQue(new dnmi::thread::CommandQue());
   std::shared_ptr<dnmi::thread::CommandQue> newObsQue(new dnmi::thread::CommandQue());

   DataReceiver dataReceiver(app, newDataQue, newObsQue);
   BufrWorker   bufrWorker(app, newObsQue);
   DelayControl delayControl(app, newDataQue);
   pt::ptime startTime;


   if(dnmi::file::isRunningPidFile(pidfile, error)){
      if(error){
         LOGFATAL("An error occured while reading the pidfile:" << endl
                  << pidfile << " remove the file if it exist and"
                  << endl << options.progname << " is not running. " <<
                  "If it is running and there is problems. Kill " << options.progname <<
                  " and " << endl << "restart it." << endl << endl);
         return 1;
      }else{
         LOGFATAL("Is " << options.progname << " allready running?" << endl
                  << "If not remove the pidfile: " << pidfile);
         return 1;
      }
   }


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

   pidFile.createPidFile(pidfile);

   boost::thread bufrWorkerThread(bufrWorker);
   IDLOGDEBUG("main","Started <BufrWorkerThread>!");

   if(startTime.is_special()){
     startTime=pt::second_clock::universal_time();
     startTime -= pt::hours(48);
   }

   IDLOGINFO("main","Getting data from kvalobs from time: " << pt::to_kvalobs_string(startTime));
   //app.getDataFrom(startTime, 1044, 0, newObsQue);
   app.getDataFrom(startTime, -1, 0, newObsQue);
   IDLOGDEBUG("main","Return from app.getDataFrom!");


   boost::thread dataReceiverThread(dataReceiver);
   IDLOGDEBUG("main","Started <dataReceiverThread>!");

   boost::thread delayThread(delayControl);
   IDLOGDEBUG("main","Started <delayControlThread>!");

   app.run(newDataQue);

   app.dbThread->dbQue->suspend();
   app.dbThread->join();

   dataReceiverThread.join();
   IDLOGDEBUG("main","Joined <dataReceiverThread>!");

   bufrWorkerThread.join();
   IDLOGDEBUG("main","Joined <bufrWorkerThread>!");

   delayThread.join();
   IDLOGDEBUG("main","Joined <delayControlThread>!");

   //app.doShutdown();

   return 0;
}

