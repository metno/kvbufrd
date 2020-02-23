/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: DataReceiver.cc,v 1.14.2.20 2007/09/27 09:02:22 paule Exp $

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

#ifndef __SRC_LOCALLOGGER_H__
#define __SRC_LOCALLOGGER_H__

#include <milog/milog.h>

namespace milog {

class LocalLogger {
  std::string id_;
  bool isDefault_;
  LogLevel old;
  void init(const std::string &path, const milog::LogLevel &level);
 public:
  ///Create a logger and set it as the default logger for the tread.
  LocalLogger(const std::string &path, const milog::LogLevel &level=milog::INFO);

  ///Create a logger and register it with the name 'id'.
  LocalLogger(const std::string &id, const std::string &path, const milog::LogLevel &level=milog::INFO);

  /// Remove the logger and possible reset the default logger.
  virtual ~LocalLogger();
};


}



#endif /* __SRC_LOCALLOGGER_H__ */
