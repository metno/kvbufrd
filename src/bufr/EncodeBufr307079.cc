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
#include "EncodeBufr307079.h"

namespace b=boost;


EncodeBufr307079::
EncodeBufr307079()
{
}
EncodeBufr307079::
~EncodeBufr307079()
{
}


std::string
EncodeBufr307079::
logIdentifier() const
{
   return "307079";
}

std::list<int>
EncodeBufr307079::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(307079);

   return ids;
}

void
EncodeBufr307079::
encode(  )
{
   bufr->setObsTime( obstime );

   /*
    * Fixed surface station identification, time,
    * horizontal and vertical coordinates
    */
   encodeTemplate( 301090 );
   encodeTemplate( 302031 ); //Pressure data.

   encodeTemplate( 302035 );
   encodeTemplate( 302036 );

   /* 3 02 047
    * Direction of cloud drift (56DLDMDH)
    * Not implemented
    */
   bufr->addDelayedReplicationFactor( 31000, 0, "Delayed replication");

   bufr->addValue( 8002, INT_MAX, "Vertical significance", false);

   /* 3 02 048
    * Direction and elevation of clouds (57CDaec)
    * Not implemented.
    */
   bufr->addDelayedReplicationFactor( 31000, 0, "Delayed replication");

   // State of ground, snow depth, ground minimum temperature
   encodeTemplate( 302037 );


   /* State of the sea. */
   bufr->addDelayedReplicationFactor( 31000, (data->SG.valid()?1:0), "Delayed replication");

   if( data->SG.valid() ) {
      bufr->addValue( 22061, data->SG.valAsInt(), "SG, state of sea");
      bufr->addValue( 20058, FLT_MAX, "Vs, Visibility seawards from a coastal station");
   }

   // Sea/water temperature
   encodeTemplate( 302056, 31000 );

   /* 3 02 055 Icing and Ice.
    * At the momment we do not report this.
    */
   bufr->addDelayedReplicationFactor( 31000, 0, "Delayed replication");

   //Basic synoptic "periode " data.

   encodeTemplate( 302043 );

   /* 3 02 044
    * Evaporation data.
    * Not implemented.
    */
   bufr->addValue(  4024, INT_MAX, "Time periode in hours", false);
   bufr->addValue(  2004, INT_MAX, "Type of instrumentation", false);
   bufr->addValue( 13033, INT_MAX, "Evaporation");

   /* 3 02 045
    * Radiation data (1 hour and 24 hour period)
    * Not implemented.
    */
   bufr->addDelayedReplicationFactor( 31000, 0, "Delayed replication");

   /* 3 02 046
    * Temperature change (54g0sndT)
    * Not implemented
    */
   bufr->addDelayedReplicationFactor( 31000, 0, "Delayed replication");
}

