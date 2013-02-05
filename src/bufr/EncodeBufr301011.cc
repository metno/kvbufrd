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
#include "EncodeBufr301011.h"

EncodeBufr301011::
EncodeBufr301011()
{

}
std::string
EncodeBufr301011::
logIdentifier() const
{
   return "301011";
}

std::list<int>
EncodeBufr301011::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(301011);

   return ids;
}



void
EncodeBufr301011::
encode( )
{
   bufr->addValue( 4001, static_cast<float>( obstime.year() ), "Year", false );
   bufr->addValue( 4002, static_cast<float>( obstime.month() ), "Month", false );
   bufr->addValue( 4003, static_cast<float>( obstime.day() ), "YY (Day)", false );
}

