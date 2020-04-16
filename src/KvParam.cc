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

#include <math.h>
#include "KvParam.h"
#include "DataElementList.h"

KvParamList::
KvParamList()
{
}

int
KvParamList::
numberOfValidParams() const
{
   int n=0;
   for( const_iterator it=begin(); it != end(); ++it ) {
      if( (*it)->value() != FLT_MAX )
         ++n;
   }

   return n;
}

void
KvParam::Sensor::set(float value, int level) {
   levels[level]=value;
}

float 
KvParam::Sensor::getLevel(int level)const {
   auto it = levels.find(level);
   if( it == levels.end() )
      return FLT_MAX;
   return it->second;
}


KvParam::KvParam()
   : id_(INT_MAX), levelScale_(1)
{
   KvParamList *params = DataElement::pParams.get();

   if( params )
      params->params.push_back( this );
}

KvParam::
KvParam( KvParamList &paramList, const char *name, int id, int levelScale )
{
   levelScale_= powf(static_cast<float>(10), static_cast<float>(levelScale));
   name_=name;
   id_ = id;
   paramList.params.push_back( this );
}

KvParam::
KvParam( KvParamList &paramList, const KvParam &param )
   : name_( param.name_ ), levelScale_(param.levelScale_), id_( param.id_ ), sensors_(param.sensors_)
{
   paramList.params.push_back( this );
}

KvParam&
KvParam::
operator=( const KvParam &rhs )
{
   if( this != &rhs ) {
      name_ = rhs.name_;
      levelScale_ = rhs.levelScale_;
      id_ = rhs.id_;
      sensors_=rhs.sensors_;
   }
   return *this;
}

KvParam&
KvParam::
operator=( float rhs )
{
   value(rhs, 0 , 0);
   return *this;
}

KvParam::operator float()const{ 
   return value(0,0);
}


KvParam::Sensor* KvParam::sensorRef(int sensor, bool addIfNotFound)
{
   auto it = sensors_.find(sensor);
   if( it == sensors_.end() ) {
      if(addIfNotFound) {
         sensors_[sensor]=Sensor(sensor);
      } else {
         return nullptr;
      }
   }
   return &sensors_[sensor];
}

std::map<int, float> 
KvParam::getBySensorsAndLevels()const {
  std::map<int, float> res;
  for ( auto s : sensors_) {
    for( auto &l : s.second.levels ){
      auto il = res.find(l.first);
      if( il == res.end()) {
        res[l.first]=l.second;
      }
    }
  }

  return std::move(res);
}

float KvParam::getFirstValueAtLevel(int level)const {
   for ( auto s : sensors_) {
      for( auto &l : s.second.levels ){
         if( l.first == level ) {
            return l.second;
         }
      }
   }
   return FLT_MAX;
}



bool 
KvParam::
valid(int sensor, int level )const 
{ 
   return value(sensor, level)!=FLT_MAX;
}


void
KvParam::
value( float val, int sensor, int level )
{
   auto s = sensorRef(sensor, true);
   auto l=level*levelScale_;
   s->set(val, l);
}


float 
KvParam::
value( int sensor, int level)const 
{ 
   auto s = const_cast<KvParam*>(this)->sensorRef( sensor, false);
   if( !s )
      return FLT_MAX;
   return s->getLevel(level); 
}

int
KvParam::
valAsInt(int sensor, int level )const
{
   auto v=value(sensor, level);
   if( v==FLT_MAX)
      return INT_MAX;
   return static_cast<int>( v + 0.5 );
}


std::vector<int> 
KvParam::sensors()const
{
   auto v = std::vector<int>(sensors_.size());
   int i=0;
   for( auto &s : sensors_){
      v[i]=s.second.num;
      i++;
   }
   return std::move(v);
}

KvParam::Sensor 
KvParam::getSensor(int sensor)const
{
   auto it=sensors_.find(sensor);
   if(it == sensors_.end())
      return Sensor(sensor);
   return it->second;
}