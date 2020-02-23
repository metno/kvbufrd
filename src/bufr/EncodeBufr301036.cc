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
#include "EncodeBufr301036.h"

EncodeBufr301036::
EncodeBufr301036()
{

}
std::string
EncodeBufr301036::
logIdentifier() const
{
   return "301036";
}

std::list<int>
EncodeBufr301036::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(301036);

   return ids;
}



void
EncodeBufr301036::
encode( )
{
	//Dirty: Hack to anonymize the identity to the "coast guard".
	if( stationInfo->owner() == "KYST")
		bufr->addValue( 1011, "SHIP", "callsign", false );
	else
		bufr->addValue( 1011, stationInfo->callsign(), "callsign", false );

   bufr->addValue( 1012, data->MDIR, "Direction of motion of moving ship/platform", false);
   bufr->addValue( 1013, data->MSPEED, "Speed of motion of moving ship/platform", false);
   bufr->addValue( 2001, data->IX.valAsInt(), "ix", false );
   bufr->addValue( 4001, data->time().date().year(), "Year", false);
   bufr->addValue( 4002, data->time().date().month(), "Month", false);
   bufr->addValue( 4003, data->time().date().day(), "Day", false);
   bufr->addValue( 4004, static_cast<int>(data->time().time_of_day().hours()), "Hour", false);
   bufr->addValue( 4005, static_cast<int>(data->time().time_of_day().minutes()), "Minute", false);

   if( data->MLAT.valid() && data->MLON.valid() ) {
	   bufr->addValue( 5002, data->MLAT, "Latitude", false);
	   bufr->addValue( 6002, data->MLON, "Longitude", false);
   } else {
	   bufr->addValue( 5002, stationInfo->latitude(), "Latitude", false);
	   bufr->addValue( 6002, stationInfo->longitude(), "Longitude", false);
   }
}

