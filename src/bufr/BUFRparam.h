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
#ifndef __BUFRPARAM_H__
#define __BUFRPARAM_H__
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "bufrexceptions.h"

struct BufrParamType
{
   typedef enum {INT, FLOAT, CODE, STRING} Type;
   BufrParamType( Type type_, int id_, int w, int s=0, int r=0  )
      : type(type_), id( id_ ), width( w ), scale( s ), reference( r ){};
   Type type;
   int id;
   int width;
   int scale;
   int reference;
   std::string name;
   std::string unit;

   std::string expecting()const;
   std::string typeToString()const;

   virtual bool valid( int value ) const;
   virtual bool valid( float value ) const;
   virtual bool valid( std::string& value ) const;
   virtual double roundToPrec( float value )const;
   virtual std::ostream& print( std::ostream &o ) const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamType &pt);
};

struct BufrParamInt : public BufrParamType
{
public:
   long min, max;
   BufrParamInt( int id, int width, int scale, int reference );
   virtual bool valid( int value )const;
   virtual double roundToPrec( float value )const { return round(value); }
   virtual std::ostream& print( std::ostream &o ) const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamInt &pt);
};

struct BufrParamFloat : public BufrParamType
{
public:
   double min, max, prec;
   BufrParamFloat( int id, int width, int scale, int reference );
   virtual double roundToPrec( float value ) const { return round(value/prec)*prec; }
   virtual bool valid( float value )const;
   virtual std::ostream& print( std::ostream &o ) const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamFloat &pt);
};


struct BufrParamCode : public BufrParamType
{
   std::vector<int> validCode;
   BufrParamCode( int id, int width );
   unsigned long max;
   virtual double roundToPrec( float value )const { return round(value); }
   virtual bool valid( int value )const;
   virtual std::ostream& print( std::ostream &o ) const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamCode &pt);
};

struct BufrParamString : public BufrParamType
{
   BufrParamString( int id, int width );
   virtual bool valid( std::string &value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamString &pt);
};

struct BufrParamFlag : public BufrParamType
{

   BufrParamFlag( int id, int width );

   virtual bool valid( int value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamFlag &pt);
};

class  BufrParamValidater;

typedef boost::shared_ptr<BufrParamType> BufrParamTypePtr;
typedef boost::shared_ptr<BufrParamValidater> BufrParamValidaterPtr;

class BufrParamValidater : public std::map<int, BufrParamTypePtr>
{
public:
   BufrParamValidater();

   //Throws
   //  IdException when the bufrParamId
   //              is not defined.
   //  TypeException When the value type do not match the paramdef type.
   bool isValid( int bufrParamId, float value )const;
   bool isValid( int bufrParamId, int value )const;
   bool isValid( int bufrParamId, std::string &value )const;
   double roundToPrec( int bufrParamId, float value );
   BufrParamTypePtr findParamDef( int bufrParamId ) const;
   static BufrParamValidaterPtr loadTable( const std::string &filename );
   /**
    * Load ParamTable B*.TXT. Either using enviromnet variable BUFR_TABLES or
    * hardcoded path decided from compile time config DATADIR. 
    * On return the 'file' parameter is set to  the file that is used to read the ParamTable from.
    * Return BufrParamValidaterPtr != null on success.
    */
   static BufrParamValidaterPtr loadTable( int masterTable, std::string *file);

};



std::ostream& operator<<(std::ostream &o, const BufrParamType &pt);
std::ostream& operator<<(std::ostream &o, const BufrParamInt &pi);
std::ostream& operator<<(std::ostream &o, const BufrParamFloat &pf);
std::ostream& operator<<(std::ostream &o, const BufrParamCode &pc);
std::ostream& operator<<(std::ostream &o, const BufrParamString &ps);
std::ostream& operator<<(std::ostream &o, const BufrParamFlag &pfl);
#endif /* BUFRPARAM_H_ */
