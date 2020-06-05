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
#include <math.h>
#include <vector>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "fixstring.h"
#include "BUFRparam.h"
#include "../scanstring.h"

namespace fs=boost::filesystem;
namespace b=boost;
using namespace std;

namespace {

string
trim( const string &v )
{
   string s(v);
   b::trim( s );
   return s;
}




BufrParamType::Type
findParamType( const string &unit )
{
   string::size_type i;

   if( (i=unit.find("TABLE")) != string::npos )
      return BufrParamType::CODE;
   else if( (i=unit.find("NUMERIC")) != string::npos )
      return BufrParamType::INT;
   else if( (i=unit.find("CCITTIA5")) != string::npos )
      return BufrParamType::STRING;
   else
      return BufrParamType::FLOAT;
}

BufrParamType*
mkBufrParamType( int nElems, const ScanStringSpecList &elems )
{
   int id;
   string name;
   string unit;
   int scale;
   int refVal;
   int with;

   try {
      id = b::lexical_cast<int>( trim( elems[0].value ) );
      name = trim( elems[1].value );
      unit = trim( elems[2].value );
      scale = b::lexical_cast<int>( trim( elems[3].value ) );
      refVal = b::lexical_cast<int>( trim( elems[4].value ) );
      with = b::lexical_cast<int>( trim( elems[5].value ) );
   }
   catch( const b::bad_lexical_cast &ex ) {
      cerr << " --- EXCETION: " << ex.what() << endl;
      return 0;
   }
   BufrParamType *param;
   switch( findParamType( unit )  ) {
      case BufrParamType::INT:
         if( scale == 0 )
            param = new BufrParamInt( id, with, scale, refVal );
         else
            param = new BufrParamFloat( id, with, scale, refVal );
         break;
      case BufrParamType::FLOAT:
         param = new BufrParamFloat( id, with, scale, refVal );
         break;
      case BufrParamType::CODE:
         param = new BufrParamCode( id, with );
         break;
      case BufrParamType::STRING:
         param = new BufrParamString( id, with );
         break;
      default:
         return 0;
   }

   param->name = name;
   param->unit = unit;
   return param;
}


}

std::string
BufrParamType::
expecting()const
{
   switch( type ){
      case INT: return "Expecting <integer>.";
      case FLOAT: return "Expecting <double>.";
      case STRING: return "Expecting <string>.";
      case CODE: return "Expecting <integer> (CODE/FLAG table).";
   }
   return "Expecting <UNKNOW>.";
}
std::string
BufrParamType::
typeToString()const
{
   switch( type ){
      case INT: return "<integer>";
      case FLOAT: return "<double>";
      case STRING: return "<string>";
      case CODE: return "<CODE/FLAG>";
   }
   return "<UNKNOW>";
}

double
BufrParamType::
roundToPrec( float value )const
{
   throw NotImplementedException( string("Rounding not implemented for ") + typeToString()+"." );
}

bool
BufrParamType::
valid( int value ) const
{
   throw TypeException( id, "Got <int>. " + expecting() );
}

bool
BufrParamType::
valid( float value ) const
{
   throw TypeException( id, "Got <float>. " + expecting() );
}


bool
BufrParamType::
valid( std::string& value ) const
{
   throw TypeException( id, "Got <string>. " + expecting() );
}

std::ostream&
BufrParamType::
print( std::ostream &o ) const
{
   o << "( " << setfill('0')<<setw(6) << id
     << " t: ";
   switch( type ) {
      case INT: o << "INT"; break;
      case FLOAT: o << "FLOAT"; break;
      case CODE: o << "CODE"; break;
      case STRING: o << "STRING"; break;
      default: o << type; break;
   }

   o << " w: " << width << " s: " << scale << " r: " << reference;

   return o;
}

BufrParamInt::
BufrParamInt( int id, int width, int scale, int reference )
   : BufrParamType( INT, id,  width, scale, reference )
{
   double maxVal = pow( 2, width );
   double pow10= pow( 10, scale );

//   cout << "maxVal: " << maxVal << " pow10: " << pow10 << endl;

   max = static_cast<long>((maxVal - reference)/pow10);
   min = static_cast<long>(reference/pow10);
}

std::ostream&
BufrParamInt::
print( std::ostream &o ) const
{
   BufrParamType::print( o );
   o << " [min-max>: [" << min << ", " << max << "]";

   if( ! unit.empty() )
      o << " [" << unit << "]";

   if( ! name.empty() )
      o << " : " << name;

   return o;
}

bool
BufrParamInt::
valid( int value )const
{
   return value<=max && value >= min;
}

BufrParamFloat::
BufrParamFloat( int id, int width, int scale, int reference )
   : BufrParamType( FLOAT, id, width, scale, reference )
{
   double maxVal = pow( 2, width );
   double pow10= pow( 10, scale );


   //cout << "maxVal: " << maxVal << " pow10: " << pow10 << endl;
   prec = 1/pow10;
   max = (maxVal - reference)/pow10;
   min = reference/pow10;
}

std::ostream&
BufrParamFloat::
print( std::ostream &o ) const
{
   BufrParamType::print( o );
   o << " [min-max]: [" << fixed << setprecision( scale ) << min
     << ", " << fixed << setprecision( scale ) << max
     << ", " << fixed << setprecision( scale ) << prec << "]";

   if( ! unit.empty() )
      o << " [" << unit << "]";

   if( ! name.empty() )
      o << " : " << name;

   return o;
}

bool
BufrParamFloat::
valid( float value_ )const
{
   double value=roundToPrec( value_ );
//   cerr << "BufrParamFloat: " << setprecision( 20 ) << min << "  " << setprecision( 20 ) << max << endl
//        << "                " << setprecision( 20 ) << value << endl;
   return value<=max && value >= min;
}

BufrParamCode::
BufrParamCode( int id, int width )
   : BufrParamType( CODE, id, width )
{
   max = static_cast<unsigned long>(pow(2, width));
}

std::ostream&
BufrParamCode::
print( std::ostream &o ) const
{
   BufrParamType::print( o );
   o << " [min-max] [0, " << max << "]";

   if( ! unit.empty() )
      o << " [" << unit << "]";

   if( ! name.empty() )
      o << " : " << name;

   return o;
}

bool
BufrParamCode::
valid( int value )const
{
   return value<=max;
}

BufrParamString::
BufrParamString( int id, int width )
   : BufrParamType( STRING, id, width/8 )
{
}

bool
BufrParamString::
BufrParamString::
valid( std::string &value )const
{
	//Check if width is less than maximum characters allowed
	//in CVALS, this is 80 in the FORTRAN code.

	if( width > 80 ) {
		ostringstream o;
		o << "The encoding of the PARAM '" << id << "' has a 'with' specifier "
	      << "of '" << width << "' the maximum size allowed by the BUFR encodings "
	      << "is 80.";
		throw BufrException( o.str() );
	}

   //TODO
   //Should replace all non CCITTIA5 with valid chars.
	value = fixString( value );
	if( value.length() > width )
		value.erase( width );
	else //value.length < width
		value += std::string( width-value.length(), ' ' );

   return true;
}

BufrParamFlag::
BufrParamFlag( int id, int width )
   : BufrParamType( INT, id, width )
{
}


bool
BufrParamFlag::
valid( int value )const
{
   return true;
}





BufrParamValidater::
BufrParamValidater()
{
}

BufrParamValidaterPtr
BufrParamValidater::
loadTable( const std::string &filepath_ )
{
   BufrParamValidaterPtr validater( new BufrParamValidater() );
   fs::path filepath( filepath_ );

   cerr << filepath << endl;

   if( ! fs::exists( filepath ) ) {
      //TODO
      //Check for the filename at predefined locations.
      //fs::path filename( filepath).filename();
      //filepath = predefinedlocation / filename;
      //if( ! fs::exists( filepath ) )
      //   return false;
      //For NOW just return false;
      return BufrParamValidaterPtr();
   }

   fs::ifstream in( filepath );

   if( !in )
     return BufrParamValidaterPtr();

   string line;
   ScanStringSpecList elems;

   //The position and with is found from the file btable.F
   //from the bufr software from ECMWF.
   //Negative width represent right justified.
   boost::assign::push_back( elems )
      ( 1, 6 )(8, 64)(73, 24)(100,-3)(113,-12)(117,-3);


   int n;

   while( getline( in, line ) ) {
      n = scanstring( line, elems );

      if( n < 6 )
         continue;

      BufrParamType *param = mkBufrParamType( n, elems );

      if( !param ) {
         continue;
      }

      //cout << *param << endl;
      (*validater)[param->id]=b::shared_ptr<BufrParamType>(param);
   }


   return validater;
   //string filename="/home/borgem/projects/workspace/kvbufrd/share/bufrtables/B0000000000098014001.TXT";
}

BufrParamValidaterPtr BufrParamValidater::loadTable( int masterTable, std::string *file) {
   char *tbl=getenv("BUFR_TABLES");
   ostringstream f;

   if( tbl ) {
      f << tbl; 
   } else {
      f << string(DATADIR);
   }

   f << "/B00000000000000" << masterTable <<  "000.TXT";

   if( file ) {
      *file=f.str();
   }
   return loadTable(f.str());
}


bool
BufrParamValidater::
isValid( int bufrParamId, float value )const
{
   BufrParamTypePtr param=findParamDef( bufrParamId );

   return param->valid( value );
}

bool
BufrParamValidater::
isValid( int bufrParamId, int value )const
{
   BufrParamTypePtr param=findParamDef( bufrParamId );

   return param->valid( value );
}

bool
BufrParamValidater::
isValid( int bufrParamId, std::string &value )const
{
   BufrParamTypePtr param=findParamDef( bufrParamId );

   return param->valid( value );
}

double
BufrParamValidater::
roundToPrec( int bufrParamId, float value )
{
   BufrParamTypePtr param=findParamDef( bufrParamId );
   return param->roundToPrec( value );
}


BufrParamTypePtr
BufrParamValidater::
findParamDef( int bufrParamId ) const
{
   const_iterator it = find( bufrParamId );

   if( it == end() )
      throw IdException( bufrParamId );

   return it->second;
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

