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

#ifndef __BUFRDATA_H__
#define __BUFRDATA_H__

#include "DataElementList.h"

/**
 * BufrData add some special elements to use in coding of buffers.
 */
class BufrData : public DataElement
{
public:

   struct Wind {
      float ff;
      float dd;
      float i;

      Wind():
         ff( FLT_MAX ), dd( FLT_MAX ), i( FLT_MAX ) {}
      Wind& operator=( const Wind &rhs )
      {
         if( this != &rhs ) {
            ff = rhs.ff;
            dd = rhs.dd;
            i  = rhs.i;
         }
         return *this;
      }
      friend std::ostream &operator<<(std::ostream &o, const Wind &wind );
   };

   struct Precip {
      float hTr;
      float RR;

      Precip():
         hTr( FLT_MAX ), RR( FLT_MAX ) {}
      Precip& operator=( const Precip &rhs )
      {
         if( this != &rhs ) {
            hTr = rhs.hTr;
            RR = rhs.RR;
         }
         return *this;
      }
      friend std::ostream &operator<<(std::ostream &o, const Precip &precip );
   };

   Wind  FxMax;
   Precip precip24;
   Precip precipRegional;
   Precip precipNational;

   BufrData();
   BufrData( const BufrData &bd );

   BufrData operator=( const BufrData &rhs );
};

std::ostream &operator<<(std::ostream &o, const BufrData::Wind &wind );
std::ostream &operator<<(std::ostream &o, const BufrData::Precip &precip );


#endif /* BUFRDATA_H_ */
