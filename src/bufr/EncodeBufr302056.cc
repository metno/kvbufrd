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
#include "EncodeBufr302056.h"

EncodeBufr302056::
EncodeBufr302056()
{

}
std::string
EncodeBufr302056::
logIdentifier() const
{
   return "302056";
}

std::list<int>
EncodeBufr302056::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302056);

   return ids;
}



void
EncodeBufr302056::
encode( )
{
   int nRep=1;

   if( hasReplicator() ) {
      if( isDelayedReplicator() ) {
         nRep = (data->TW.valid()?1:0);
         bufr->addDelayedReplicationFactor( replicator, nRep );
      } else {
         nRep = replicator;
      }
   }

   if( nRep == 0 )
      return;

   //Sea/water surface temperature, method of measurement, and depth below sea surface
   bufr->addValue( 2038, 14, "Method of sea/water temperature measurement", false );
   bufr->addValue( 7063, 0.5f, "Sea/water depth of measurement", false );
   bufr->addValue( 22043, data->TW, "TW, sea/water temperature" );
   bufr->addValue( 7063, FLT_MAX, "Sea/water depth of measurement", false );

   for( int i=1; i<nRep; ++i ) {
      bufr->addValue( 2038, INT_MAX, "Method of sea/water temperature measurement", false );
      bufr->addValue( 7063, FLT_MAX, "Sea/water depth of measurement", false );
      bufr->addValue( 22043, FLT_MAX, "TW, sea/water temperature" );
      bufr->addValue( 7063, FLT_MAX, "Sea/water depth of measurement", false );
   }
}

