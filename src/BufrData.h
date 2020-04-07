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

#ifndef __BUFRDATA_H__
#define __BUFRDATA_H__

#include <stdexcept>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "DataElementList.h"
#include <iostream>
#include <sstream>

/**
 * BufrData add some special elements to use in coding of buffers.
 */
class BufrData : public DataElement
{
public:

   struct Wind {
      float ff;
      float dd;
      float t;

      Wind():
         ff( FLT_MAX ), dd( FLT_MAX ), t( FLT_MAX ) {}
      Wind& operator=( const Wind &rhs )
      {
         if( this != &rhs ) {
            ff = rhs.ff;
            dd = rhs.dd;
            t  = rhs.t;
         }
         return *this;
      }
      void crcHelper(std::ostream &o, const std::string &which )const {
         using std::endl;
         if( ff != FLT_MAX ) {
            o << which << "::ff: " << ff << endl;
         }
         if( dd != FLT_MAX ) {
            o << which << "::dd: " << dd << endl;
         }

         if( t != FLT_MAX ) {
            o << which << "::t: " << t << endl;
         }
      }
      friend std::ostream &operator<<(std::ostream &o, const Wind &wind );
   };

   struct Precip {
      float hTr;
      float RR;

      Precip():
         hTr( FLT_MAX ), RR( FLT_MAX ) {}
      Precip& operator=( const Precip &rhs )
      {
         if( this != &rhs ) {
            hTr = rhs.hTr;
            RR = rhs.RR;
         }
         return *this;
      }

      void crcHelper(std::ostream &o, const std::string &which )const {
         using std::endl;
         if( RR != FLT_MAX ) {
            o << which << "::RR: " << RR << endl;
         }
         if( hTr != FLT_MAX ) {
            o << which << "::hTr: " <<  hTr << endl;
         }

      }
      bool valid()const {
         return RR!=FLT_MAX && hTr != FLT_MAX;
      }

      friend std::ostream &operator<<(std::ostream &o, const Precip &precip );
   };

   struct CloudDataExtra {
      int vsci;
      int Ns;
      int C;
      float hshs;

      CloudDataExtra():
         vsci( INT_MAX ), Ns( INT_MAX ), C( INT_MAX ), hshs( FLT_MAX )
      {}

      CloudDataExtra( int vsci_, int ns, int c, int hs )
         : vsci( vsci_ ), Ns( ns ), C( c ), hshs( hs ){}

      CloudDataExtra& operator=( const CloudDataExtra &rhs )
      {
         if( this != &rhs ) {
            vsci = rhs.vsci;
            Ns = rhs.Ns;
            C = rhs.C;
            hshs = rhs.hshs;
         }
         return *this;
      }

      bool valid() const {
         return vsci != INT_MAX || Ns != INT_MAX || C != INT_MAX || hshs!=FLT_MAX;
      }
      
      void crcHelper(std::ostream &o, const std::string &which )const {
         using std::endl;
         if( vsci != INT_MAX ) {
            o << which << "::vsci: " << vsci<< endl;
         }
         if( Ns != INT_MAX ) {
            o << which << "::Ns: " <<  Ns << endl;
         }
         if( C != INT_MAX ) {
            o << which << "::C: " <<  C << endl;
         }
         if( hshs != FLT_MAX ) {
            o << which << "::hshs: " <<  hshs << endl;
         }

      }

      friend std::ostream &operator<<(std::ostream &o, const CloudDataExtra &cd );
   };

   class CloudExtra {
      std::vector<CloudDataExtra> cloudData;
      int nElements_;

   public:
      CloudExtra();
      CloudExtra( const CloudExtra &ce );
      CloudExtra& operator=( const CloudExtra &rhs );

      int size()const { return nElements_; }

      /**
       *
       * @param cd
       * @param index
       * @throw std::range_error
       */
      void add( const CloudDataExtra &cd, int index=-1 );

      void crcHelper(std::ostream &o, const std::string &which )const {
         using std::endl;
         std::ostringstream s;
         for( int i=0; i<nElements_; ++i) {
            s.str("");
            s << which <<"[" << i << "]";
            cloudData[i].crcHelper(o, s.str());
         }   
      }


      /**
       *
       * @param index
       * @return
       * @throw std::range_error
       */
      CloudDataExtra& operator[]( int index )const;
   };

   float tWeatherPeriod; //Time since last weather observation (clouds etc.).
   int vsci; //Vertical significance of clouds for CL, CM and CH.
   int tTAX_N; //Periode for max temperature.
   int tTAN_N; //Periode for max temperature.
   float TAX_N;
   float TAN_N;
   Wind  FgMax;
   Wind  FxMax;
   Precip precip24;
   Precip precipRegional;
   Precip precipNational;
   CloudExtra cloudExtra;

   BufrData();
   BufrData( const BufrData &bd );
   BufrData( const DataElement &de );

   virtual void crcHelper(std::ostream &o)const override;

   BufrData& operator=( const BufrData &rhs );
   
};

typedef boost::shared_ptr<BufrData> BufrDataPtr;

std::ostream &operator<<(std::ostream &o, const BufrData::CloudDataExtra &cd );
std::ostream &operator<<(std::ostream &o, const BufrData::Wind &wind );
std::ostream &operator<<(std::ostream &o, const BufrData::Precip &precip );


#endif /* BUFRDATA_H_ */
