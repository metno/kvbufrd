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
#include "EncodeBufr302054.h"

EncodeBufr302054::
EncodeBufr302054()
{

}
std::string
EncodeBufr302054::
logIdentifier() const
{
   return "302054";
}

std::list<int>
EncodeBufr302054::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302054);

   return ids;
}



void
EncodeBufr302054::
encode( )
{
	encodeTemplate( 302052 );
	encodeTemplate( 302053 );
	bufr->addValue( 7033, FLT_MAX, "Height of sensor above water surface.", false);
	encodeTemplate( 302034 );
	bufr->addValue( 7032, FLT_MAX, "Height of sensor above marine deck platform.", false);
	encodeTemplate( 302004 );
	encodeTemplate( 302005, 31001 );
}
