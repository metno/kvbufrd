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
#include "EncodeBufr302021.h"
#include <boost/assign.hpp>
#include <float.h>

EncodeBufr302021::EncodeBufr302021() {}
std::string EncodeBufr302021::logIdentifier() const { return "302021"; }

std::list<int> EncodeBufr302021::encodeIds() const {
  std::list<int> ids;
  boost::assign::push_back(ids)(302021);

  return ids;
}

/**
 * Encode Waves data as amplitude and direction.
 * There is two parameters that may be actual
 * for each component:
 *
 *   - amplitude Pwa or WTZ
 *   - direction Hwa or WHM0.
 *
 * If both parameters exist for the amplitude, Pwa has priority.
 * If both parameters exist for direction, Hwa has priority.
 */
void EncodeBufr302021::encode() {
  float pw = data->PWA;
  float wa = data->Hwa;
  std::string usedWindWaveParam("Pwa");

  if (pw == FLT_MAX) {
    usedWindWaveParam = "WTZ";
    pw = data->WTZ;
  }

  if (pw == FLT_MAX) {
    pw = data->WTM02;
    usedWindWaveParam = "WTM02";
  }

  if (wa == FLT_MAX)
    wa = data->WHM0;

  bufr->addValue(22001, data->WDMT, "WDMT, direction of waves");
  bufr->addValue(22011, pw,
                 usedWindWaveParam +
                     ", period of wind waves. Automatic observation.");
  bufr->addValue(22021, wa, "Hwa, height of waves. Automatic observation.");
}
