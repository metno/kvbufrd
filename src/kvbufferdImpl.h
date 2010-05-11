/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvbufferdImpl.h,v 1.5.2.4 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbufferdImpl_h__
#define __kvbufferdImpl_h__

#include "kvbufferd.hh"
#include "obsevent.h"
#include <kvskel/adminInterface.h>

class App;

class kvBufferdImpl: public virtual POA_kvbufferd::buffer,
		    public virtual micutil::AdminInterface,
		    public PortableServer::RefCountServantBase 
{
  App &app;
  dnmi::thread::CommandQue &que;

public:
  // standard constructor
  kvBufferdImpl(App &app_, dnmi::thread::CommandQue &que_);
  virtual ~kvBufferdImpl();


  CORBA::Boolean createBuffer(CORBA::Short wmono,
			     const char* obstime, 
			     const micutil::KeyValList& keyVals,
			     kvbufferd::buffercb_ptr callback);
  CORBA::Boolean stations(kvbufferd::StationInfoList_out infoList);
  CORBA::Boolean uptime(CORBA::String_out startTime, 
			CORBA::Long& uptimeInSeconds);
  CORBA::Boolean delays(CORBA::String_out nowTime, 
			kvbufferd::DelayList_out dl);
  CORBA::Boolean reloadConf(CORBA::String_out message);
  CORBA::Boolean reloadCache(const char* fromTime, 
			     kvbufferd::ReloadList_out wmonolist,
			     CORBA::String_out message);
  CORBA::Boolean getbuffer(const kvbufferd::WmoNoList& wmoList,
			  const char* fromtime, const char* totime, 
			  kvbufferd::BufferList_out buffers,
			  CORBA::String_out message);
  CORBA::Boolean getdata(CORBA::Short wmono, const char* obstime, 
			 kvbufferd::DataElementList_out datalist,
			 CORBA::String_out message);


};




#endif
