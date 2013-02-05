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
#include <limits.h>
#include "EncodeBufr302040.h"

EncodeBufr302040::
EncodeBufr302040()
{
}

std::string
EncodeBufr302040::
logIdentifier() const
{
   return "302040";
}

std::list<int>
EncodeBufr302040::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302040);

   return ids;
}



void
EncodeBufr302040::
encode( )
{
   /* Precipitation measurement */
   bufr->addValue(  7032, stationInfo->heightPrecip(), "h_Precip, height of precipitation sensor", false);
   bufr->addValue(  4024, data->precipRegional.hTr, "hTr, regional", false);
   bufr->addValue( 13011, data->precipRegional.RR, "RR, regional");
   bufr->addValue(  4024, data->precipNational.hTr, "hTr, national", false );
   bufr->addValue( 13011, data->precipNational.RR, "RR, national");
}

