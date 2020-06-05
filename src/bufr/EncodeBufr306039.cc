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
#include "EncodeBufr306039.h"

EncodeBufr306039::
EncodeBufr306039()
{

}
std::string
EncodeBufr306039::
logIdentifier() const
{
   return "306039";
}

std::list<int>
EncodeBufr306039::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(306039);

   return ids;
}



void
EncodeBufr306039::
encode( )
{
  
  auto count=[](float v) {return v!=FLT_MAX;};

  bufr->addValue(22078, FLT_MAX,"DURATION OF WAVE RECORD [S]", false);

  auto v=data->WHM0.getFirstValueAtLevel(0);
  bufr->addValue(22070, v, "SIGNIFICANT WAVE HEIGHT [M]", count(v));
  
  v=data->WHMAX.getFirstValueAtLevel(0);
  bufr->addValue(22073, v, "MAXIMUM WAVE HEIGHT [M]", count(v));

  v=data->WTZ.getFirstValueAtLevel(0);
  if( v == FLT_MAX ){ 
    v=data->WTM02.getFirstValueAtLevel(0); 
  }
  bufr->addValue(22074, v, "AVERAGE WAVE PERIOD [S]", count(v));

  v=data->WTP.getFirstValueAtLevel(0);
  bufr->addValue(22071, v, "SPECTRAL PEAK WAVE PERIOD [S]", count(v));
  
  v=data->WDP1.getFirstValueAtLevel(0);
  bufr->addValue(22076, v, "DIRECTION FROM WHICH DOMINANT WAVES ARE COMING [DEG]", count(v));
 
  v=data->WSPRTP.getFirstValueAtLevel(0);
  bufr->addValue(22077, v, "DIRECTION FROM WHICH DOMINANT WAVES ARE COMING [DEG]", count(v));
}
