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
#include "EncodeBufr302045.h"

EncodeBufr302045::
EncodeBufr302045()
{

}
std::string
EncodeBufr302045::
logIdentifier() const
{
   return "302045";
}

std::list<int>
EncodeBufr302045::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302045);

   return ids;
}



void
EncodeBufr302045::
encode( )
{
  
  bufr->addValue(4024, static_cast<float>(-1), "Time period in hours", false, "Radiation period" );
  bufr->addValue(14002, (data->QLI.valid()?data->QLI:FLT_MAX), "Long-wave radiation, integrated over period specified", true, "Radiation");
  bufr->addValue(14004, FLT_MAX, "Short-wave radiation, integrated over period specified", false, "Radiation");
  bufr->addValue(14016, FLT_MAX, "Net radiation, integrated over period specified", false, "Radiation");
  bufr->addValue(14028, (data->QSI.valid()?data->QSI:FLT_MAX), "Global solar radiation (high accuracy), integrated over period specified", true, "Radiation");
  bufr->addValue(14029, (data->QD.valid()?data->QD:FLT_MAX), "Diffuse solar radiation (high accuracy), integrated over period specified", true, "Radiation");   
  bufr->addValue(14030, (data->QS.valid()?data->QS:FLT_MAX), "Direct solar radiation (high accuracy), integrated over period specified", true, "Radiation");   
}

