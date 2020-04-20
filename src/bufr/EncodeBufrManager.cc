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

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <milog/milog.h>
#include "EncodeBufrManager.h"
#include "EncodeBufrBase.h"
#include "EncodeBufr301004.h"
#include "EncodeBufr301011.h"
#include "EncodeBufr301012.h"
#include "EncodeBufr301021.h"
#include "EncodeBufr301036.h"
#include "EncodeBufr301089.h"
#include "EncodeBufr301090.h"
#include "EncodeBufr301093.h"
#include "EncodeBufr301126.h"
#include "EncodeBufr302001.h"
#include "EncodeBufr302004.h"
#include "EncodeBufr302005.h"
#include "EncodeBufr302021.h"
#include "EncodeBufr302023.h"
#include "EncodeBufr302024.h"
#include "EncodeBufr302031.h"
#include "EncodeBufr302032.h"
#include "EncodeBufr302033.h"
#include "EncodeBufr302034.h"
#include "EncodeBufr302035.h"
#include "EncodeBufr302036.h"
#include "EncodeBufr302037.h"
#include "EncodeBufr302038.h"
#include "EncodeBufr302039.h"
#include "EncodeBufr302040.h"
#include "EncodeBufr302041.h"
#include "EncodeBufr302042.h"
#include "EncodeBufr302043.h"
#include "EncodeBufr302052.h"
#include "EncodeBufr302053.h"
#include "EncodeBufr302054.h"
#include "EncodeBufr302055.h"
#include "EncodeBufr302056.h"
#include "EncodeBufr302057.h"
#include "EncodeBufr302058.h"
#include "EncodeBufr302059.h"
#include "EncodeBufr302060.h"
#include "EncodeBufr302082.h"
#include "EncodeBufr302091.h"
#include "EncodeBufr306004.h"
#include "EncodeBufr306005.h"
#include "EncodeBufr306038.h"
#include "EncodeBufr306039.h"
#include "EncodeBufr306040.h"
#include "EncodeBufr306041.h"
#include "EncodeBufr307079.h"
#include "EncodeBufr308009.h"
#include "EncodeBufr315008.h"
#include "EncodeBufr900000_SYNOP.h"
#include "EncodeBufr900001_Bstations.h"
#include "EncodeBufr900002_PRECIP_AND_SNOW.h"
#include "EncodeBufr900003_SHIP.h"
#include "EncodeBufr900004_SVV.h"
#include "EncodeBufr900005_MOORED_BUOY.h"
#include "GuessBufrTemplate.h"


namespace b=boost;

int EncodeBufrManager::masterBufrTable = 31;

EncodeBufrManager::
EncodeBufrManager()
{
   addEncoder( new EncodeBufr301004() );
   addEncoder( new EncodeBufr301011() );
   addEncoder( new EncodeBufr301012() );
   addEncoder( new EncodeBufr301021() );
   addEncoder( new EncodeBufr301036() );
   addEncoder( new EncodeBufr301089() );
   addEncoder( new EncodeBufr301090() );
   addEncoder( new EncodeBufr301093() );
   addEncoder( new EncodeBufr301126() );
   addEncoder( new EncodeBufr302001() );
   addEncoder( new EncodeBufr302004() );
   addEncoder( new EncodeBufr302005() );
   addEncoder( new EncodeBufr302021() );
   addEncoder( new EncodeBufr302023() );
   addEncoder( new EncodeBufr302024() );
   addEncoder( new EncodeBufr302031() );
   addEncoder( new EncodeBufr302032() );
   addEncoder( new EncodeBufr302033() );
   addEncoder( new EncodeBufr302034() );
   addEncoder( new EncodeBufr302035() );
   addEncoder( new EncodeBufr302036() );
   addEncoder( new EncodeBufr302037() );
   addEncoder( new EncodeBufr302038() );
   addEncoder( new EncodeBufr302039() );
   addEncoder( new EncodeBufr302040() );
   addEncoder( new EncodeBufr302041() );
   addEncoder( new EncodeBufr302042() );
   addEncoder( new EncodeBufr302043() );
   addEncoder( new EncodeBufr302052() );
   addEncoder( new EncodeBufr302053() );
   addEncoder( new EncodeBufr302054() );
   addEncoder( new EncodeBufr302055() );
   addEncoder( new EncodeBufr302056() );
   addEncoder( new EncodeBufr302057() );
   addEncoder( new EncodeBufr302058() );
   addEncoder( new EncodeBufr302059() );
   addEncoder( new EncodeBufr302060() );
   addEncoder( new EncodeBufr302082() );
   addEncoder( new EncodeBufr302091() );
   addEncoder( new EncodeBufr306004() );
   addEncoder( new EncodeBufr306005() );
   addEncoder( new EncodeBufr306038() );
   addEncoder( new EncodeBufr306039() );
   addEncoder( new EncodeBufr306040() );
   addEncoder( new EncodeBufr306041() );
   addEncoder( new EncodeBufr307079() );
   addEncoder( new EncodeBufr308009() );
   addEncoder( new EncodeBufr315008() );
   addEncoder( new EncodeBufr900000() );
   addEncoder( new EncodeBufr900001() );
   addEncoder( new EncodeBufr900002() );
   addEncoder( new EncodeBufr900003() );
   addEncoder( new EncodeBufr900004() );
   addEncoder( new EncodeBufr900005() );
}

bool
EncodeBufrManager::
addEncoder( EncodeBufrBase *encoder_ )
{
   b::shared_ptr<EncodeBufrBase> encoder( encoder_ );
   std::list<int> ids = encoder->encodeIds();

   for( std::list<int>::iterator it=ids.begin(); it != ids.end(); ++it )
      encoders[*it] = encoder;
}

void
EncodeBufrManager::
encode( int templateid, BufrHelper &bufrHelper, int replicator )
{
   CountHelper count( this );
   EncoderList::iterator itEncoder;

   itEncoder = encoders.find( templateid );

   if( itEncoder == encoders.end() ) {
      std::ostringstream o;

      if( templateid >= 300000 && templateid <=399999)
         o << "Template: " << templateid << " is not implemented.";
      else if( templateid >= 10000 && templateid <=99999 )
         o << "Paramid: " << templateid << " is not implemented.";
      else if( templateid >= 100000 && templateid <=199999 )
         o << "Replicator: " << templateid << " is not implemented.";
      else if( templateid >= 900000 && templateid <=999999)
         o << "Internal: " << templateid << " is not implemented.";
      else
         o << "Operator id: " << templateid << " is not implemented.";

      throw NotImplementedException( o.str() );
   }

   itEncoder->second->encode( itEncoder->first, bufrHelper, *this, replicator );
}

void
EncodeBufrManager::
encode( const BufrTemplateList &templateList,
        BufrHelper &bufrHelper )
{
   bool res;

   for( BufrTemplateList::const_iterator it=templateList.begin();
         it != templateList.end(); ++it ) {
      bufrHelper.encoderName = boost::lexical_cast<std::string>(*it);
      encode( *it, bufrHelper );

      if( *it < 400000 ) {
         bufrHelper.addDescriptor( *it );
      }
   }

}

void
EncodeBufrManager::
encode( BufrHelper &bufrHelper )
{
   int bufrCode=bufrHelper.getStationInfo()->code();

   if( bufrCode < 900000 )
         bufrCode += 900000;
     
   std::ostringstream o;
   o << "Using BUFR encoder: "  << bufrCode;

   LOGINFO( o.str() );
   encode( bufrCode, bufrHelper );
}
