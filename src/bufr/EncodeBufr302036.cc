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
#include "EncodeBufr302036.h"

EncodeBufr302036::
EncodeBufr302036()
{

}
std::string
EncodeBufr302036::
logIdentifier() const
{
   return "302036";
}

std::list<int>
EncodeBufr302036::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302036);

   return ids;
}



void
EncodeBufr302036::
encode( )
{
   bufr->addDelayedReplicationFactor(31001, 0 );

   /* These descriptors is not implemented.
   /* 008002 Vertical significance (surface observations) */
   /* 020011 Cloud amount (N') */
   /* 020012 Cloud type (C') */
   /* 020014 Height of top of cloud (H'H') */
   /* 020017 Height of top of cloud (Ct) */
}

