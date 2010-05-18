/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufrCltApp.h,v 1.3.2.3 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbufrCltApp_h__
#define __kvbufrCltApp_h__

#include <puTools/miTime.h>
#include <map>
#include <list>
#include <miconfparser/miconfparser.h>
#include "kvbufrCorbaThread.h"

typedef std::map<std::string, std::string>                   TKeyVal;
typedef std::map<std::string, std::string>::iterator        ITKeyVal;
typedef std::map<std::string, std::string>::const_iterator CITKeyVal;

void 
use(int exitcode);


struct Options{
  typedef enum{ Bufr, Uptime, StationList, Delays, Reload, Help,
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


class BufrCltApp
{
  bool getOptions(int argn, char **argv, miutil::conf::ConfSection *conf, Options &opt);
  Options opt;
  BufrCltCorbaThread   *corbaThread;
  kvbufrd::bufrcb_var bufrcb;
  static BufrCltApp    *app;
  bool                  shutdown_;
  CorbaHelper::CorbaApp *capp; 
  kvbufrd::bufr_var   bufr;
  bool                  wait_;
  kvbufrd::BufrData   result;
 public:
  BufrCltApp(int argn, char **argv, miutil::conf::ConfSection *conf );
  ~BufrCltApp();

  Options options() const{ return opt;}
  void doShutdown();
  bool shutdown()const;

  bool uptime(miutil::miTime &startTime, long &uptime);
  bool stationsList(kvbufrd::StationInfoList &infoList);
  bool wait()const;
  void wait(bool w);

  void setResult(const kvbufrd::BufrData &d){ result=d;}

  bool createBufr(int wmono,
		   const miutil::miTime &obstime,
		   const TKeyVal &keyvals,
		   int timeoutInSeconds,
		   kvbufrd::BufrData &result);

  bool delayList(kvbufrd::DelayList &delayList, miutil::miTime &nowTime);
  
  kvbufrd::ReloadList* cacheReloadList(std::string &message);

  bool reloadConf();

  void run();
};


#endif
