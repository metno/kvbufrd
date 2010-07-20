/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblKeyVal.h,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __tblStInfoSysParam_h__
#define __tblStInfoSysParam_h__

#include <kvalobs/kvDbBase.h>

class TblStInfoSysParam : public kvalobs::kvDbBase {
private:
   int paramid_;
   std::string name_;
   int hlevel_scale_;
   int standard_hlevel_;
   int standard_physical_height_;

  void createSortIndex();

public:
  TblStInfoSysParam {clean();}
  TblStInfoSysParam(const TblKeyVal &keyVal){ set(keyVal);}
  TblStInfoSysParam(const dnmi::db::DRow &r){set(r);}
  TblStInfoSysParam(const std::string &key,
	    const std::string &val)
  { set(key, val);}

  bool set(const std::string &key, const std::string &val);

  bool set(const dnmi::db::DRow&);
  bool set(const TblKeyVal &keyVal);

  TblStInfoSysParam& operator=(const TblKeyVal &keyVal){
                  if(&keyVal!=this)
		    set(keyVal);
		  return *this;
             }

  void clean();

  const char* tableName() const {return "param";}
  miutil::miString toSend()    const;
  miutil::miString toUpdate()  const;
  miutil::miString uniqueKey() const;

  int paramid() const { return paramid_ ; }
  std::string name() const { return name_; }
  int hlevel_scale() const { return hlevel_scale_;}
  int standard_hlevel() const { return standard_hlevel_; }
  int standard_physical_height()const { return standard_physical_height_;}
};

#endif
