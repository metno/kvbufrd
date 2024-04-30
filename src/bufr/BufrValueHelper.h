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

#endif
