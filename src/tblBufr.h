/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblSynop.h,v 1.2.6.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvbufr_tblBufr_h__
#define __kvbufr_tblBufr_h__

#include <string>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvDbBase.h"

class TblBufr : public kvalobs::kvDbBase {
private:
   int              wmono_;
   int              id_;
   std::string      callsign_;
   std::string      code_;
   boost::posix_time::ptime obstime_;
   boost::posix_time::ptime createtime_;
   int              crc_;
   int              ccx_;
   std::string      data_;
   std::string      bufrBase64_;
   mutable boost::posix_time::ptime tbtime_;

   void createSortIndex();

public:
   TblBufr() {clean();}
   TblBufr(const TblBufr &bufr){ set(bufr);}
   TblBufr(const dnmi::db::DRow &r){set(r);}
   TblBufr(int                  wmono,
           int                  id,
           const std::string    &callsign,
           const std::string    &code,
           const boost::posix_time::ptime &obtime,
           const boost::posix_time::ptime &createtime,
           int                  crc,
           int                  ccx,
           const std::string    &data,
           const std::string    &bufrBase64,
           const boost::posix_time::ptime &tbtime=boost::posix_time::second_clock::universal_time() )
   { set( wmono, id, callsign, code, obtime, createtime, crc, ccx, data, bufrBase64, tbtime );}

   bool set(int                  wmono,
            int                  id,
            const std::string    &callsign,
            const std::string    &code,
            const boost::posix_time::ptime &obtime,
            const boost::posix_time::ptime &createtime,
            int                  crc,
            int                  ccx,
            const std::string    &data,
            const std::string    &bufrBase64,
            const boost::posix_time::ptime &tbtime=boost::posix_time::second_clock::universal_time()
   );

   bool set(const dnmi::db::DRow&);
   bool set(const TblBufr &bufr );

   TblBufr& operator=(const TblBufr &ts){
      if(&ts!=this)
         set(ts);
      return *this;
   }

   void clean();

   const char* tableName()            const {return "bufr";}

   std::string toSend()    const;
   std::string toUpdate()  const;
   std::string uniqueKey() const;

   int              wmono()      const { return wmono_; }
   int              id()         const { return id_; }
   std::string      callsign()   const { return callsign_; }
   std::string      code()       const { return code_; }
   boost::posix_time::ptime obstime()    const { return obstime_; }
   boost::posix_time::ptime createtime() const { return createtime_;}
   int              crc()        const { return crc_; }
   int              ccx()        const { return ccx_; }
   std::string      data()       const { return data_; }
   std::string      bufrBase64() const { return bufrBase64_; }
   boost::posix_time::ptime tbtime() const { return tbtime_;}

   void createtime(const boost::posix_time::ptime &t){ createtime_=t;}
   void crc(int c)                         { crc_=c; }
   void incCcx()                           { ccx_++; }
   void ccx(int c)                         { ccx_=c; }
   void data(const std::string &s)         { data_=s;}
   void bufrBase64(const std::string &s)   { bufrBase64_=s; }
};

#endif
