/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvbufrCltApp.h,v 1.3.2.3 2007/09/27 09:02:23 paule Exp $

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

#include "KvParam.h"


KvParamList::
KvParamList()
{
}


KvParam::
KvParam( KvParamList &paramList, const char *name, int id )
   : value_( FLT_MAX )
{
   name_=name;
   id_ = id;
   paramList.params.push_back( this );
}

KvParam::
KvParam( KvParamList &paramList, const KvParam &param )
   : name_( param.name_ ), id_( param.id_ ), value_( param.value_ )
{
   paramList.params.push_back( this );
}

KvParam&
KvParam::
operator=( const KvParam &rhs )
{
   if( this != &rhs ) {
      name_ = rhs.name_;
      id_ = rhs.id_;
      value_ = rhs.value_;
   }
   return *this;
}

KvParam&
KvParam::
operator=( float rhs )
{
   value_=rhs;
}

/*
KvParam&
KvParam::
operator=( int rhs )
{
   value_=rhs;
}
*/

void
KvParam::
value( float val )
{
   value_ = val;
}

int
KvParam::
valAsInt()const
{
   return static_cast<int>( value_ + 0.5 );
}

