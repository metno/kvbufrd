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
#include "EncodeBufr315008.h"

namespace b=boost;


EncodeBufr315008::
EncodeBufr315008()
{
}
EncodeBufr315008::
~EncodeBufr315008()
{
}


std::string
EncodeBufr315008::
logIdentifier() const
{
   return "315008";
}

std::list<int>
EncodeBufr315008::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(315008);

   return ids;
}

void
EncodeBufr315008::
encode(  )
{
   auto count=[](float v) {return v!=FLT_MAX;};
   
   bufr->setObsTime( obstime );

   //Buoy identification and location
   encodeTemplate( 301126 );

   //Standard meteorological data
   encodeTemplate( 306038 );

   //Optional ancillary met data
   if( data->VV.valid() || data->precipRegional.valid() ) {
      bufr->addDelayedReplicationFactor(31000, 1), "Encode TM302091";
       encodeTemplate( 302091 );
   } else {
    bufr->addDelayedReplicationFactor(31000, 0);
   }

   
   //optional radiation measurement
   bufr->addDelayedReplicationFactor(31000, 0, "Encode TM302082");
   //encodeTemplate( 302082 );

   //Basic wave measurements
   if( data->WHM0.hasValidValues() || data->WHMAX.hasValidValues() || data->WTZ.hasValidValues() 
    || data->WTP.hasValidValues() || data->WDP1.hasValidValues() || data->WSPRTP.hasValidValues() 
    || data->WTM02.hasValidValues() || data->PWA.hasValidValues()) {
    bufr->addDelayedReplicationFactor(31000, 1, "Encode TM306039");
    encodeTemplate( 306039 );
  } else {
    bufr->addDelayedReplicationFactor(31000, 0, "Encode TM306039");
  }

   

   //Sequence for representation of detailed spectral wave measurements
   bufr->addDelayedReplicationFactor(31000, 0,"Encode TM306040");
   //encodeTemplate( 306040 );


   //We code either template 3 06 041 or 3 06 004 depending on the 
   //present of salinity data. 
   //If we have salinity data we encode 306004.
   //Encode 306041, if we have only temperature measurements


   //Depth and temperature profile (high accuracy/precision)
   auto ssw = data->SSW.getBySensorsAndLevels(); //salinity
   auto tw = data->TW.getBySensorsAndLevels(); //seawater temperature.

   //No saliniy and we have seawater temperature. Encode 3 06 041
   if ( ssw.size() == 0 && tw.size() > 0) {
      LOGINFO("Encoding template 3 06 041, sea temperature without salinity. Sizes SSW (" << ssw.size() 
         << ") TW (" << tw.size() << "). \n" << data->TW );
      bufr->addDelayedReplicationFactor(31000, 1, "Encode TM306041");
      bufr->addValue(2005, FLT_MAX, "Precision of temperature observation."); //Set to missing for now.
      encodeTemplate( 306041 );
   } else {
      bufr->addDelayedReplicationFactor(31000, 0, "Encode TM306041");
   }

   //Depth, temperature, salinity
   //We have salinity. Encode 3 06 004
   if ( ssw.size() > 0 ) {
      LOGINFO("Encoding template 3 06 004, sea temperature with salinity.");
      bufr->addDelayedReplicationFactor(31000, 1, "Encode TM306004");
      bufr->addValue(2005, FLT_MAX, "Precision of temperature observation."); //Set to missing for now.
      encodeTemplate( 306004 );
   } else {
      bufr->addDelayedReplicationFactor(31000, 0, "Encode TM306004");
   }

   //Sub-surface current measurements
   if( data->CV.getSensor(0).size() == 0) { 
      bufr->addDelayedReplicationFactor(31000, 0,"Encode TM306005");
   } else {
      bufr->addDelayedReplicationFactor(31000, 1, "Encode TM306005");
      encodeTemplate( 306005 );
   }
}

