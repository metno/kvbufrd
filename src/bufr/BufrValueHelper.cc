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
#include <iomanip>
#include "bufrdefs.h"
#include "BufrValueHelper.h"
#include "BufrHelper.h"

Values::Proxy::
Proxy( const Values *val_, int index,
       BufrParamValidaterPtr validater )
   :val( const_cast<Values*>( val_ ) ), index( index ),
    validater_( validater )
{
}

Values::Proxy&
Values::Proxy::
operator=( const double &rhs ){
   val->values_[index] = rhs;
   val->log_ <<  index << " : (noid) : " << rhs << std::endl;
   return *this;
}

Values::Proxy::
operator double() {
   return val->values_[index];
}

void
Values::Proxy::
toBufr( const std::string &id, int   value, bool mustHaveValidValue )
{
   double ret=val->miss;

   val->log_ << index << " : " << id << " : ";
   if( value == INT_MIN || value == INT_MAX ) {
      val->log_ << "NA";
      if( mustHaveValidValue ) {
         val->log_ << ". Exception: Mandatory value." << std::endl;
         throw EncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
      }
   } else {
      ret = static_cast<double>( value );
      val->log_ << ret;
   }

   val->log_ << std::endl;

   val->values_[index] = ret;
}

void
Values::Proxy::
toBufr( const std::string &id, float value, bool mustHaveValidValue )
{
   double ret=val->miss;

   val->log_ << index << " : " << id << " : ";
   if( value == FLT_MIN || value == FLT_MAX ) {
      val->log_ << "NA";
      if( mustHaveValidValue ) {
         val->log_ << ". Exception: Mandatory value." << std::endl;
         throw EncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
      }
   } else {
      ret = static_cast<double>( value );
      val->log_ << ret;
   }

   val->log_ << std::endl;

   val->values_[index] = ret;
}

void
Values::Proxy::
toBufrIf( const std::string &id, float value, bool valid, bool mustHaveValidValue )
{
   double ret=val->miss;

   val->log_ << index << " : " << id << " : ";
   if( value == FLT_MIN || value == FLT_MAX || !valid  ) {
      val->log_ << "NA";
      if( mustHaveValidValue ) {
         val->log_ << ". Exception: Mandatory value." << std::endl;
         throw EncodeException( "Bufr: Missing mandatory value for <" + std::string(id) + ">." );
      }
   } else {
      ret = static_cast<double>( value );
      val->log_ << ret;
   }

   val->log_ << std::endl;

   val->values_[index] = ret;
}

bool
Values::Proxy::
insert( int bufrParamId, int value, const std::string &name )
{
   bool isValid=true;
   double ret=val->miss;


   if( value == INT_MIN || value == INT_MAX
         || ( validater_ && ! validater_->isValid( bufrParamId, value )) )
      isValid = false;
   else
      ret = static_cast<double>( value );

   val->log_ << std::setfill(' ')<< std::setw(3) << index << " : "
             << std::setfill('0')<< std::setw(6)<< bufrParamId << " : ";
   if( isValid )
      val->log_ << std::setfill(' ') << std::fixed << std::setprecision(2) << std::setw(8) << ret;
   else
      val->log_ << std::setfill(' ') << std::setw(8) << "NA";

   if( ! name.empty() )
      val->log_ << "  " << name;

   val->log_ << std::endl;

   val->values_[index] = ret;
   return isValid;
}

bool
Values::Proxy::
insert( int bufrParamId, float value, const std::string &name )
{
   bool isValid=true;
   double ret=val->miss;

   if( value == FLT_MIN || value == FLT_MAX
         || ( validater_ && ! validater_->isValid( bufrParamId, value ))) {
      isValid=false;
   } else {
      ret = validater_->roundToPrec( bufrParamId, value );
   }

   val->log_ << std::setfill(' ')<< std::setw(3) << index << " : "
          << std::setfill('0')<< std::setw(6)<< bufrParamId << " : ";

   if( isValid )
      val->log_ << std::setfill(' ') << std::fixed << std::setprecision(2) << std::setw(8) << ret;
   else
      val->log_ << std::setfill(' ') << std::setw(8) << "NA";

   if( ! name.empty() )
      val->log_ << "  " << name;

   val->log_ << std::endl;
   val->values_[index] = ret;
   return isValid;
}

bool
Values::Proxy::
insert( int bufrParamId, std::string &value, int sIndex, const std::string &name )
{
   bool isValid=true;
   double ret= val->miss;


   if( validater_ && ! validater_->isValid( bufrParamId, value )) {
      val->log_ << "NA";
      isValid = false;
      value.erase();
   }

   ret = static_cast<double>( sIndex*1000 + value.length() );

   val->log_ << std::setfill(' ')<< std::setw(3) << index << " : "
             << std::setfill('0')<< std::setw(6)<< bufrParamId << " : ";

   val->log_ << std::fixed << std::setprecision(0) << std::setw(8) << ret
         << "  '" << value << "'";

   if( ! name.empty() )
      val->log_ << "  " << name ;

   val->log_ << std::endl;
   val->values_[index] = ret;
   return isValid;
}


Values::
Values()
   :miss( RVIND ), values_(0), size_( 0 )
{
}

Values::
Values( int i, BufrParamValidaterPtr validater )
: miss( RVIND ), size_( i ), validater_( validater)
{
   try{
      values_ = new double[size_];
      for( int i=0; i<size_; ++i )
         values_[i] = miss;
   }
   catch( ... ) {
      values_=0;
   }
}

Values::
Values( int i )
   : miss( RVIND ), size_( i )
{
   try{
      values_ = new double[size_];
      for( int i=0; i<size_; ++i )
         values_[i] = miss;
   }
   catch( ... ) {
      values_=0;
   }
}

Values::
~Values()
{
   if( values_ )
      delete[] values_;
}

const
Values::Proxy
Values::
operator[]( int index )const
{
   if( index > size_-1 )
      throw std::range_error( "Values: Index out of ranges.");

   return Proxy ( this, index, validater_  );
}

Values::Proxy
Values::
operator[]( int index )
{
   if( index > size_-1 )
      throw std::range_error( "Values: Index out of ranges.");
   return Proxy ( this, index, validater_  );
}

int
Values::
size()const
{
   return size_;
}

double*
Values::
values()
{
   return values_;
}

std::string
Values::
log()const
{
   return log_.str();
}

