/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: InitLogger.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $

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

#include <strings.h>
#include "parseMilogLogLevel.h"

milog::LogLevel
parseMilogLogLevel(const std::string &logLevel)
{
   if( strcasecmp(logLevel.c_str(),"FATAL") == 0 ){
      return milog::FATAL;
   }else if( strcasecmp(logLevel.c_str(),"ERROR") == 0 ){
      return milog::ERROR;
   }else if( strcasecmp(logLevel.c_str(),"WARN") == 0 ){
      return milog::WARN;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG") == 0 ){
      return milog::DEBUG;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG1") == 0 ){
      return milog::DEBUG1;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG2") == 0 ){
      return milog::DEBUG2;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG3") == 0 ){
      return milog::DEBUG3;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG4") == 0 ){
      return milog::DEBUG4;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG5") == 0 ){
      return milog::DEBUG5;
   }else if( strcasecmp(logLevel.c_str(),"DEBUG6") == 0 ){
      return milog::DEBUG6;
   }else if( strcasecmp(logLevel.c_str(),"INFO") == 0 ){
      return milog::INFO;
   }else if( logLevel == "0" ){
      return milog::FATAL;
   }else if( logLevel == "1" ){
      return milog::ERROR;
   }else if( logLevel == "2" ){
      return milog::WARN;
   }else if( logLevel == "3" ){
      return milog::INFO;
   }else if( logLevel == "4" ){
      return milog::DEBUG;
   }else if( logLevel == "5" ){
      return milog::DEBUG1;
   }else if( logLevel == "6" ){
      return milog::DEBUG2;
   }else if( logLevel == "7" ){
      return milog::DEBUG3;
   }else if( logLevel == "8" ){
      return milog::DEBUG4;
   }else if( logLevel == "9" ){
      return milog::DEBUG5;
   }else if( logLevel == "10" ){
      return milog::DEBUG6;
   }else{
      return milog::NOTSET;
   }
}

