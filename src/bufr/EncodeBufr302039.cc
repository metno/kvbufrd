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
#include "EncodeBufr302039.h"

EncodeBufr302039::
EncodeBufr302039()
{
}

std::string
EncodeBufr302039::
logIdentifier() const
{
   return "302039";
}

std::list<int>
EncodeBufr302039::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302039);

   return ids;
}



void
EncodeBufr302039::
encode( )
{
   //For now do not send out sunsshine data, we hardcode it to missing.
   //Remove the following two lines when the quality of OT_1 and OT_24
   //is ok.
   // data->OT_1 = FLT_MAX;
   // data->OT_24 = FLT_MAX;

   int rep=1;
   if( hasReplicator() ) {
      if( isDelayedReplicator() ) {
         bufr->addDelayedReplicationFactor( replicator, 0, "Delayed replicator" );
      } else {
         rep = replicator;
      }
   }

   if( rep == 1 ) {
      float OT=FLT_MAX;
      float t=FLT_MAX;

      if( data->OT_24.valid() ) {
         OT = data->OT_24;
         t = -24;
      } else if( data->OT_1.valid() ) {
         OT = data->OT_1;
         t = -1;
      }

      bufr->addValue(  4024, t, "t_OT, time periode", false);
      bufr->addValue( 14031, OT, "OT, sunshine past hour");
   } else {
      for( int i=0; i<rep; ++i ) {
         if( i==0 ) {
            bufr->addValue(  4024, data->OT_1.valid()?-1:FLT_MAX, "t_OT_1, time periode", false);
            bufr->addValue( 14031, data->OT_1, "OT_1, sunshine past 1 hour");
         } else if( i == 1 ) {
            bufr->addValue(  4024, data->OT_24.valid()?-24:FLT_MAX, "t_OT_24, time periode", false);
            bufr->addValue( 14031, data->OT_24, "OT_24, sunshine past 24 hour");
         }else {
            bufr->addValue(  4024, FLT_MAX, "t_OT, time periode", false);
            bufr->addValue( 14031, FLT_MAX, "OT, sunshine");
         }
      }
   }
}

