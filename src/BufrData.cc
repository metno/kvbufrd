/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: BufrData.h,v 1.8.6.6 2007/09/27 09:02:23 paule Exp $

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
#include "BufrData.h"

using namespace std;


BufrData::CloudExtra::
CloudExtra(): cloudData( 4,CloudDataExtra()  ), nElements_( 0 )
{}

BufrData::CloudExtra::
CloudExtra( const CloudExtra &ce )
   : cloudData( ce.cloudData ),nElements_( ce.nElements_ )
{
}


BufrData::CloudExtra&
BufrData::CloudExtra::
operator=( const CloudExtra &rhs )
{
   if( this != &rhs ) {
      cloudData = rhs.cloudData;
      nElements_ = rhs.nElements_;
   }
   return *this;
}

void
BufrData::CloudExtra::
add( const CloudDataExtra &cd, int index )
{
   cerr << "CloadExtra size: " << cloudData.size() << endl;
   if( index < 0 && nElements_ < cloudData.size() ) {
      cloudData[ nElements_ ] = cd;
      nElements_++;
   } else if( index < cloudData.size() && (index <= nElements_) ) {
      if( index == nElements_ )
         nElements_++;

      cloudData[ index ] = cd;
   } else
      throw std::range_error("BufrData::CloudExtra: out of bound [0,3].");
}

BufrData::CloudDataExtra&
BufrData::CloudExtra::
operator[]( int index )const
{
   if( index < cloudData.size() ) {
      return const_cast<CloudExtra*>(this)->cloudData[index];
   } else {
      throw std::range_error("BufrData::CloudExtra: out of bound [0,3].");
   }
}


BufrData::
BufrData():
   tWeatherPeriod( FLT_MAX ), vsci( INT_MAX ),
   tTAX_N( INT_MAX ), tTAN_N( INT_MAX ),
   TAX_N( FLT_MAX ), TAN_N( FLT_MAX )
{
}

BufrData::
BufrData( const DataElement &de )
   :   DataElement( de ),
       tWeatherPeriod( FLT_MAX ), vsci( INT_MAX ),
       tTAX_N( INT_MAX ), tTAN_N( INT_MAX ),
       TAX_N( FLT_MAX ), TAN_N( FLT_MAX )
{
}


BufrData::
BufrData( const BufrData &bd ):
   DataElement( static_cast<DataElement>( bd ) ),
   tWeatherPeriod( bd.tWeatherPeriod ),
   vsci( bd.vsci ),
   tTAX_N( bd.tTAX_N ), tTAN_N( bd.tTAN_N ),
   TAX_N( bd.TAX_N ), TAN_N( bd.TAN_N ),
   FgMax( bd.FgMax ),
   FxMax( bd.FxMax ),
   precip24( bd.precip24 ),
   precipRegional( bd.precipRegional ),
   precipNational( bd.precipNational ),
   cloudExtra( bd.cloudExtra )
{
}

void BufrData::crcHelper(std::ostream &o)const {
   using std::endl;
   DataElement::crcHelper(o);

   if( tWeatherPeriod !=  FLT_MAX ) 
      o << "tWeatherPeriod: " << tWeatherPeriod << endl;

   if(vsci!=INT_MAX)
      o << "vsci: " << vsci << endl;

   if(  tTAX_N != INT_MAX )
      o << "tTAX_N: " << tTAX_N << endl;

   if(    tTAN_N!= INT_MAX)
      o << "tTAN_N: " << tTAN_N << endl;

   if(TAX_N!= FLT_MAX )
      o << "TAX_N: "  << TAX_N << endl;
   
   if(TAN_N!= FLT_MAX )
      o << "TAN_N: " << TAN_N << endl;

   FgMax.crcHelper( o, "FgMax");
   FxMax.crcHelper( o, "FxMax");
   precip24.crcHelper( o, "precip24");
   precipRegional.crcHelper(o, "precipRegional");
   precipNational.crcHelper(o, "precipNational");
   cloudExtra.crcHelper(o, "cloudExtra");

}


BufrData&
BufrData::
operator=( const BufrData &rhs )
{

   if( this != &rhs ) {
      
      *static_cast<DataElement*>(this) = static_cast<DataElement&>( const_cast<BufrData&>(rhs) );
      tWeatherPeriod = rhs.tWeatherPeriod;
      vsci = rhs.vsci;
      tTAX_N = rhs.tTAX_N;
      tTAN_N = rhs.tTAN_N;
      TAX_N = rhs.TAX_N;
      TAN_N = rhs.TAN_N;
      FgMax = rhs.FgMax;
      FxMax = rhs.FxMax;
      precip24 = rhs.precip24;
      precipRegional = rhs.precipRegional;
      precipNational = rhs.precipNational;
      cloudExtra = rhs.cloudExtra;
   }

   return *this;
}



std::ostream
&operator<<(std::ostream &o, const BufrData::CloudDataExtra &cd )
{
   o << "vsci: ";

   if( cd.vsci == FLT_MAX )
      o << "NA";
   else
      o << cd.vsci;

   o << " Ns: ";
   if( cd.Ns == FLT_MAX )
      o << "NA";
   else
      o << cd.Ns;


   o << " C: ";
   if( cd.C == FLT_MAX )
      o << "NA";
   else
      o << cd.C;

   o << " hshs: ";
   if( cd.hshs == FLT_MAX )
      o << "NA";
   else
      o << cd.hshs;

   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrData::Wind &wind )
{
   o << "ff: ";
   if( wind.ff == FLT_MAX )
      o << "NA";
   else
      o << wind.ff;

   o << " dd: ";

   if( wind.dd == FLT_MAX )
      o << "NA";
   else
      o << wind.dd;

   o << " t: ";
   if( wind.t == FLT_MAX )
      o << "NA";
   else
      o << wind.t;

   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrData::Precip &precip )
{
   o << "hTr: ";
   if( precip.hTr == FLT_MAX )
      o << "NA";
   else
      o << precip.hTr;

   o << " RR: ";

   if( precip.RR == FLT_MAX )
      o << "NA";
   else
      o << precip.RR;

   return o;
}


