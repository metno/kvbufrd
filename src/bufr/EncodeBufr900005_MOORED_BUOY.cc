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
#include <boost/assign.hpp>
#include "AreaDesignator.h"
#include "EncodeBufr900005_MOORED_BUOY.h"


// Ankrede bøyer (MOORED BUOYS)
// stationid | lat   | lon   | height | maxspeed | name              | call_sign
// ----------+-------+-------+--------+----------+-------------------+-----------
// 20925     | 73.5  | 24.13 |   0    | 0        | WISTING           | 6301003
// 20926     | 72.49 | 20.15 |   0    | 5        | HJELMSØYBANKEN    | 6301004
// 20927     | 72.66 | 34.7  |   0    | 0        | NORDKAPPBASSENGET | 6301005
// 76900     | 66    |  2    |   2    | 0        | MIKE              | 6301000
// 76907     | 62.8  |  4.3  |   0    | 0        | STOREGGA          | 6301002
// 76933     | 63.49 |  5.015 |       | 0        | ORMEN LANGE       | 6301001

namespace b=boost;
using namespace std;


EncodeBufr900005::
EncodeBufr900005()
{
}
EncodeBufr900005::
~EncodeBufr900005()
{
}


std::string
EncodeBufr900005::
logIdentifier() const
{
   return "900005";
}

std::list<int>
EncodeBufr900005::
encodeIds()const
{
   list<int> ids;
   b::assign::push_back( ids )(900005);

   return ids;
}

void
EncodeBufr900005::
encode()
{
   BufrTemplateList templateList;


   boost::assign::push_back( templateList )
      (315008);

   bufr->addDescriptorList( templateList );

   bufr->setObsTime( obstime );

   encodeTemplate( 315008 );

string A1("S");
//   int internationalSubCategory;
//
//   switch( obstime.hour() ){
//      case 0:
//      case 6:
//      case 12:
//      case 18:
//         internationalSubCategory = 2;
//         A1="M";
//         break;
//      case 3:
//      case 9:
//      case 15:
//      case 21:
//         internationalSubCategory = 1;
//         A1 ="I";
//         break;
//      default:
//         internationalSubCategory = 0;
//         A1 = "N";
//         break;
//   }

   float latitude;
   float longitude;
   bool  hasValidPos=true;

   if( data->MLAT.valid() && data->MLON.valid() ) {
	   latitude = data->MLAT;
	   longitude = data->MLON;
   } else if( stationInfo->latitude() != FLT_MAX && stationInfo->longitude() != FLT_MAX ){
	   latitude = stationInfo->latitude();
	   longitude = stationInfo->longitude();
   } else {
       hasValidPos = false;
   }

   if( ! hasValidPos ) {
       bufr->validBufr( false );
       bufr->addErrorMessage( "Missing location data (latitude/longitude) for the station/ship.");
   }

   string A2=computeAreaDesignator( longitude, latitude );
   bufr->setDataAndInternationalSubDataCategory( 1, 0/*internationalSubCategory*/ );
   bufr->setGTSHeader( "IS", A1+A2 );
}

