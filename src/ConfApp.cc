/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: App.h,v 1.13.2.9 2007/09/27 09:02:22 paule Exp $

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

#include <stdlib.h>
#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "ConfApp.h"

using namespace std;

ConfApp::
ConfApp( int argn, char **argv,
         const std::string &confFile_, miutil::conf::ConfSection *conf)
{
   ValElementList valElem;
   string         val;

   createGlobalLogger("kvbufrconf");
   milog::LogManager::setDefaultLogger( "kvbufrconf" );

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

}
   
ConfApp::
~ConfApp()
{
}

void
ConfApp::
createGlobalLogger(const std::string &id)
{
   try{
      milog::FLogStream *logs=new milog::FLogStream(2, 204800); //200k
      std::ostringstream ost;

      ost << kvPath("logdir") << "/kvbufr/" << id << ".log";

      if(logs->open(ost.str())){
         if(!milog::LogManager::createLogger(id, logs)){
            delete logs;
            cerr << "*** Cant create globale logger for log file <" << ost.str() << ">." << endl;
            exit(1);
         }

         return;
      }else{
         LOGERROR("Cant open the logfile <" << ost.str() << ">!");
         cerr << "*** Cant create log file <" << ost.str() << ">." << endl;
         delete logs;
         exit(1);
      }
   }
   catch(...){
      cerr << "Cant create a logstream for LOGID <" << id << ">. Log file <" << ost.str() << ">." << endl;
      exit(1);
   }
}

dnmi::db::Connection*
ConfApp::
getNewDbConnection()
{
  dnmi::db::Connection *con;

  con=dbMgr.connect(dbDriverId, dbConnect);

  if(!con){
    LOGERROR("Can't create a database connection  ("
        << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!");
    return 0;
  }

  LOGINFO("New database connection (" << dbDriverId
       << ") created!");
  return con;
}

void
ConfApp::
releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
}

