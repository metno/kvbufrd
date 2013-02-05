/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: BufrData.h,v 1.8.6.6 2007/09/27 09:02:23 paule Exp $

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

#ifndef __SCANSTRING_H__
#define __SCANSTRING_H__

#include <vector>
#include <string>


struct ScanStringSpec {
   int start;
   int with;
   std::string value;

   ScanStringSpec( int start_, int with_ )
      : start( start_ ), with( with_ )
   {
      if( with < 0 ) {//Right justified.
         with *= -1;
         start = start - with + 1;
      }
   }
};


typedef std::vector<ScanStringSpec> ScanStringSpecList;

int scanstring( const std::string &line, ScanStringSpecList &elements );


#endif /* SCANSTRING_H_ */
