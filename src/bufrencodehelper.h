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

#ifndef __BUFRENCODE_HELPER__
#define __BUFRENCODE_HELPER__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "encodebufr.h"

#define KELEM 4000    /* Max number of elements in values array. */
#define KVALS 20000   /* Max number of elements in CVALS array.
                       * Note that 200000 (as used in example Fortran programs
                       * from ECMWF) is too big; results in segmentation
                       * violation for cvals */
#define KDLEN 200     /* Max number of elements in kdata array */
#define MAX_BUFLEN 200000 /* Max lenght (in bytes) of bufr message */
#define RVIND 1.7E38  /* 'Missing' value */


#if defined(__cplusplus)
extern "C" {
#endif

extern void pbopen_(int *fd, char *filename, char *fmod, int *errorIndicator, int lengthOfFilename, int unknown /*set to 2*/);
extern void pbclose_(int *fd, int *errorIndicator );
extern void pbwrite_(int *fd, int *kbuff, int *nbytes, int *errorIndicator );
extern void bufren_(int *ksec0, int *ksec1, int *ksec2, int *ksec3, int *ksec4,
                    int *ktdlen, int *ktdlst, int *kdlen, int *kdata,
                    int *kelem, int *kvals, double *values, char **cvals,
                    int *kbufl, int *kbuff, int *errorIndicator);

#if defined(__cplusplus)
}
#endif

class Values
{
   const double miss;
   double *values_;
   int size_;
   std::ostringstream log_;

public:
   class Proxy {
      Values *val;
      int index;
   public:
      Proxy( const Values *val_, int index ):val( const_cast<Values*>( val_ ) ), index( index ) {}
      Proxy& operator=( const double &rhs ){
         val->values_[index] = rhs;
         val->log_ <<  index << " : (noid) : " << rhs << std::endl;
         return *this;
      }

      operator double() {
         return val->values_[index];
      }

      void toBufr( const std::string &id, int   value, bool mustHaveValidValue=false ){
         double ret=val->miss;

         val->log_ << index << " : " << id << " : ";
           if( value == INT_MIN || value ==INT_MAX ) {
              val->log_ << "NA";
              if( mustHaveValidValue ) {
                 val->log_ << ". Exception: Mandatory value." << std::endl;
                 throw BufrEncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
              }
           } else {
              ret = static_cast<double>( value );
              val->log_ << ret;
           }

           val->log_ << std::endl;

           val->values_[index] = ret;
      }

      void toBufr( const std::string &id, float val ){

      }
   };

   Values():miss( RVIND ), values_(0), size_( 0 ){};

   Values( int i ):miss( RVIND ), size_( i )  {
      values_ = new double[size_];
      for( int i=0; i<size_; ++i )
         values_[i] = miss;
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
   double* values(){ return values_;}

   std::string log()const { return log_.str(); }

};

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
                 throw BufrEncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
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
           if( value == FLT_MIN || value ==FLT_MAX ) {
              val->log_ << "NA";
              if( mustHaveValidValue ) {
                 val->log_ << ". Exception: Mandatory value." << std::endl;
                 throw BufrEncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
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
      values_ = new int[size_];
      for( int i=0; i<size_; ++i )
         values_[i] = miss;
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
