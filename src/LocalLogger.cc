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

#include <sstream>
#include "LocalLogger.h">

namespace milog {

///Create a logger and set is as the default logger for the tread.
LocalLogger::LocalLogger(const std::string &path, const milog::LogLevel &level)
  : id_(""), isDefault_(true){
  init(path, level);
}

  ///Create a logge and register it with the name 'id'.
LocalLogger::LocalLogger(const std::string &id, const std::string &path, const milog::LogLevel &level=milog::INFO)
: id_(id), isDefault_(false){
  init(path, level);
}

void LocalLogger::init(const std::string &path, const milog::LogLevel &level)
{
  old=NOTSET;
  try {
    FLogStream *logs = new FLogStream(1, 204800);  //200k
    if (logs->open(path)) {
      if( isDefault_)
        Logger::setDefaultLogger(logs);
      else
        Logger::createLogger(id, logs);

      old=Logger::logger().logLevel();
      Logger::logger().logLevel(level);
    } else {
      LOGERROR("Cant open the logfile <" << path << ">!");
      delete logs;
    }
  } catch (...) {
    LOGERROR("Cant create a logstream for id: '" << (id_.empty()?"__default__":id_) << "' path: " << path );
  }
}

LocalLogger::~LocalLogger(){
  if( old != NOTSET ) {
    if( isDefault_ )
      Logger::resetDefaultLogger();
    else
      Logger::removeLogger(id);
    Logger::logger().logLevel(old);
  }
}

}

