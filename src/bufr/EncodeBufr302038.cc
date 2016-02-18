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
#include "EncodeBufr302038.h"

EncodeBufr302038::
EncodeBufr302038()
{
}

std::string
EncodeBufr302038::
logIdentifier() const
{
   return "302038";
}

std::list<int>
EncodeBufr302038::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302038);

   return ids;
}



void
EncodeBufr302038::
encode( )
{
   float t_ww;

   if( (obstime.time_of_day().hours()%6) == 0 )
      t_ww = -6;
   else if( (obstime.time_of_day().hours()%3) == 0 )
      t_ww = -3;
   else
      t_ww = FLT_MAX;

   /* Present and past weather */

   bufr->addValue( 20003, data->ww.valAsInt(), "ww, present weather" );
   bufr->addValue(  4024, t_ww, "t_ww, time periode in hours", false );
   bufr->addValue( 20004, data->W1.valAsInt(), "W1, past weather (1)" );
   bufr->addValue( 20005, data->W2.valAsInt(), "W2, past weather (2)" );
}

