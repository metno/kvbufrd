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

#ifndef __ENCODE_BUFR__
#define __ENCODE_BUFR__

#include <stdexcept>
#include "BufrData.h"
#include "StationInfo.h"





class BufrEncoder {
   BufrEncoder();
   BufrEncoder( const BufrEncoder & );
   BufrEncoder& operator=(const BufrEncoder &);

   StationInfoPtr station;
   int kbuflen;
   int *kbuff; /* integer array containing bufr message */
   miutil::miTime obstime;
   int ccx;
   bool test;

public:
   BufrEncoder( StationInfoPtr station_, bool test=false );
   ~BufrEncoder();
   /**
   * Encode the data in a BufrData to a bufr message.
   *
   * @param data The data to encode to bufr.
   * @param station Station information.
   * @exception BufrEncodeException on error.
   */
   void encodeBufr( const BufrData &data, int ccx );
   const char* getBufr( int &nSize )const;

   std::string wmoHeader()const;

   std::string filePrefix()const;

   bool writeToStream( std::ostream &out )const;
   /**
    * @see saveToFile( const std::string &path, bool overwrite=false )const
    * @param path
    * @param overwrite
    */
   void saveToFile( const std::string &path, bool overwrite=false )const;

   /**
    * Save the file to the path defined by copyto in the configuration file.
    *
    * The name of the file is generated to be: wmono-ddhh(-cxxN)(-I).bufr
    *
    *
    * Where N is a number in the range 1-9 and I is a sequence number to make the name uniqe.
    * dd is the day in month and hh is the hour in the day.
    *
    * The -cxxN part is missing if the cxx is 0. The (-I) part is missing if not needed.
    *
    * If overwrite is true the file is unconditionally overwritten if it exist. If overwrite
    * is false the file is not overwritten if the modification time is the same as today, and
    * overwritten if the file is before today.
    *
    * The file is only written if the "copy" parameter in the configuration file is set to true for
    * the station.
    *
    * @param overwrite
    */
   void saveToFile( bool overwrite=false )const;
};

#endif
