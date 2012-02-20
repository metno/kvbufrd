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
#include "BUFRparam.h"

using namespace std;

std::ostream&
BufrParamType::
print( std::ostream &o ) const
{
   o << "( t: ";
   switch( type ) {
      case INT: o << "INT"; break;
      case FLOAT: o << "FLOAT"; break;
      case CODE: o << "CODE"; break;
      case STRING: o << "STRING"; break;
      default: o << type; break;
   }

   o << " w: " << width << " s: " << scale << " r: " << reference << " )";
   return o;
}

BufrParamInt::
BufrParamInt( int width, int scale, int reference )
   : BufrParamType( INT, width, scale, reference )
{
}

bool
BufrParamInt::
valid( int value )const
{
   return true;
}

BufrParamFloat::
BufrParamFloat( int width, int scale, int reference )
   : BufrParamType( FLOAT, width, scale, reference )
{
}

bool
BufrParamFloat::
valid( double value )const
{
   return true;
}

BufrParamCode::
BufrParamCode( int width )
   : BufrParamType( CODE, width )
{
}

bool
BufrParamCode::
valid( int value )const
{
   return true;
}

BufrParamString::
BufrParamString( int width )
   : BufrParamType( STRING, width )
{
}

bool
BufrParamString::
BufrParamString::
valid( const std::string &value )const
{
   return true;
}

BufrParamFlag::
BufrParamFlag( int width )
   : BufrParamType( INT, width )
{
}


bool
BufrParamFlag::
valid( int value )const
{
   return true;
}

BufrValidater::
BufrValidater()
{
}

bool
BufrValidater::
loadTable( const std::string &filepath )
{
   string filename;
   string::size_type i = filepath.find_last_of('/');

   if( i == string::npos )
      filename = filepath;
   else
      filename = filepath.substr( i+1 );

   return true;
   //string filename="/home/borgem/projects/workspace/kvbufrd/share/bufrtables/B0000000000098014001.TXT";
}


std::ostream&
operator<<(std::ostream &o, const BufrParamType &pt)
{
   return pt.print( o );
}

std::ostream&
operator<<(std::ostream &o, const BufrParamInt &pi)
{
   o << pi;
   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrParamFloat &pf)
{
   o << pf;
   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrParamCode &pc)
{
   o << pc;
   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrParamString &ps)
{
   o << ps;
   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrParamFlag &pfl)
{
   o << pfl;
   return o;
}

