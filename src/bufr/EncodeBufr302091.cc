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
#include "EncodeBufr302091.h"

EncodeBufr302091::
EncodeBufr302091()
{
}

std::string
EncodeBufr302091::
logIdentifier() const
{
   return "302091";
}

std::list<int>
EncodeBufr302091::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302091);

   return ids;
}



void
EncodeBufr302091::
encode( )
{

  
   /* Precipitation measurement */
   bufr->addValue( 20001, data->VV, "VV, horisental visibility" );
   bufr->addValue(  4024, data->precipRegional.hTr, "hTr, regional", false);
   bufr->addValue( 13011, data->precipRegional.RR, "RR, regional");
   
}

