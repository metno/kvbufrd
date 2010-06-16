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


/* PS 2007-10-06
 *
 * Usage: encodebufr
 *
 * Example program in C to show how a SYNOP may be encoded into BUFR using
 * WMO template 307080. Calling Fortran library libbufr. 
 *
 * Note: ought to be rewritten using WMO template 307079
 *
 * Output: BUFR file containing the encoded BUFR message, named 'synop.bufr'
 */

#ifndef __ENCODE_BUFR__
#define __ENCODE_BUFR__

#include <stdexcept>
#include "BufrData.h"
#include "StationInfo.h"



class BufrEncodeException : public std::exception
{
   std::string message;

public:
   BufrEncodeException( const std::string &msg )
      : message( msg ) {}
   ~BufrEncodeException() throw (){}

   const char* what() throw() { return message.c_str(); }
};



/**
 * Encode the data in a BufrData to a bufr message.
 *
 * @param data The data to encode to bufr.
 * @param station Station information.
 * @exception BufrEncodeException on error.
 */
void
encodeBufr( const BufrData &data, StationInfoPtr station );

#endif
