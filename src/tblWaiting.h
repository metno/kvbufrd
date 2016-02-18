/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblWaiting.h,v 1.2.2.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbuffer_tblWaiting_h__
#define __kvbuffer_tblWaiting_h__

#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvDbBase.h"

class TblWaiting : public kvalobs::kvDbBase {
private:
  int              wmono_;
  int              id_;
  std::string      callsign_;
  std::string      code_;
  boost::posix_time::ptime obstime_;
  boost::posix_time::ptime delaytime_;

  void createSortIndex();

 public:
  TblWaiting() {clean();}
  TblWaiting(const TblWaiting &waiting){ set(waiting);}
  TblWaiting(const dnmi::db::DRow &r){set(r);}
  TblWaiting(int                  wmono,
             int                  id,
             const std::string    &callsign,
             const std::string    &code,
	     const boost::posix_time::ptime &obstime,
	     const boost::posix_time::ptime &delaytime)
  { set(wmono, id, callsign, code, obstime, delaytime);}

  bool set(int                  wmono,
           int id,
           const std::string    &callsign,
           const std::string    &code,
           const boost::posix_time::ptime &obtime,
           const boost::posix_time::ptime &delaytime);

  bool set(const dnmi::db::DRow&);
  bool set(const TblWaiting &waiting);

  void clean();

  const char*            tableName() const {return "waiting";}
  std::string toSend()    const;
  std::string toUpdate()  const;
  std::string uniqueKey() const;

  int              wmono()    const { return wmono_;    }
  int              id()       const { return id_; }
  std::string      callsign() const { return callsign_; }
  std::string      code()     const { return code_; }
  boost::posix_time::ptime obstime()  const { return obstime_;  }
  boost::posix_time::ptime delaytime()const { return delaytime_;}

  void delaytime(const boost::posix_time::ptime &t){ delaytime_=t;}
};

#endif
