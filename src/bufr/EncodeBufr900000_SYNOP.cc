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
#include "EncodeBufr900000_SYNOP.h"

namespace b=boost;
using namespace std;


EncodeBufr900000::
EncodeBufr900000()
{
}
EncodeBufr900000::
~EncodeBufr900000()
{
}


std::string
EncodeBufr900000::
logIdentifier() const
{
   return "900000";
}

std::list<int>
EncodeBufr900000::
encodeIds()const
{
   list<int> ids;
   b::assign::push_back( ids )(900000);

   return ids;
}

void
EncodeBufr900000::
encode()
{
   BufrTemplateList templateList;


   boost::assign::push_back( templateList )
      (307079)(4025)(11042);

   bufr->addDescriptorList( templateList );

   bufr->setObsTime( obstime );

   encodeTemplate( 307079 );

   //Data elements only used by met.no.
   bufr->addValue(  4025, data->FxMax.t,  "Time when max wind occured." );
   bufr->addValue( 11042, data->FxMax.ff, "Max wind." );

   string A1;
   int internationalSubCategory;

   switch( obstime.time_of_day().hours() ){
      case 0:
      case 6:
      case 12:
      case 18:
         internationalSubCategory = 2;
         A1="M";
         break;
      case 3:
      case 9:
      case 15:
      case 21:
         internationalSubCategory = 1;
         A1 ="I";
         break;
      default:
         internationalSubCategory = 0;
         A1 = "N";
         break;
   }

   string A2 = computeAreaDesignator( stationInfo->longitude(), stationInfo->latitude());

   bufr->setDataAndInternationalSubDataCategory( 0, internationalSubCategory );
   bufr->setGTSHeader( "IS", A1 + A2 );
}

