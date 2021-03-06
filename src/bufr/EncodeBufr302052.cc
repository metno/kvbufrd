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
#include "EncodeBufr302052.h"

EncodeBufr302052::
EncodeBufr302052()
{

}
std::string
EncodeBufr302052::
logIdentifier() const
{
    return "302052";
}

std::list<int>
EncodeBufr302052::
encodeIds()const
{
    std::list<int> ids;
    boost::assign::push_back( ids )(302052);

    return ids;
}



void
EncodeBufr302052::
encode( )
{
    bufr->addValue( 7032,  stationInfo->heightTemperature(), "h_t, height of temperature sensor above marine deck.", false );
    bufr->addValue( 7033,  INT_MAX, "h_t, height of temperature sensor above water surface.", false );
    bufr->addValue( 12101, data->TA, "TA" );

    //When wetbulb temperature is implemented, do set correct value for 2039.
    //bufr->addValue( 2039, 7, "Method of wetbulb temperature measurement.", false );
    bufr->addValue( 2039, INT_MAX, "Method of wetbulb temperature measurement.", false );
    bufr->addValue( 12102, FLT_MAX, "Wetbulb temperature." );

    bufr->addValue( 12103, data->TD, "TD, dew-poin temperature" );
    bufr->addValue( 13003, data->UU, "UU, relativ humidity" );
}

