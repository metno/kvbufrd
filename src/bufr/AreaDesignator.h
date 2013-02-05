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

#ifndef __AREADESIGNATOR_H__
#define __AREADESIGNATOR_H__

#include <string>

/**
 * | A1 |  Geographical Area Designator    |
 * |:--:|:---------------------------------|
 * | A  | 0° - 90°W northern hemisphere    |
 * | B  | 90°W - 180° northern hemisphere  |
 * | C  | 180° - 90°E northern hemisphere  |
 * | D  | 90°E - 0° northern hemisphere    |
 * | E  | 0° - 90°W tropical belt          |
 * | F  | 90°W - 180° tropical belt        |
 * | G  | 180° - 90°E tropical belt        |
 * | H  | 90°E - 0° tropical belt          |
 * | I  | 0° - 90°W southern hemisphere    |
 * | J  | 90°W - 180° southern hemisphere  |
 * | K  | 180° - 90°E southern hemisphere  |
 * | L  | 90°E - 0° southern hemisphere    |
 * | N  | Northern hemisphere              |
 * | S  | Southern hemisphere              |
 * | T  | 45°W - 180° northern hemisphere  |
 * | X  | Global Area (area not definable) |
 *
 * At the moment Area designators N, S and T is not used.
 * The area designator for Norway is D.
 *
 * The topical belt is according to wikipedia between the
 * latitudes 23° 26′ 16 N and  23° 26′ 16 S
 *
 */
std::string
computeAreaDesignator( float longitude, float latitude );



#endif
