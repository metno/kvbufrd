/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: bufr.h,v 1.12.2.5 2007/09/27 09:02:18 paule Exp $

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

#ifndef __kvbufrd_util_h__
#define __kvbufrd_util_h__

#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <kvalobs/milog/milogtypes.h>

namespace util {
  namespace file {
    /**
     * Returns the modification time for the file given with path. Returns the modification time
     * if the file exist and an undefined time if the file do not exist. Other error give an exception.
     * 
     * @param path path to the file
     * @return the modification time if the file exist and an undefined time if the file do not exist.
     * @exception std::runtime_error on other errors.
     */
    boost::posix_time::ptime modificationTime(const std::string &path);
  }
}

std::ostream &operator<<(std::ostream &o, milog::LogLevel ll);

#endif