/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/filesystem.hpp>
#include "LogAppender.h"
#include <kvalobs/kvPath.h>
#include <milog/milog.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <chrono>
#include <cstdio>
#include <cstring>

namespace fs = boost::filesystem;

namespace {}
int openFile(const std::string &file, std::string *err) {
  int fd = open(file.c_str(), O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd < 0) {
    const int BUF_SIZE = 512;
    char msg[BUF_SIZE];
    auto ignore_return_value=strerror_r(errno, msg, BUF_SIZE);
    msg[BUF_SIZE-1]='\0';
    std::ostringstream o;
    o << "Faile to creat or open log file '" << file << "'. " << msg;
    *err = o.str();
    LOGWARN("Error when creating/open log file '" << file << "'. " << msg);
    return -1;
  } 

  return fd;
}

std::string timestamp() {
  struct tm stm;
  auto now=std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  gmtime_r(&t, &stm);
  char timeString[256];
  std::strftime(timeString, 256, "%F %T", &stm);
  return timeString;
}







LogAppender::LogAppender(const std::string &logfile, const std::string &dir):ok_(true)
{
  fs::path path(logfile);

  if( path.is_relative() ) {
    fs::path d(dir);
    path=d/path;
  }

  logfile_=path.string();

  fs::path d=path.parent_path();
  try {
    if( !d.empty() )
     fs::create_directories(d);
  }
  catch( const std::exception &e){
    std::ostringstream o;
    o << "Failed to create directory '" << d << "'. " << e.what();
    lastError_=o.str();
    ok_=false;
  }
}
    
LogAppender::~LogAppender(){  
}

void LogAppender::setLoglevel(const std::string &loglevel){
  loglevel_=loglevel;
}

void LogAppender::setContext(const std::string &context) {
  context_=context;
}

bool LogAppender::log(const std::string &message)  {
  if( message.empty() )
    return true;

  int fd = openFile(logfile_, &lastError_);

  if ( fd < 0 ) {
    return false;
  }

  std::ostringstream o;

  o << timestamp();
  
  if( loglevel_.empty() ) {
    o << " INFO";
  } else {
    o << " " << loglevel_;
  }
    
  if( ! context_.empty() ) {
    o << " (" << context_<< ")";
  }
    
  o << ": " << message;
  
  if( *message.rend() != '\n')
    o << "\n";

  auto m = o.str(); 

  auto s=write(fd, m.c_str(), m.size());
  bool ret=true;
  if( s != m.size() ) {
    std::ostringstream o;
    ret = false;
    if( s<0 ) {
      const int BUF_SIZE = 512;
      char msg[BUF_SIZE];
      auto ignore_return_value = strerror_r(errno, msg, BUF_SIZE);
      msg[BUF_SIZE-1]='\0';
      o << "Failed to write to logfile '" << logfile_ << "'. " << msg;
    } else {
      o << "Failed to write to write complete logline to logfile '" << logfile_ << "'.";
    
    }
    lastError_=o.str();
    LOGWARN(lastError_);
  }
  
  close(fd);
  return ret;
}
