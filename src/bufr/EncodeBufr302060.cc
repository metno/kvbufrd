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
#include "EncodeBufr302060.h"

EncodeBufr302060::
EncodeBufr302060()
{

}
std::string
EncodeBufr302060::
logIdentifier() const
{
   return "302060";
}

std::list<int>
EncodeBufr302060::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302060);

   return ids;
}



void
EncodeBufr302060::
encode( )
{
	encodeTemplate( 302038 );
	encodeTemplate( 302040 );
	encodeTemplate( 302058 );
	encodeTemplate( 302059 );
}

