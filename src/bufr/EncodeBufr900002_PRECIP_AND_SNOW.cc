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
#include <iostream>
#include <boost/assign.hpp>
#include <boost/assign.hpp>
#include "EncodeBufr900002_PRECIP_AND_SNOW.h"

using namespace std;
namespace b=boost;

#define FEQ(f1, f2, d) ((fabsf((f1)-(f2)))<(d)?true:false)

EncodeBufr900002::
EncodeBufr900002()
{
}
EncodeBufr900002::
~EncodeBufr900002()
{
}


std::string
EncodeBufr900002::
logIdentifier() const
{
   return "900002";
}

std::list<int>
EncodeBufr900002::
encodeIds()const
{
   std::list<int> ids;
   b::assign::push_back( ids )(900002);

   return ids;
}

void
EncodeBufr900002::
encode()
{
   BufrTemplateList templateList;


   b::assign::push_back( templateList )
      (1101)(1102)(1019)(2001)(4001)(4002)
      (4003)(4004)(4005)(5001)(6001)(7030)
      (7032)(12101)(7032)(2177)(20062)(13013)
      (13023)(13012);

   bufr->addDescriptorList( templateList );

   bufr->setObsTime( obstime );


   //001101, 001102
   encodeTemplate( 301089 );

   bufr->addValue( 1019, stationInfo->name(), "Station name.", false );
   bufr->addValue( 2001, data->IX.valAsInt(), "Type of station. Code table 0.", false );

   // 004001, 004002, 004003
   encodeTemplate( 301011 );

   //004004, 004005
   encodeTemplate( 301012 );

   // 005001, 006001
   encodeTemplate( 301021 );
   bufr->addValue( 7030, static_cast<float>( stationInfo->height() ), "Station height", false );

   bufr->addValue( 7032,  stationInfo->heightTemperature(), "Height of temperatue sensor", false );

   bufr->addValue( 12101, data->TA, "TA" );

   //  State of ground, snow depth
   bufr->addValue( 7032,  FLT_MAX, "Height of snow depth sensor", false );
   bufr->addValue( 2177,  0, "Method of snow depth measurement. Code table 2177.", false );

   bufr->addValue( 20062, data->EE.valAsInt(), "EE, state of ground" );
   bufr->addValue( 13013, data->SA, "SA, total snow depth" );

//   013023  TOTAL PRECIPITATION PAST 24 HOURS [KG/M**2] 1 -1 14
//   013012  DEPTH OF FRESH SNOW [M] 2 -2 12
   float rr=FLT_MAX;

//   cerr << "PRECIP: RR_24="<<data->RR_24.value() << endl;

   if( data->RR_24.valid() )
       rr = encodeRR( data->RR_24.value() );

   bufr->addValue( 13023, rr, "RR24, precipitation past 24 hours" );
   bufr->addValue( 13012, data->SS_24, "Depth of fresh snow." );

   bufr->setDataAndInternationalSubDataCategory( 0, 0 );
   bufr->setLocalDataSubCategory( 10 );
   bufr->setGTSHeader("IS", "XD" );
}

