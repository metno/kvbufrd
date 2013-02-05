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
#include "EncodeBufr302059.h"

EncodeBufr302059::
EncodeBufr302059()
{
}

std::string
EncodeBufr302059::
logIdentifier() const
{
    return "302059";
}

std::list<int>
EncodeBufr302059::
encodeIds()const
{
    std::list<int> ids;
    boost::assign::push_back( ids )(302059);

    return ids;
}



void
EncodeBufr302059::
encode( )
{
    /* Wind data */
    bufr->addValue(  7032, stationInfo->heightWind(), "h_Wind, height of sensor above marine deck.", false );
    bufr->addValue(  7033, stationInfo->heightWindAboveSea(), "h_Wind, height of sensor above water surface.", false );
    bufr->addValue(  2002, 8, "Type of instrumentation");
    bufr->addValue(  8021, 2, "Time significance (=2: time averaged)", false);
    bufr->addValue(  4025, static_cast<float>(-10), "Time period or displacement (minutes)", false );
    bufr->addValue( 11001, data->DD, "DD, wind direction");
    bufr->addValue( 11002, data->FF, "FF, wind speed");
    //bufr->addValue(  8021, 2, "Time significance (=2: time averaged)", false);
    bufr->addValue(  8021, INT_MAX, "Time significance", false);
    bufr->addValue(  4025, (data->FG_010.valid()?static_cast<float>(-10):FLT_MAX), "Time period or displacement (minutes)", false);
    bufr->addValue( 11043, data->DG_010, "DG_010, wind direction (gust)");
    bufr->addValue( 11041, data->FG_010, "FG_010, wind speed (gust)");
    bufr->addValue(  4025, data->FgMax.t, "Time period or displacement (minutes)", false);
    bufr->addValue( 11043, data->FgMax.dd, "FgMax.dd, maximum wind gust direction");
    bufr->addValue( 11041, data->FgMax.ff, "FgMax.ff, maximum wind gust speed");
}

