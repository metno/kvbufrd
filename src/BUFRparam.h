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
#include <vector>
#include <map>
#include <string>
#include <iostream>

struct BufrParamType
{
   typedef enum {INT, FLOAT, CODE, STRING} Type;
   BufrParamType( Type type_, int w, int s=0, int r=0  )
      : type(type_), width( w ), scale( s ), reference( r ){};
   Type type;
   int width;
   int scale;
   int reference;

   std::ostream& print( std::ostream &o ) const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamType &pt);
};

struct BufrParamInt : public BufrParamType
{
public:
   BufrParamInt( int width, int scale, int reference );
   bool valid( int value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamInt &pt);
};

struct BufrParamFloat : public BufrParamType
{
public:
   BufrParamFloat( int width, int scale, int reference );
   bool valid( double value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamFloat &pt);
};


struct BufrParamCode : public BufrParamType
{
   std::vector<int> validCode;
   BufrParamCode( int width );

   bool valid( int value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamCode &pt);
};

struct BufrParamString : public BufrParamType
{
   std::vector<int> validCode;
   BufrParamString( int width );

   bool valid( const std::string &value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamString &pt);
};

struct BufrParamFlag : public BufrParamType
{

   BufrParamFlag( int width );

   bool valid( int value )const;
   friend std::ostream& operator<<(std::ostream &o, const BufrParamFlag &pt);
};

class BufrValidater : public std::map<int, BufrParamType*>
{
public:
   BufrValidater();

   bool loadTable( const std::string &filename );

};

std::ostream& operator<<(std::ostream &o, const BufrParamType &pt);
std::ostream& operator<<(std::ostream &o, const BufrParamInt &pi);
std::ostream& operator<<(std::ostream &o, const BufrParamFloat &pf);
std::ostream& operator<<(std::ostream &o, const BufrParamCode &pc);
std::ostream& operator<<(std::ostream &o, const BufrParamString &ps);
std::ostream& operator<<(std::ostream &o, const BufrParamFlag &pfl);
#endif /* BUFRPARAM_H_ */
