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
#include <algorithm>
#include <boost/function.hpp>
#include <boost/assign.hpp>
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

   if(!dnmi::db::DriverManager::loadDriver(dbDriver, dbDriverId)){
     LOGFATAL("Can't load driver <" << dbDriver << ">" << endl
         << dnmi::db::DriverManager::getErr() << endl
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

  con=dnmi::db::DriverManager::connect(dbDriverId, dbConnect);

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
  dnmi::db::DriverManager::releaseConnection(con);
}

bool
ConfApp::
loadStationOutmessage( StInfoSysStationOutmessageList &stationOutmessages )
{
   StInfoSysStationOutmessageList dbRes;
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );

   gate.select( dbRes , "WHERE totime>='now' OR totime IS NULL");

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
hasNetworkStation( int stationid, int networkid )
{
	dnmi::db::Connection *con = getDbConnection();
	kvDbGate gate( con );
	ostringstream q;
	StInfoSysNetworkStationList networkStationList;

	q << " WHERE fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)"
	  << "       AND networkid=" << networkid
	  << "       AND stationid=" << stationid;

	if( gate.select( networkStationList, q.str() ) )
		return ! networkStationList.empty();

	throw std::logic_error("Excpetion: hasNetworkStation: DB error: " + gate.getErrorStr() );
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

   q << " WHERE totime IS NULL AND networkid IN (" << *it;

   ++it;

   for( ; it != networkidList.end(); ++it )
      q << ","<< *it;

   q << ") ORDER BY networkid, stationid";

   return gate.select( networkStationList, q.str() );
}



void
ConfApp::
loadNetworkStationByStationid( StInfoSysNetworkStationList &networkStationList,
		                       int stationid,
		                       const std::list<int> &networkidList )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;

   networkStationList.clear();

   if( networkidList.empty() )
      return;

   std::list<int>::const_iterator it = networkidList.begin();

   q << " WHERE fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)"
        " AND networkid IN (" << *it;

   ++it;

   for( ; it != networkidList.end(); ++it )
      q << ","<< *it;

   q << ") AND stationid=" << stationid << " ORDER BY networkid";

   if( ! gate.select( networkStationList, q.str() ) )
	   throw std::logic_error("Exception: loadNetworkStationByStationid: " + gate.getErrorStr() );
}


std::list<int>
ConfApp::
hasParamDefForTypeids( int stationid, const std::list<int> &typeids )
{
	dnmi::db::Connection *con = getDbConnection();

	std::list<int> ret;
	ostringstream q;

	q << "SELECT message_formatid FROM obspgm_h WHERE stationid=" << stationid
	  << " AND message_formatid in (";

	for(std::list<int>::const_iterator it=typeids.begin(); it != typeids.end(); ++it ){
		if( it == typeids.begin() )	q << *it;
		else q << "," << *it;
	}

	q <<") AND (fromtime<='now' AND (totime is NULL OR totime>='now')) group by message_formatid order by message_formatid" ;

	try {
		std::unique_ptr<dnmi::db::Result> res( con->execQuery( q.str() ) );

		while( res->hasNext() ) {
			dnmi::db::DRow &row=res->next();
			ret.push_back( boost::lexical_cast<int>( row[0] ) );
		}
		return ret;
	}
	catch( const dnmi::db::SQLException &ex ) {
		throw std::logic_error( "DB error: " + std::string(ex.what()) );
	}
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
loadStationData(int stationid,  TblStInfoSysStation &station )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;
   StInfoSysStationList stations;

   q << " WHERE stationid=" << stationid << " AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL)";

   if( ! gate.select( stations, q.str() ) )
	   throw std::logic_error("Exception: loadStationData: " + gate.getErrorStr() );

   if( stations.empty() )
      return false;

   if( stations.size() > 1 ) {
      LOGWARN( "More than one record for the station <" << stationid << "> was selected from the 'station' table in stinfosys."
               << endl << " Using the first selected.");
   }

   station = *stations.begin();

   return true;
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




bool
ConfApp::
loadWigosStationData( int stationid,
                 TblStInfoSysWigosStation &station,
                 StInfoSysSensorInfoList &sensors,
                 TblStInfoSysNetworkStation &networkStation )
{
   dnmi::db::Connection *con = getDbConnection();
   kvDbGate gate( con );
   ostringstream q;
   StInfoSysWigosStationList stations;
   StInfoSysNetworkStationList networkStationList;

   q << " WHERE stationid=" << stationid << " AND  (select max(fromtime) from wigos_station where stationid=" << stationid <<") < 'now'";
   //q << " WHERE stationid=" << stationid << " AND ( totime >= 'now' OR totime IS NULL)";

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
   q << " WHERE stationid=" << stationid << " AND networkid IN ( 5 ) AND fromtime<='today' AND ( totime >= 'now' OR totime IS NULL) ORDER BY networkid";
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



bool
ConfApp::
isPlatform( int stationid )
{
	std::list<int> ids;
	StInfoSysNetworkStationList networkStationList;
	TblStInfoSysStation station;
	string callsign = getCallsign( stationid );

	boost::assign::push_back( ids )(11)(22);
	//Used to get the callsign. (Do we need this to verify that the station is a platform?)
	if( callsign.empty() )
		return false;

	if( ! loadStationData( stationid, station) )
		return false;

	//networkid 105 - fast installasjon på kontinentalsokkelen
	if( /*hasNetworkStation( stationid, 105 ) && */
		hasObsPgmHTypeids( stationid, ids, AnyTime_Ignore) &&
  	    (station.maxspeed() == kvDbBase::FLT_NULL || lrintf( station.maxspeed())  == 0 ) )
		return true;
	else
		return false;
}


std::string
ConfApp::
getCallsign( int stationid )
{
	std::list<int> networksids;
	StInfoSysNetworkStationList networkStationList;

	// networkid 6 - Den internasjonale telekomunionens standard kallesignal
	boost::assign::push_back( networksids)(6);
	loadNetworkStationByStationid( networkStationList, stationid, networksids );

	if( networkStationList.empty() )
		return "";

	return networkStationList.begin()->externalStationcode();
}



bool
ConfApp::
isPlatformOrShip( int stationid )
{
	if( ! getCallsign( stationid ).empty() )
		return true;

	return isPlatform( stationid );
}

bool
ConfApp::
findBStations( StInfoSysStationList &stations )
{
	StInfoSysNetworkStationList networkStations;
	StInfoSysStationList myStations;
	std::list<int> ids;
	string query( " WHERE wmono IS NULL AND stationid IN (SELECT stationid FROM obspgm_h WHERE paramid IN (211,81) AND totime IS NULL) AND totime IS NULL AND maxspeed=0");
	stations.clear();

	boost::assign::push_back( ids )(33)(6);
	if( ! loadNetworkStation( networkStations, ids ) ) {
		LOGERROR("Can' load network_stations for networkid=33 (SVV-stations).");
		return false;
	}

	dnmi::db::Connection *con = getDbConnection();

	if( !con ) {
		LOGERROR("Can connect to stinfosys.");
		return false;
	}
	kvDbGate gate( con );

	gate.select( myStations, query );

	if( myStations.empty() ) {
		LOGWARN("No BSTATIONS found!");
		return true;
	}

	//exclude SVV (statens veivesen) stations and stations with callsign (SHIP).
	for( StInfoSysStationList::const_iterator it = myStations.begin(); it != myStations.end(); ++it) {
		StInfoSysNetworkStationList::const_iterator nit;
		for( nit = networkStations.begin(); nit != networkStations.end() && nit->stationid() != it->stationid(); ++nit);

		if( nit == networkStations.end() )
			stations.push_back( *it );
	}

	if( stations.empty() ) {
		LOGWARN("No BSTATIONS found!");
		return true;
	} else 	{
		LOGINFO("Found " << stations.size() << " BSTATIONS.");
	}

	return true;
}

namespace {

bool
hasEqualTypeid( const TblStInfoSysObsPgmH &o1, const TblStInfoSysObsPgmH &o2 )
{
	return o1.messageFormatid() == o2.messageFormatid();
}
}

bool
ConfApp::
findObsPgmHTypeids( StInfoSysObsObsPgmHList &obspgm, int stationid, const std::list<int> &paramids )
{
	ostringstream q;
	obspgm.clear();
	StInfoSysObsObsPgmHList tmp;
	dnmi::db::Connection *con = getDbConnection();

	if( !con ) {
		LOGERROR("Can connect to stinfosys.");
		return false;
	}

	kvDbGate gate( con );

	q << " WHERE stationid=" << stationid;

	if( !paramids.empty() ) {
		q << " AND paramid IN (";
		for( list<int>::const_iterator it=paramids.begin(); it != paramids.end(); ++it ) {
			if( it != paramids.begin() )
				q << ",";
			q << *it;
		}
		q << ")";
	}

	q << " AND fromtime <= 'today' AND totime IS NULL AND priority_message";

	gate.select( tmp, q.str() );


	std::unique_copy( tmp.begin(), tmp.end(), back_inserter( obspgm ), hasEqualTypeid );

	return true;
}

bool
ConfApp::
hasObsPgmHParamsids( StInfoSysObsObsPgmHList &obspgm, int stationid, int typeid_,const std::list<int> &paramids, ObsPgmAnyTime anytime )
{
	ostringstream q;
	obspgm.clear();
	dnmi::db::Connection *con = getDbConnection();

	if( !con ) {
		LOGERROR("Can connect to stinfosys.");
		return false;
	}

	kvDbGate gate( con );

	q << " WHERE stationid=" << stationid << " AND message_formatid=" << typeid_;

	if( !paramids.empty() ) {
		q << " AND paramid IN (";
		for( list<int>::const_iterator it=paramids.begin(); it != paramids.end(); ++it ) {
			if( it != paramids.begin() )
				q << ",";
			q << *it;
		}
		q << ")";
	}

	q << " AND fromtime <= 'today' AND totime IS NULL AND priority_message";

	if( anytime != AnyTime_Ignore )
		q << " AND anytime=" << (anytime==AnyTime_True?"true":"false");

	q << " ORDER BY paramid";

	if( ! gate.select( obspgm, q.str() ) ) {
		LOGERROR("hasObsPgmHParamsids: invalid query: '" << q.str() << "'"<<endl
				<< "Reasom: " << gate.getErrorStr() );
		return false;
	}


	return true;
}

bool
ConfApp::
hasObsPgmHTypeids( int stationid, const std::list<int> &typeids, ObsPgmAnyTime anytime )
{
	ostringstream q;
	dnmi::db::Connection *con = getDbConnection();

	if( !con ) {
		LOGERROR("Can connect to stinfosys.");
		throw std::logic_error("Exception: hasObsPgmHTypeids: DB Error: Cant connect to stinfosys.");
	}

	q << "SELECT stationid, message_formatid FROM obspgm_h WHERE stationid=" << stationid;

	if( !typeids.empty() ) {
		q << " AND message_formatid IN (";
		for( list<int>::const_iterator it=typeids.begin(); it != typeids.end(); ++it ) {
			if( it != typeids.begin() )
				q << ",";
			q << *it;
		}
		q << ")";
	}

	q << " AND fromtime <= 'today' AND totime IS NULL AND priority_message";

	if( anytime != AnyTime_Ignore )
		q << " AND anytime=" << (anytime==AnyTime_True?"true":"false");

	q << " GROUP BY stationid, message_formatid";


	try {
		std::unique_ptr<dnmi::db::Result> res( con->execQuery( q.str() ) );

		return  res->size() > 0;
	}
	catch( const dnmi::db::SQLException &ex ) {
		throw std::logic_error( "Exception: hasObsPgmHTypeid: DB error: " + std::string(ex.what()) );
	}
	throw std::logic_error( "Exception: hasObsPgmHTypeid: DB error: Unknown error." );
}


