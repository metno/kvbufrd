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

#include <iostream>
#include <sstream>
#include "EncodeBufrBase.h"

using namespace std;


EncodeBufrBase::
EncodeBufrBase():
    supportReplication(false),
    iValueOld(0),
    templateid(0),
    validValues( false ),
    nValidValues( 0 ),
    bufr(nullptr),
    encodeMgr(nullptr),
    replicator(0)
{
}

EncodeBufrBase::
~EncodeBufrBase()
{
}


void
EncodeBufrBase::
encodeTemplate( int templateid_, int replicator )
{
   return encodeMgr->encode( templateid_, *bufr, replicator );
}

bool
EncodeBufrBase::
hasReplicator()
{
   supportReplication=true;
   return replicator >= 0;
}

bool
EncodeBufrBase::
isDelayedReplicator()
{
   supportReplication=true;
   return replicator>=31000 && replicator <= 31002;
}

void
EncodeBufrBase::
setHasValidValue()
{
    ++nValidValues;
   validValues = true;
}


float
EncodeBufrBase::
encodeRR( float rr )
{
    if( rr == FLT_MAX || rr < -1.01 )
        return FLT_MAX;

    if( rr >=0 && rr < 0.05 )
        rr = -0.1;
    else if( rr < 0 ) {
        if( rr < -0.99 && rr > -1.01 )
            rr=0;
        else
            rr = FLT_MAX;
    }

    return rr;
}


bool
EncodeBufrBase::
emptyBufr()const
{
    return ! validValues;
}

int
EncodeBufrBase::
nValues()const
{
    return nValidValues;
}

void
EncodeBufrBase::
encode( int bufrParamId,
        BufrHelper &bufr_,
        EncodeBufrManager &mgr,
        int replicator_ )
{

   bufr = &bufr_;
   iValueOld = bufr->iValue;
   supportReplication = false;
   templateid = bufrParamId;
   stationInfo = bufr->getStationInfo();
   data = bufr->getData();
   obstime = data->time();
   encodeMgr = &mgr;
   replicator = replicator_;
   validValues = false;
   nValidValues = 0;

   if( bufr->bufrBase ) {
//       cerr << "Incomming: (cur/prev) (" << templateid << "/"<< bufr->bufrBase->templateid << ") validValues: " << (bufr->bufrBase->validValues?"T":"F") << " nValidValues: " << bufr->bufrBase->nValidValues << endl;
       validValues = bufr->bufrBase->validValues;
       nValidValues = bufr->bufrBase->nValidValues;
   } else {
//       cerr << "Incomming: (cur/prev) (" << templateid << "/(NULL)) validValues: " << (validValues?"T":"F") << " nValidValues: " << nValidValues << endl;
   }

   bufr->bufrBase = this;
   if( replicator > 31002 ) {
      ostringstream o;
      o << "Replicator <" << replicator << "> not implemented.";
      throw NotImplementedException( o.str() );
   }

   encode();

   if( replicator_ >= 0 && ! supportReplication ) {
      ostringstream o;
      o << "Replication not implemented for template/paramid <"
        << bufrParamId << ">." ;
      throw NotImplementedException( o.str() );
   }
}

