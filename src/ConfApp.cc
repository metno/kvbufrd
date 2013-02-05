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
using namespace kvalobs;
using namespace miutil::conf;

ConfApp::
ConfApp( Options &options, miutil::conf::ConfSection *conf)
   : connection( 0 )
{
   ValElementList valElem;
   string         val;

   valElem=conf->getValue("database.dbdriver");

   if( valElem.empty() ) {
     LOGFATAL("No <database.driver> in the configurationfile!");
     exit(1);
   }

   dbDriver=valElem[0].valAsString();

   if( ! dbDriver.empty() &&  dbDriver[0]!='/' && dbDriver[0]!='.' ) //This count also for ..
      dbDriver = kvPath("pkglibdir") + "/db/" + dbDriver;

   LOGINFO("Loading driver for database engine <" << dbDriver << ">!\n");

   if(!dbMgr.loadDriver(dbDriver, dbDriverId)){
     LOGFATAL("Can't load driver <" << dbDriver << ">" << endl
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
   ostringstream ost;

   ost << kvPath("logdir") << "/" << id << ".log";

   try{
      milog::FLogStream *logs=new milog::FLogStream(2, 204800); //200k

      if(logs->open(ost.str())){
         if(!milog::LogManager::createLogger(id, logs)){
            delete logs;
            cerr << "*** Cant create globale logger for log file <" << ost.str() << ">." << endl;
            exit(1);
         }

         return;
      }else{
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
getDbConnection()
{
   if( connection )
      return connection;

   connection = getNewDbConnection();

   if( ! connection ) {
      LOGFATAL( "Cant create a connection to the database." );
      exit( 1 );
   }

   return connection;
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

bool
ConfApp::
loadStationOutmessage( StInfoSysStationOutmessageList &stationOutmessages )
{
   StInfoSysStationOutmessageList dbRes;
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );

   gate.select( dbRes , "WHERE fromtime<='now' AND (totime>='now' OR totime IS NULL)");

   if( gate.getError() != kvDbGate::NoError ) {
      LOGERROR( "DB: Failed to load StationOutmessage. '" << gate.getErrorStr() << "'.");
      return false;
   }

   stationOutmessages.clear();

   for( StInfoSysStationOutmessageList::iterator it=dbRes.begin(); it!=dbRes.end(); ++it ) {
      StInfoSysStationOutmessageList::iterator itFind=stationOutmessages.begin();
      for(; itFind != stationOutmessages.end(); ++itFind ) {
         if( itFind->stationid() == it->stationid() ) {
            if( it->fromTime() > itFind->fromTime() )
               *itFind = *it;

            break;
         }
      }

      if( itFind == stationOutmessages.end() )
         stationOutmessages.push_back( *it );
   }

   return true;
}

bool
ConfApp::
loadParams( StInfoSysParamList &params )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );

   gate.select( params );

   if( gate.getError() != kvDbGate::NoError ) {
      LOGERROR( "DB: Failed to load Param. '" << gate.getErrorStr() << "'.");
      return false;
   }

   return true;
}

bool
ConfApp::
loadNetworkStation( StInfoSysNetworkStationList &networkStationList,
                    const std::list<int> &networkidList )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;

   networkStationList.clear();

   if( networkidList.empty() ) {
      return true;
   }

   std::list<int>::const_iterator it = networkidList.begin();

   q << " WHERE fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)"
        " AND networkid IN (" << *it;

   ++it;

   for( ; it != networkidList.end(); ++it )
      q << ","<< *it;

   q << ") ORDER BY networkid, stationid";

   return gate.select( networkStationList, q.str() );
}

bool
ConfApp::
loadObsPgmH( StInfoSysObsObsPgmHList &obsPgmHList,
             const std::list<int> &typeidList )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;

   obsPgmHList.clear();

   if( typeidList.empty() ) {
      return true;
   }

   std::list<int>::const_iterator it = typeidList.begin();

   q << " WHERE fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)"
        " AND message_formatid IN (" << *it;

   ++it;

   for( ; it != typeidList.end(); ++it )
      q << ","<< *it;

   q << ") ORDER BY message_formatid, stationid, paramid";

   return gate.select( obsPgmHList, q.str() );
}

bool
ConfApp::
loadStationData(int stationid,  TblStInfoSysStation &station, StInfoSysSensorInfoList &sensors )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;
   StInfoSysStationList stations;

   q << " WHERE stationid=" << stationid << " AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)";

   gate.select( stations, q.str() );

   if( stations.empty() )
      return false;

   if( stations.size() > 1 ) {
      LOGWARN( "More than one record for the station <" << stationid << "> was selected from the 'station' table in stinfosys."
               << endl << " Using the first selected.");
   }

   station = *stations.begin();

   q.str("");
   q << " WHERE stationid=" << stationid << " AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL) AND operational=true";

   gate.select( sensors, q.str() );

   return true;
}

bool
ConfApp::
loadStationData( int stationid,
                 TblStInfoSysStation &station,
                 StInfoSysSensorInfoList &sensors,
                 TblStInfoSysNetworkStation &networkStation )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;
   StInfoSysStationList stations;
   StInfoSysNetworkStationList networkStationList;

   q << " WHERE stationid=" << stationid << " AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)";

   gate.select( stations, q.str() );

   if( stations.empty() )
      return false;

   if( stations.size() > 1 ) {
      LOGWARN( "More than one record for the station <" << stationid << "> was selected from the 'station' table in stinfosys."
               << endl << " Using the first selected.");
   }

   station = *stations.begin();

   q.str("");
   q << " WHERE stationid=" << stationid << " AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL) AND operational=true";

   gate.select( sensors, q.str() );


   //Lookup in the network_station to find the wmo name. Synopdata has networkid=4
   //Search the table obs_network to find the correct networkid. it can not change over time so it is hardcoded to 4 here.
   q.str("");
   q << " WHERE stationid=" << stationid << " AND networkid IN ( 4, 44 ) AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL) ORDER BY networkid";
   gate.select( networkStationList, q.str() );

   if( ! networkStationList.empty() ) {
      networkStation = *networkStationList.begin();
      if( networkStationList.size() > 1 ) {
         LOGWARN( "More than one record for the station <" << stationid << "> was selected from the 'network_station' table in stinfosys."
                  << endl << " Using the first selected station (networkid=" << networkStation.networkid() << ").");
      }

   } else {
      LOGWARN("No data in the table 'network_station' for stationid " << stationid << ". ie we are missing any name for the station.");
      networkStation.clean();
   }

   return true;
}
