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
#include "EncodeBufr302004.h"

EncodeBufr302004::
EncodeBufr302004()
{

}
std::string
EncodeBufr302004::
logIdentifier() const
{
   return "302004";
}

std::list<int>
EncodeBufr302004::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302004);

   return ids;
}



void
EncodeBufr302004::
encode( )
{
   bufr->addValue( 20010, data->N, "N, Total cloud cover" );
   bufr->addValue( 8002, data->vsci, "vsci, vertical significance", false );
   bufr->addValue( 20011, data->NH.valAsInt(), "NH, low or middle clouds" );
   bufr->addValue( 20013, data->HL, "HL, height of base cloud" );
   bufr->addValue( 20012, data->CL.valAsInt(), "CL, cloud type low clouds" );
   bufr->addValue( 20012, data->CM.valAsInt(), "CM, cloud type middle clouds" );
   bufr->addValue( 20012, data->CH.valAsInt(), "CH, cloud type high clouds" );
}

