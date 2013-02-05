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

#ifndef __GUESSBUFRTEMPLATE_H__
#define __GUESSBUFRTEMPLATE_H__

#include <BufrHelper.h>

/**
 * Guess which template to use from the typeid and wmono.
 *
 * At the moment the following table describe the relationship.
 *
 * wmono   | typeid |Template
 * --------|--------|----------------
 *  >0     |   x    | 900000 (307079)
 *   x     |  505   | 900001 (SVV)
 *   x     |  302   | 900002 (Precip)
 *   x     |  304   | 900002 (Precip)
 *
 */
BufrTemplateList
guessBufrTemplate( const StationInfo &bufrHelper );


#endif

