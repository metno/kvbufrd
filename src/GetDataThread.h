/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetDataThread.h,v 1.4.2.3 2007/09/27 09:02:22 paule Exp $                                                       

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
/* $Header: /var/lib/cvs/kvalobs/src/kvbufferd/GetDataThread.h,v 1.4.2.3 2007/09/27 09:02:22 paule Exp $ */

#ifndef __kvbuffer_getdatathread_h__
#define __kvbuffer_getdatathread_h__

#include <memory>
#include "boost/thread/thread.hpp"
#include "boost/date_time/posix_time/ptime.hpp"

class App;

class GetData {
  	App                      &app;
  	std::shared_ptr<dnmi::thread::CommandQue> que;
  	boost::posix_time::ptime fromTime;
  	boost::shared_ptr<bool>  joinable_;
  	boost::shared_ptr<boost::thread> thread;
  	int                      wmono;
  	int                      hours;

  	void reloadAll(kvalobs::kvDbGateProxy &gate,
		 		   const boost::posix_time::ptime &bufferFromTime);

  	void reloadOne(kvalobs::kvDbGateProxy &gate,
		           const boost::posix_time::ptime &bufferFromTime);

  	void reload(kvalobs::kvDbGateProxy    &gate,
  	          const StationInfoPtr station,
  	          const boost::posix_time::ptime &bufferFromTime);


 	public:
  		GetData(App &app,
	  		    const boost::posix_time::ptime &fromTime,
	  			int                  wmono,
	  			int                  hours,
	  			std::shared_ptr<dnmi::thread::CommandQue> que);

  		void operator()();
 		bool joinable(){ return *joinable_; }
  		void join(){ if(thread) thread->join(); } 
  		void setThread(boost::thread *th){ thread.reset(th);}
}; 



#endif
