/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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

#include <boost/assign.hpp>
#include "EncodeBufr306004.h"

EncodeBufr306004::
EncodeBufr306004()
{

}
std::string
EncodeBufr306004::
logIdentifier() const
{
   return "306004";
}

std::list<int>
EncodeBufr306004::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306004);

   return ids;
}



void
EncodeBufr306004::
encode( )
{
  //Start her

  bufr->addDelayedReplicationFactor(31000, 0);


  // //Pressure data
  // bufr->addValue( 10004, data->PO, "P0" );
  // bufr->addValue( 10051, data->PR, "PP" );
  
   
  // //temperature 
  // bufr->addValue( 7033, stationInfo->heightTemperature(), "Height of sensor above water surface" );
  // bufr->addValue( 12101, data->TA, "TA" );
  // bufr->addValue( 12103, data->TD, "TD, dew-point temperature" );
  // bufr->addValue( 13003, data->UU, "UU, relativ humidity" );

  // //wind
  //  bufr->addValue(  7033, stationInfo->heightWind(), "Height of sensor above water surface", false);
  //  bufr->addValue(  8021, 2, "Time significance (=2: time averaged)", false);
  //  bufr->addValue(  4025, static_cast<float>(-10), "Time period or displacement (minutes)", false);
  //  bufr->addValue( 11001, data->DD, "DD, wind direction");
  //  bufr->addValue( 11002, data->FF, "FF, wind speed");
  //  bufr->addValue(  8021, INT_MAX, "Time significance", false);

  // //Gust
  // bufr->addValue(  4025, (data->FG_010.valid()?static_cast<float>(-10):FLT_MAX), "Time period or displacement (minutes)", false, "GUST");
  // bufr->addValue( 11041, data->FG_010, "FG_010, wind speed (gust)"), true, "GUST";

  // //Sea temperature
  // bufr->addValue(  4025, FLT_MAX, "Time period or displacement (minutes)", false);
  // bufr->addValue( 7033, FLT_MAX, "Height of sensor above water surface.", false);
  // bufr->addValue(2005,FLT_MAX, "Precision of temperature observation (K)", false);
  // bufr->addValue( 7063, 0.5f, "Sea/water depth of measurement (cm)", false );
  // bufr->addValue( 22049, data->TW, "Sea/water temperature" );
  
}

