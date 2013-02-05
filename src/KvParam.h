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
#ifndef __KV_PARAM_H__
#define __KV_PARAM_H__

#include <float.h>
#include <string>
#include <list>

class KvParamList;

class KvParam {

   std::string name_;
   int         id_;
   float       value_;

public:
   KvParam();
   KvParam( const KvParam &param)
      : name_( param.name_ ), id_( param.id_ ), value_(param.value_){};

   KvParam( KvParamList &paramList, const char *name, int id );
   KvParam( KvParamList &paramList, const KvParam &param );

   operator float()const{ return value_;}
   //operator double()const{ return static_cast<double>(value_);}

   KvParam& operator=( const KvParam &rhs );
   KvParam& operator=( float rhs );
   //KvParam& operator=( int rhs );

   std::string name()const { return name_; }
   int id()const{ return id_;}

   bool valid()const { return value_ != FLT_MAX; }
   void value( float val );
   float value()const { return value_; }
   int valAsInt()const;
};


class KvParamList
{
   friend class KvParam;
   std::list<KvParam*> params;
public:
   typedef std::list<KvParam*>::iterator iterator;
   typedef std::list<KvParam*>::const_iterator const_iterator;

   KvParamList();
   iterator begin() { return params.begin(); }
   iterator end() { return params.end(); }

   const_iterator begin()const { return params.begin(); }
   const_iterator end()const { return params.end(); }

   int numberOfValidParams() const;

   std::list<KvParam*>::size_type size()const { return params.size(); }
};


#endif
