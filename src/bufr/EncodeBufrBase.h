/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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

#ifndef __ENCODEBUFRBASE_H__
#define __ENCODEBUFRBASE_H__

#include <list>
#include "boost/date_time/posix_time/ptime.hpp"
#include "BufrHelper.h"
#include "../BufrData.h"
#include "../StationInfo.h"
#include "EncodeBufrManager.h"


class EncodeBufrBase
{
protected:
   bool supportReplication;
   int iValueOld;
   int templateid;
   bool validValues;
   int  nValidValues;
   StationInfoPtr stationInfo;
   BufrDataPtr data;
   BufrHelper *bufr;
   EncodeBufrManager *encodeMgr;
   boost::posix_time::ptime obstime;
   int replicator;

   /**
    *
    * @param templateid
    * @throw IdException on failure.
    */
   void encodeTemplate( int templateid, int replicator = -1 );

   bool isDelayedReplicator();
   bool hasReplicator();

   /**
    * @throw IdException on failure.
    */
   virtual void encode( ) = 0;

public:

   EncodeBufrBase();
   virtual ~EncodeBufrBase();

   void setHasValidValue();

   bool emptyBufr()const;
   int  nValues()const;

   virtual std::string logIdentifier() const = 0;
   virtual std::list<int> encodeIds()const = 0;

   static float encodeRR( float rr );

   /**
    *
    * @param bufrParamId
    * @param bufr
    * @param mgr
    * @throw IdException on failure.
    */
   void encode( int bufrParamId,
                BufrHelper &bufr,
                EncodeBufrManager &mgr,
                int replicator );

};

#endif /* ENCODEBUFRBASE_H_ */
