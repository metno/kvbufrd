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

#include <tuple>
#include <boost/assign.hpp>
#include "EncodeBufr306004.h"

EncodeBufr306004::
EncodeBufr306004()
{

}
std::string
EncodeBufr306004::
logIdentifier() const
{
   return "306004";
}

std::list<int>
EncodeBufr306004::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306004);

   return ids;
}




namespace {
  //Get data from all sensor and levels, 
  //lower sensor numbers has priority before higher numbers.
  //map< level, tuple<tw, ssw> >
  std::map<int, std::tuple<float,float> > merge(const KvParam &tw, const KvParam &ssw) {
    using namespace std;
    map<int, std::tuple<float,float> > res;

    for(auto t : tw.getBySensorsAndLevels() ) {
      res[t.first]=make_tuple(t.second, FLT_MAX);
    }

    for(auto t : ssw.getBySensorsAndLevels() ) {
      auto it = res.find(t.first);

      if( it == res.end() ) {
        res[t.first]=make_tuple(FLT_MAX, t.second);
      } else {
        res[it->first]=make_tuple(get<0>(it->second), t.second);
      }
    }

    return res;
  }
}


void
EncodeBufr306004::
encode( )
{
  using namespace std;
  auto count=[](float v) {return v!=FLT_MAX;};
  auto d = merge(data->TW, data->SSW);
  bufr->addValue(2032, 3, "Indicator of digitization (tbl code 3 = missing)");
  bufr->addValue(2033, 7, "Method of salinity/depth measurement (tbl code 7 = missing)");

  bufr->addDelayedReplicationFactor(31001, d.size());
  for( auto &s : d) {
    bufr->addValue(7062, s.first, "SSW.level");
    bufr->addValue(22043, get<0>(s.second), "TW", count(get<0>(s.second)));
    bufr->addValue(22062, get<1>(s.second), "SSW", count(get<1>(s.second)));
  }
}

