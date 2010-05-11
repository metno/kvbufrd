/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufferCltApp.h,v 1.3.2.3 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbufferCltApp_h__
#define __kvbufferCltApp_h__

#include <puTools/miTime.h>
#include <map>
#include <list>
#include <miconfparser/miconfparser.h>
#include "kvbufferCorbaThread.h"

typedef std::map<std::string, std::string>                   TKeyVal;
typedef std::map<std::string, std::string>::iterator        ITKeyVal;
typedef std::map<std::string, std::string>::const_iterator CITKeyVal;

void 
use(int exitcode);


struct Options{
  typedef enum{ Buffer, Uptime, StationList, Delays, Reload, Help,
		CacheReload, Undef} Cmd;
  typedef std::list<int>           IntList;
  typedef std::list<int>::iterator IIntList;

  Options()
    :cmd(Undef), kvserver("kvalobs"), nshost("corbans.met.no")
  {
  }

  Options(const Options &o)
    :cmd(o.cmd), kvserver(o.kvserver), nshost(o.nshost),
       time(o.time), wmonoList(o.wmonoList)
       
  {
  }

  Options& operator=(const Options &rhs){
    if(this!=&rhs){
      cmd=rhs.cmd; 
      kvserver=rhs.kvserver; 
      nshost=rhs.nshost;
      time=rhs.time; 
      wmonoList=rhs.wmonoList;
    }
    return *this;
  }

  Cmd cmd;

  std::string kvserver;
  std::string nshost;
  miutil::miTime time;
  IntList  wmonoList;
};  


class BufferCltApp
{
  bool getOptions(int argn, char **argv, miutil::conf::ConfSection *conf, Options &opt);
  Options opt;
  BufferCltCorbaThread   *corbaThread;
  kvbufferd::buffercb_var buffercb;
  static BufferCltApp    *app;
  bool                  shutdown_;
  CorbaHelper::CorbaApp *capp; 
  kvbufferd::buffer_var   buffer;
  bool                  wait_;
  kvbufferd::BufferData   result;
 public:
  BufferCltApp(int argn, char **argv, miutil::conf::ConfSection *conf );
  ~BufferCltApp();

  Options options() const{ return opt;}
  void doShutdown();
  bool shutdown()const;

  bool uptime(miutil::miTime &startTime, long &uptime);
  bool stationsList(kvbufferd::StationInfoList &infoList);
  bool wait()const;
  void wait(bool w);

  void setResult(const kvbufferd::BufferData &d){ result=d;}

  bool createBuffer(int wmono,
		   const miutil::miTime &obstime,
		   const TKeyVal &keyvals,
		   int timeoutInSeconds,
		   kvbufferd::BufferData &result);

  bool delayList(kvbufferd::DelayList &delayList, miutil::miTime &nowTime);
  
  kvbufferd::ReloadList* cacheReloadList(std::string &message);

  bool reloadConf();

  void run();
};


#endif
