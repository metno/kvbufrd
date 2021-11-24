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
#include "EncodeBufr306005.h"

EncodeBufr306005::
EncodeBufr306005()
{

}
std::string
EncodeBufr306005::
logIdentifier() const
{
   return "306005";
}

std::list<int>
EncodeBufr306005::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306005);

   return ids;
}



void
EncodeBufr306005::
encode( )
{
  auto CD = data->CD.getSensor(0);
  auto CV = data->CV.getSensor(0);

  bufr->addValue( 2031, 31, "Duration and time of current measurement (code tbl 31 = missing)", false );
  bufr->addDelayedReplicationFactor(31001, CV.levels.size(), "Number of CV depths");
  float cd;
  for ( auto &e : CV.levels ) {
    bufr->addValue(7062, e.first, "CV-depth");
    cd = CD.getLevel(e.first);
    if( cd != FLT_MAX) {
      bufr->addValue(22004, static_cast<int>(cd + 0.5), "CD");
    } else {
      bufr->addValue(22004, INT_MAX, "CD");
    }
    bufr->addValue(22031, e.second.value, "CV");
  }
}

