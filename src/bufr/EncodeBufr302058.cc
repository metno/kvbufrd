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
#include "EncodeBufr302058.h"

EncodeBufr302058::
EncodeBufr302058()
{
}

std::string
EncodeBufr302058::
logIdentifier() const
{
   return "302058";
}

std::list<int>
EncodeBufr302058::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302058);

   return ids;
}



void
EncodeBufr302058::
encode( )
{
    /* Extreme temperature data */
    bufr->addValue(  7032, stationInfo->heightTemperature(), "h_Temp, height of temperature sensor above marine deck.", false);
    bufr->addValue(  7033, FLT_MAX, "h_Temp, height of temperature sensor above water surface.", false);
    bufr->addValue(  4024, data->tTAX_N, "tTAX_N_1, time periode", false);
    bufr->addValue(  4024, (data->tTAX_N!=INT_MAX?0:INT_MAX), "tTAX_N_2, time periode", false );
    bufr->addValue( 12111, data->TAX_N, "TAX_N, maximum temperature");
    bufr->addValue(  4024, data->tTAN_N, "tTAN_N_1, time periode", false);
    bufr->addValue(  4024, (data->tTAN_N!=INT_MAX?0:INT_MAX), "tTAN_N_2, time periode", false );
    bufr->addValue( 12112, data->TAN_N, "TAN_N, maximum temperature");
}

