/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Waiting.h,v 1.3.6.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __kvbufferd_waiting_h__
#define __kvbufferd_waiting_h__

#include <list>
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/shared_ptr.hpp"
#include "kvdb/kvdb.h"
#include "miutil/timeconvert.h"
#include "StationInfo.h"

class Waiting
{
  Waiting();
  Waiting(const Waiting &w);
  Waiting& operator=(const Waiting &);
  
  boost::posix_time::ptime delay_;
  boost::posix_time::ptime obstime_;
  StationInfoPtr info_;
  bool           waitingOnContiniusData_;
  int            count_; //How many times has this observation
                         //been waiting. To prevent an unlimited loop.
  std::string    note_;
  
 public:
  Waiting(const boost::posix_time::ptime &delay,
	  const boost::posix_time::ptime &obstime,
	  StationInfoPtr info,
    const std::string    &note,
    bool  waitingOnConData=false):
    delay_(delay),
    obstime_(obstime),
    info_(info),
    waitingOnContiniusData_(waitingOnConData),
    count_(0),
    note_(note)
    {
      
    }

  boost::posix_time::ptime delay()const{ return delay_;}
  boost::posix_time::ptime obstime()const{ return obstime_;}
  StationInfoPtr info()const{ return info_;}

  bool addToDb();
  bool removeFrom();
  bool inDb();
  
  bool waitingOnContinuesData()const{  return waitingOnContiniusData_;}
  void waitingOnContinuesData(bool f){ waitingOnContiniusData_=f;}
  int  count(){ return count_;} 
  void incCount(){ count_++;}
  void note(const std::string &n) { note_=n;}
  std::string note()const { return note_;}

  friend std::ostream& operator<<(std::ostream& ost,
				  const Waiting& sd);

};


typedef boost::shared_ptr<Waiting> WaitingPtr;

std::ostream& operator<<(std::ostream& ost,
			 const Waiting& w);


inline std::ostream& operator<<(std::ostream& ost,
				const Waiting& w)
{
  using boost::posix_time::to_kvalobs_string;

  ost << "Waiting: " << w.info_->toIdentString() << " obstime: "
      << to_kvalobs_string(w.obstime_) << " delay: "<< to_kvalobs_string(w.delay_);

  return ost;
}


typedef std::list<WaitingPtr>                   WaitingList; 
typedef std::list<WaitingPtr>::iterator        IWaitingList; 
typedef std::list<WaitingPtr>::const_iterator CIWaitingList; 


#endif
