/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufrdImpl.h,v 1.5.2.4 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbufrdImpl_h__
#define __kvbufrdImpl_h__

#include "kvbufrd.hh"
#include "obsevent.h"
#include <kvskel/adminInterface.h>

class App;

class kvBufrdImpl: public virtual POA_kvbufrd::bufr,
		    public virtual micutil::AdminInterface,
		    public PortableServer::RefCountServantBase 
{
  App &app;
  dnmi::thread::CommandQue &que;

public:
  // standard constructor
  kvBufrdImpl(App &app_, dnmi::thread::CommandQue &que_);
  virtual ~kvBufrdImpl();


  CORBA::Boolean createBufr(CORBA::Short wmono,
			     const char* obstime, 
			     const micutil::KeyValList& keyVals,
			     kvbufrd::bufrcb_ptr callback);
  CORBA::Boolean stations(kvbufrd::StationInfoList_out infoList);
  CORBA::Boolean uptime(CORBA::String_out startTime, 
			CORBA::Long& uptimeInSeconds);
  CORBA::Boolean delays(CORBA::String_out nowTime, 
			kvbufrd::DelayList_out dl);
  CORBA::Boolean reloadConf(CORBA::String_out message);
  CORBA::Boolean reloadCache(const char* fromTime, 
			     kvbufrd::ReloadList_out wmonolist,
			     CORBA::String_out message);
  CORBA::Boolean getbufr(const kvbufrd::WmoNoList& wmoList,
			  const char* fromtime, const char* totime, 
			  kvbufrd::BufrList_out bufrs,
			  CORBA::String_out message);
  CORBA::Boolean getdata(CORBA::Short wmono, const char* obstime, 
			 kvbufrd::DataElementList_out datalist,
			 CORBA::String_out message);


};




#endif
