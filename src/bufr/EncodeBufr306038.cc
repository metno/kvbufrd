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
#include <tuple>
#include "EncodeBufr306038.h"

using std::get;

EncodeBufr306038::
EncodeBufr306038()
{

}
std::string
EncodeBufr306038::
logIdentifier() const
{
   return "306038";
}

std::list<int>
EncodeBufr306038::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306038);

   return ids;
}



void
EncodeBufr306038::
encode( )
{

  //Pressure data
  bufr->addValue( 10004, get<0>(data->PO.getFirstValue()), "P0" );
  bufr->addValue( 10051, get<0>(data->PR.getFirstValue()), "PP" );
  
   
  //temperature 
  float ta=FLT_MAX;
  float td=FLT_MAX;
  float uu=FLT_MAX;
  
  //The configured heightTemperature and heightWind is probably only valid 
  //for sensor and level equal to 0.

  //TA, TD and UU must be from the same levels. It is assumed that 
  //TA allways is given. It makes no sense to report UU and TD without TA.
  int taHeight=INT_MAX;
  auto TA = data->TA.getFirstValue();
  ta = get<0>(TA);

  if( ta != FLT_MAX ) {
    int level=get<2>(TA);
    int sensor=get<1>(TA);
    td = data->TD.getFirstValueAtLevel(level);
    uu = data->UU.getFirstValueAtLevel(level);
    taHeight=stationInfo->heightTemperature(); 
    if( level != 0 || sensor !=0 ) {
      taHeight=level;
    } 
  }  

  bufr->addValue( 7033, taHeight, "Height of sensor above water surface" );
  bufr->addValue( 12101, ta, "TA" );
  bufr->addValue( 12103, td, "TD, dew-point temperature" );
  bufr->addValue( 13003, uu, "UU, relativ humidity" );

  //wind
  int   windHeight=INT_MAX;
  float fg_010=FLT_MAX;
  bool  fg_010_valid=false;
  float dd=FLT_MAX;
  auto FF=data->FF.getFirstValue();
  float ff=get<0>(FF);

  if( ff != FLT_MAX ) {
    int sensor=get<1>(FF);
    int level = get<2>(FF);
    dd=data->DD.getFirstValueAtLevel(level);
    fg_010=data->FG_010.getFirstValueAtLevel(level);
    fg_010_valid = fg_010 != FLT_MAX;
    windHeight=stationInfo->heightWind();

    if(sensor!=0 || level!=0) {
      windHeight=level;
    }
  }

  bufr->addValue(  7033, windHeight, "Height of sensor above water surface", false);
  bufr->addValue(  8021, 2, "Time significance (=2: time averaged)", false);
  bufr->addValue(  4025, static_cast<float>(-10), "Time period or displacement (minutes)", false);
  bufr->addValue( 11001, dd, "DD, wind direction");
  bufr->addValue( 11002, ff, "FF, wind speed");
  bufr->addValue(  8021, INT_MAX, "Time significance", false);

  //Gust
  bufr->addValue(  4025, fg_010_valid?static_cast<float>(-10):FLT_MAX, "Time period or displacement (minutes)", false, "GUST");
  bufr->addValue( 11041, fg_010, "FG_010, wind speed (gust)"), true, "GUST";

  //Sea temperature
  bufr->addValue(  4025, FLT_MAX, "Time period or displacement (minutes)", false);
  bufr->addValue( 7033, FLT_MAX, "Height of sensor above water surface.", false);
  bufr->addValue(2005,FLT_MAX, "Precision of temperature observation (K)", false);
  bufr->addValue( 7063, 0.5f, "Sea/water depth of measurement (cm)", false );
  bufr->addValue( 22049, data->TW, "Sea/water temperature" );
}

