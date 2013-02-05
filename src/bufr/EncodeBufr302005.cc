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

#include <limits.h>
#include <float.h>
#include <algorithm>
#include <boost/assign.hpp>
#include "EncodeBufr302005.h"

EncodeBufr302005::
EncodeBufr302005()
{

}
std::string
EncodeBufr302005::
logIdentifier() const
{
   return "302005";
}

std::list<int>
EncodeBufr302005::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302005);

   return ids;
}



void
EncodeBufr302005::
encode( )
{
   /* Individual cloud layers or masses */
   /* Looks like Norwegian stations do not report these parameters, except for
    * those sending synop. So the loop below can probably be simplified */


   if( hasReplicator() ) {
      int nRep = data->cloudExtra.size();
      int nMax = nRep;
      int nDoRep;

      if( isDelayedReplicator() ) {
         bufr->addDelayedReplicationFactor( replicator, data->cloudExtra.size(), "# of cloud layers" );
         nDoRep = nRep;
      } else {
         nMax = replicator;
         nDoRep = std::min( nRep, nMax );
      }

      int i;
      for( i=0; i < nDoRep; ++i ) {
         bufr->addValue( 8002, data->cloudExtra[i].vsci, "vsci, vertical significance", false );
         bufr->addValue( 20011, data->cloudExtra[i].Ns, "NH, low or middle clouds" );
         bufr->addValue( 20012, data->cloudExtra[i].C, "CL, cloud type low clouds" );
         bufr->addValue( 20013, data->cloudExtra[i].hshs, "HL, height of base cloud" );
      }

      for( ; i < nMax; ++i ) {
         bufr->addValue( 8002, INT_MAX, "vsci, vertical significance", false );
         bufr->addValue( 20011, INT_MAX, "NH, low or middle clouds" );
         bufr->addValue( 20012, INT_MAX, "CL, cloud type low clouds" );
         bufr->addValue( 20013, FLT_MAX, "HL, height of base cloud" );
      }
   } else {
      //This is just a guess of what to do if this is not
      //a replicating element.
      if( data->cloudExtra.size() > 0 ) {
         bufr->addValue( 8002, data->cloudExtra[0].vsci, "vsci, vertical significance", false );
         bufr->addValue( 20011, data->cloudExtra[0].Ns, "NH, low or middle clouds" );
         bufr->addValue( 20012, data->cloudExtra[0].C, "CL, cloud type low clouds" );
         bufr->addValue( 20013, data->cloudExtra[0].hshs, "HL, height of base cloud" );
      } else {
         bufr->addValue( 8002, data->vsci, "vsci, vertical significance", false );
         bufr->addValue( 20011, data->NH, "NH, low or middle clouds" );

         if( data->CL.valid() )
            bufr->addValue( 20012, data->CL, "CL, cloud type low clouds" );
         else if( data->CM.valid() )
            bufr->addValue( 20012, data->CM, "CM, cloud type middle clouds" );
         else if( data->CH.valid() )
            bufr->addValue( 20012, data->CH, "CH, cloud type high clouds" );
         else
            bufr->addValue( 20012, INT_MAX, "C, cloud type" );

         bufr->addValue( 20013, data->HL, "HL, height of base cloud" );
      }
   }
}

