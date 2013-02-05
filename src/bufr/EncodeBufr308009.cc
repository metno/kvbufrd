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
#include "EncodeBufr308009.h"

namespace b=boost;


EncodeBufr308009::
EncodeBufr308009()
{
}
EncodeBufr308009::
~EncodeBufr308009()
{
}


std::string
EncodeBufr308009::
logIdentifier() const
{
   return "308009";
}

std::list<int>
EncodeBufr308009::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(308009);

   return ids;
}

void
EncodeBufr308009::
encode(  )
{
   bufr->setObsTime( obstime );

   /*
    * Fixed surface station identification, time,
    * horizontal and vertical coordinates
    */
   encodeTemplate( 301093 );
   encodeTemplate( 302001 ); //Pressure data.
   encodeTemplate( 302054 ); //SHIP instantaneous data.
   bufr->addValue( 8002, INT_MAX, "Vertical significance", false);
   encodeTemplate( 302055 ); //Icing and ice.
   encodeTemplate( 302057 ); //Ship marine data.
   encodeTemplate( 302060 ); //Ship period data.
}

