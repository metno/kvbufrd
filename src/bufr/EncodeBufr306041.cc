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
#include "EncodeBufr306041.h"

EncodeBufr306041::
EncodeBufr306041()
{

}
std::string
EncodeBufr306041::
logIdentifier() const
{
   return "306041";
}

std::list<int>
EncodeBufr306041::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306041);

   return ids;
}



void
EncodeBufr306041::
encode( )
{
  auto tw = data->TW.getBySensorsAndLevels();
  bufr->addValue(2032, (tw.size()>0?0:3), "Indicator of digitization (tbl code 3 = missing)");
  bufr->addDelayedReplicationFactor(31001, tw.size());
  for( auto &s : tw) {
    bufr->addValue(7062, s.first, "TW.level");
    bufr->addValue(22043, s.second, "TW", true);
  }
}

