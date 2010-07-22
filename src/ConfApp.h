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
#include "tblStInfoSysParam.h"
#include "tblStInfoSysStationOutmessage.h"
#include "StationInfo.h"

typedef std::list<TblStInfoSysParam> StInfoSysParamList;
typedef std::list<TblStInfoSysStationOutmessage> StInfoSysStationOutmessageList;

class ConfApp
{
   dnmi::db::DriverManager dbMgr;
   std::string             dbDriver;
   std::string             dbConnect;
   std::string             dbDriverId;
   StationList             stationList;
   StInfoSysParamList      stInfoSysParamList;

public:
   ConfApp( int argn, char **argv,
            const std::string &confFile_, miutil::conf::ConfSection *conf);
   ~ConfApp();

   void createGlobalLogger(const std::string &id);
   /**
    * \brief Creates a new connection to the database.
    *
    * The caller must call releaseDbConnection after use.
    *
    * \return A database connection.
    */
   dnmi::db::Connection *getNewDbConnection();

   /**
    * \brief release a connection to the database.
    *
    * The connction to be released must be obtained by a call to
    * getNewDbConnection().
    *
    * \param con The connection to release.
    */
   void                 releaseDbConnection(dnmi::db::Connection *con);


   void loadDataFromStInfoSys();

};

#endif
