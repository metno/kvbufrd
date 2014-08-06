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

#ifndef __ConfApp_h__
#define __ConfApp_h__

#include <list>
#include <puTools/miTime.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include "tblStInfoSysParam.h"
#include "tblStInfoSysNetworkStation.h"
#include "tblStInfoSysStationOutmessage.h"
#include "tblStInfoSysStation.h"
#include "tblStInfoSysSensorInfo.h"
#include "tblStInfoSysObsPgmH.h"
#include "StationInfo.h"
#include "kvbufrconfOptions.h"

typedef std::list<TblStInfoSysParam> StInfoSysParamList;
typedef std::list<TblStInfoSysStationOutmessage> StInfoSysStationOutmessageList;
typedef std::list<TblStInfoSysStation> StInfoSysStationList;
typedef std::list<TblStInfoSysSensorInfo> StInfoSysSensorInfoList;
typedef std::list<TblStInfoSysNetworkStation> StInfoSysNetworkStationList;
typedef std::list<TblStInfoSysObsPgmH> StInfoSysObsObsPgmHList;

class ConfApp
{
   dnmi::db::DriverManager dbMgr;
   std::string             dbDriver;
   std::string             dbConnect;
   std::string             dbDriverId;
   StationList             stationList;
   StInfoSysParamList      stInfoSysParamList;
   dnmi::db::Connection    *connection;

   dnmi::db::Connection* getDbConnection();

public:
   typedef enum {AnyTime_True, AnyTime_False, AnyTime_Ignore} ObsPgmAnyTime;
   ConfApp( Options &options, miutil::conf::ConfSection *conf);
   ~ConfApp();

   void createGlobalLogger(const std::string &id);
   /**
    * \brief Creates a new connection to the database.
    *
    * The caller must call releaseDbConnection after use.
    *
    * \return A database connection.
    */
   dnmi::db::Connection* getNewDbConnection();

   /**
    * \brief release a connection to the database.
    *
    * The connction to be released must be obtained by a call to
    * getNewDbConnection().
    *
    * \param con The connection to release.
    */
   void                 releaseDbConnection(dnmi::db::Connection *con);

   bool loadStationOutmessage( StInfoSysStationOutmessageList &stationOutmessages );
   bool loadParams( StInfoSysParamList &params );
   bool hasNetworkStation( int stationid, int networkid );

   /**
    * Load data from stinfosys network_station table by
    * stationid and networkid.
    *
    * Returns all stations found in networkStationList.
    *
    * @param[out] networkStationList The stations found.
    * @param stationid The stationid to search for.
    * @param networkidList a list of networkids to search for.
    * @throws std::logical_error on db error.
    */
   void loadNetworkStationByStationid( StInfoSysNetworkStationList &networkStationList,
                                       int stationid, const std::list<int> &networkidList );

   bool loadNetworkStation( StInfoSysNetworkStationList &networkStationList,
                            const std::list<int> &networkidList );
   std::list<int> hasParamDefForTypeids( int stationid, const std::list<int> &typeids );
   bool loadObsPgmH( StInfoSysObsObsPgmHList &obsPgmHList,
                     const std::list<int> &typeidList );

   /**
    * loadStationData loads data from the \em station table in stinfosys by stationid.
    *
    * @param[out] station The station data.
    * @return true if the stationid has station data, false otherwise.
    * @throws std::logic_error on db error.
    */
   bool loadStationData(int stationid,  TblStInfoSysStation &station );
   bool loadStationData(int stationid,  TblStInfoSysStation &station, StInfoSysSensorInfoList &sensors );
   bool loadStationData(int stationid,  TblStInfoSysStation &station, StInfoSysSensorInfoList &sensors, TblStInfoSysNetworkStation &networkStation );
   std::string getCallsign( int stationid );
   bool isPlatform( int stationid );
   bool isPlatformOrShip( int stationid );
   bool findBStations( StInfoSysStationList &stations );
   bool findObsPgmHTypeids( StInfoSysObsObsPgmHList &obspgm, int stationid, const std::list<int> &paramids );
   bool hasObsPgmHParamsids( StInfoSysObsObsPgmHList &obspgm, int stationid, int typeid_,const std::list<int> &paramids, ObsPgmAnyTime anytime=AnyTime_Ignore  );

   /**
    * @throws std::logic_error on DB problems.
    */
   bool hasObsPgmHTypeids( int stationid, const std::list<int> &typeids, ObsPgmAnyTime anytime=AnyTime_Ignore );


};

#endif
