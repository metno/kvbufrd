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
#include "EncodeBufr301004.h"

EncodeBufr301004::
EncodeBufr301004()
{

}
std::string
EncodeBufr301004::
logIdentifier() const
{
   return "301004";
}

std::list<int>
EncodeBufr301004::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(301004);

   return ids;
}



void
EncodeBufr301004::
encode( )
{
   //WMO block number  II
   bufr->addValue( 1001, static_cast<int>( stationInfo->wmono()/1000 ), "II", false );
   //WMO station number  iii*
   bufr->addValue(1002, static_cast<int>( stationInfo->wmono()%1000 ), "iii", false);
   bufr->addValue(1015, stationInfo->name(), "Site name", false);
   bufr->addValue(2001, data->IX.valAsInt(), "ix", false );

}

