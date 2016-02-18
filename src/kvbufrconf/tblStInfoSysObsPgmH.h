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
#ifndef __tblStInfoSysObsPgmH_h__
#define __tblStInfoSysObsPgmH_h__

#include <bitset>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvDbBase.h"

namespace pt=boost::posix_time;

class TblStInfoSysObsPgmH : public kvalobs::kvDbBase {
   int stationid_;
   int paramid_;
   int hlevel_;
   int message_formatid_;
   int nsensor_;
   bool priority_message_;
   bool anytime_;
   std::bitset<24> hour_;
   bool test_;
   pt::ptime totime_;
   pt::ptime fromtime_;


  void createSortIndex();

public:
  TblStInfoSysObsPgmH() {clean();}
  TblStInfoSysObsPgmH(const TblStInfoSysObsPgmH &obspgm){ set(obspgm);}
  TblStInfoSysObsPgmH(const dnmi::db::DRow &r){set(r);}

  bool set(const dnmi::db::DRow&);
  void set(const TblStInfoSysObsPgmH &obspgm );

  TblStInfoSysObsPgmH& operator=(const TblStInfoSysObsPgmH &rhs ){
                  if( &rhs != this)
                     set(rhs);
                  return *this;
                }

  void clean();

  const char* tableName() const {return "obspgm_h";}

  std::string toSend()    const { return ""; } //NOT used
  std::string toUpdate()  const{ return ""; }  //NOT used
  std::string uniqueKey() const;

  int stationid()const { return  stationid_; }
  int paramid()const { return paramid_; }
  int hlevel()const { return hlevel_; }
  int messageFormatid()const { return message_formatid_;}
  int nsensor() const { return nsensor_; }
  bool priorityMessage()const { return priority_message_; }
  bool anytime() const { return anytime_; }
  std::bitset<24> hour()const { return hour_; }
  bool test()const { return test_; }
  pt::ptime totime() const { return totime_; }
  pt::ptime fromtime() const { return fromtime_; }

  friend std::ostream& operator<<(std::ostream &o, TblStInfoSysObsPgmH &op );
};

std::ostream& operator<<(std::ostream &o, TblStInfoSysObsPgmH &op );
#endif
