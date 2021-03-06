/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiver.h,v 1.6.2.4 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __DataReceiver_h__
#define __DataReceiver_h__

#include <memory>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvData.h"
#include "App.h"
#include "kvevents.h"
#include "KvObsData.h"

class DataReceiver {
  App                      &app;
  std::shared_ptr<threadutil::CommandQueue> inputQue;
  std::shared_ptr<threadutil::CommandQueueBase> outputQue;

 public:
  DataReceiver(App &app, 
               std::shared_ptr<threadutil::CommandQueue> inputQue,
               std::shared_ptr<threadutil::CommandQueueBase> outputQue);

  /**
   * Which data is received for the station given with wmono in
   * obsevent. The obsevent is comming from the delaycontroll thread.
   */
  void doCheckReceivedData(ObsEvent *obsevent);

  /**
   * Save the data to the cache. After the data is saved the cache is
   * looked up to retrive all stations/typeid it is received data for,
   * before an event is sent to BufrWorker.
   * Do not 
   */
  void newData(const DataEvent &event);
  void prepareToProcessAnyBufrBasedOnThisObs(const boost::posix_time::ptime &obstime,
					      StationInfoPtr station);
  bool typeidReceived(ObsEvent &event);
  void regenerateToWaiting(ObsEvent *event, int minutesToDelay);
  
  /**
   * set the default logger to log to file.
   * $KVALOBS/var/kvsynop/dr-'wmono'.log.
   */
  void setDefaultLogger(StationInfoPtr station);

  void operator()();
}; 



#endif

