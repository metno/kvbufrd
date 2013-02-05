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

#ifndef __BUFRVALUEHELPER_H__
#define __BUFRVALUEHELPER_H__

#include <sstream>
#include <stdexcept>
#include "bufrexceptions.h"
#include "BUFRparam.h"

class BufrHelper;

class Values
{
   const double miss;
   double *values_;
   int size_;
   std::ostringstream log_;
   BufrParamValidaterPtr validater_;
   friend class BufrHelper;

public:
   class Proxy {
      Values *val;
      int index;
      BufrParamValidaterPtr validater_;
   public:
      Proxy( const Values *val_, int index,
             BufrParamValidaterPtr validater );
      Proxy& operator=( const double &rhs );

      operator double();

      void toBufr( const std::string &id, int   value, bool mustHaveValidValue=false );
      void toBufr( const std::string &id, float value, bool mustHaveValidValue=false );
      void toBufrIf( const std::string &id, float value, bool valid, bool mustHaveValidValue=false );
      bool insert( int bufrParamId, int value, const std::string &name="" );
      bool insert( int bufrParamId, float value, const std::string &name="" );
      bool insert( int bufrParamId, std::string &value, int index, const std::string &name="" );
   };

   Values();

   Values( int i, BufrParamValidaterPtr validater );

   Values( int i );

   ~Values();

   const Proxy operator[]( int index )const;
   Proxy operator[]( int index );

   int size()const;
   double* values();
   std::string log()const;
};

#if 0
class Kvalues
{
   const int miss;
   int *values_;
   int size_;
   std::ostringstream log_;

public:
   class Proxy {
      Kvalues *val;
      int index;
   public:
      Proxy( const Kvalues *val_, int index ):val( const_cast<Kvalues*>( val_ ) ), index( index ) {}
      Proxy& operator=( const int &rhs ){
         val->values_[index] = rhs;
         val->log_ <<  index << " : (noid) : " << rhs << std::endl;
         return *this;
      }

      operator int() {
         return val->values_[index];
      }

      void toBufr( const std::string &id, int   value, bool mustHaveValidValue=false ){
         int ret=val->miss;

         val->log_ << index << " : " << id << " : ";
         if( value == INT_MIN || value ==INT_MAX ) {
            val->log_ << "NA";
            if( mustHaveValidValue ) {
               val->log_ << ". Exception: Mandatory value." << std::endl;
               throw EncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
            }
         } else {
            ret =  value;
            val->log_ << ret;
         }

         val->log_ << std::endl;
         val->values_[index] = ret;
      }

      void toBufr( const std::string &id, float value, bool mustHaveValidValue=false ){
         int ret=val->miss;

         val->log_ << index << " : " << id << " : ";
         if( value == FLT_MIN || value == FLT_MAX ) {
            val->log_ << "NA";
            if( mustHaveValidValue ) {
               val->log_ << ". Exception: Mandatory value." << std::endl;
               throw EncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
            }
         } else {
            ret =  static_cast<int>( value );
            val->log_ << ret;
         }

         val->log_ << std::endl;

         val->values_[index] = ret;
      }
   };

   Kvalues():miss( INT_MAX ), values_(0), size_( 0 ){};

   Kvalues( int i ):miss( INT_MAX ), size_( i )  {
      try{
         values_ = new int[size_];
         for( int i=0; i<size_; ++i )
            values_[i] = miss;
      }
      catch( ... ) {
         values_ = 0;
      }
   }

   ~Kvalues() {
      if( values_)
         delete[] values_;
   }

   const Proxy operator[]( int index )const
   {
      if( index > size_-1 )
         throw std::range_error( "Values: Index out of ranges.");

      return Proxy ( this, index  );
   }

   Proxy operator[]( int index )
   {
      if( index > size_-1 )
         throw std::range_error( "Values: Index out of ranges.");
      return Proxy ( this, index  );
   }

   int size()const { return size_; }
   int *values(){ return values_;}

   std::string log()const { return log_.str(); }

};
#endif


#endif
