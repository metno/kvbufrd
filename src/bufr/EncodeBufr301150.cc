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
#include "EncodeBufr301150.h"

EncodeBufr301150::
EncodeBufr301150()
{

}
std::string
EncodeBufr301150::
logIdentifier() const
{
   return "301150";
}

std::list<int>
EncodeBufr301150::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(301150);

   return ids;
}



void
EncodeBufr301150::
encode( )
{
   if( !stationInfo->wigosIdIsDefined() ) {
      bufr->validBufr(false);
      bufr->addErrorMessage("Missing WIGOS id.");
   }
   int identifierSeries;
   int issuerOfIdentifier;
   int issueNumber;
   std::string localIdentifier;
   stationInfo->getWigosId(identifierSeries, issuerOfIdentifier,issueNumber,localIdentifier);

   bufr->addValue(1125, static_cast<int>(identifierSeries), "WIGOS identifier series", false);
   bufr->addValue(1126, static_cast<int>(issuerOfIdentifier), "WIGOS issuer of identifier", false);
   bufr->addValue(1127, static_cast<int>(issueNumber), "WIGOS issue number", false);
   bufr->addValue(1128, localIdentifier, "WIGOS local identifier", false);

}

