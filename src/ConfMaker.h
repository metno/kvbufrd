/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: bufr.h,v 1.8.2.3 2007/09/27 09:02:23 paule Exp $

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

#ifndef __CONFMAKER_H__
#define __CONFMAKER_H__

#include "Indent.h"
#include "StationInfoParse.h"
#include "ConfApp.h"

class ConfMaker
{
   ConfApp &app;
   StInfoSysParamList params;
   std::list<StationInfoPtr> stationList;
   std::list<StationInfoPtr> templateStationList;

   StationInfoPtr findStation( int wmono );
   bool findSensor( const StInfoSysSensorInfoList &sensors, TblStInfoSysSensorInfo &sensor, int paramid )const;
   bool parseTemplate( miutil::conf::ConfSection *templateConf );

   bool decodeProductCoupling( const std::string &val, StationInfoPtr station);
   bool decodeCouplingDelay( const std::string &val, StationInfoPtr station);
   bool decodePrecipPriority( const std::string &val, StationInfoPtr station);
   bool decodeWindheight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station );
   bool decodePrecipHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station );
   bool decodeTemperatureHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station );
   bool decodePressureHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station );

   std::string stationIdToConfString( StationInfoPtr station )const;
   std::string typepriorityToConfString( StationInfoPtr station )const;
   std::string precipPriorityToConfString( StationInfoPtr station )const;

   std::string doStationConf( StationInfoPtr station )const;

public:
   ConfMaker( ConfApp &app );

   bool setParams( const StInfoSysParamList &params );
   bool add( int stationid, TblStInfoSysStation &station, StInfoSysSensorInfoList &sensors );
   bool doConf( const std::string &outfile, miutil::conf::ConfSection *templateConf );
};



#endif