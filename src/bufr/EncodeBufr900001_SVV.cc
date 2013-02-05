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
#include "EncodeBufr900001_SVV.h"

namespace b=boost;


EncodeBufr900001::
EncodeBufr900001()
{
}
EncodeBufr900001::
~EncodeBufr900001()
{
}


std::string
EncodeBufr900001::
logIdentifier() const
{
   return "900001";
}

std::list<int>
EncodeBufr900001::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(900001);

   return ids;
}

void
EncodeBufr900001::
encode()
{
   BufrTemplateList templateList;

   boost::assign::push_back( templateList )
      (301089)(1019)(2001)(301011)(301012)(301021)
      (7030)(25053)(302032)(302034)(7032)(13013)(302040)
      (302042);

   bufr->addDescriptorList( templateList );

   bufr->setObsTime( obstime );

   encodeTemplate( 301089 );

   bufr->addValue(1019, stationInfo->name(), "Station name", false );
   bufr->addValue( 2001, 0, "Type of station. Code table 2001", false );

   encodeTemplate( 301011 ); //Date.
   encodeTemplate( 301012 ); //Time in day.
   encodeTemplate( 301021 ); //Position lat/long.

   bufr->addValue( 7030, stationInfo->height(), "Height of station", false );

   //At the moment the quality flag is hard coded to 3 (questionable).
   //Later we may use the quality flag somehow to to set this value.
   //This is a flag value where individual bit is set.
   //Table of values in hex.
   // 0x800 Good
   // 0x400 Redundant
   // 0x200 Questionable
   // 0x100 Bad
   // 0x080 Experimental
   // 0x040 Precipitating
   // 0xFFF Missing
   bufr->addValue(25053, 0x200,  "OBSERVATION QUALITY", false );
   encodeTemplate( 302032 );
   encodeTemplate( 302034 );

   bufr->addValue( 7032, INT_MAX );

   bufr->addValue( 13013, data->SA, "SA, total snow depth" );

   encodeTemplate( 302040 );
   encodeTemplate( 302042 );

   bufr->setDataAndInternationalSubDataCategory( 0, 0 );
   bufr->setLocalDataSubCategory( 20 );
   bufr->setGTSHeader("IS", "XD" );
}

