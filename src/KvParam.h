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
#include <map>
#include <vector>


class KvParamList;

class KvParam {
public:
   struct Sensor {
      int num;
      std::map<int, float> levels;
      
      Sensor():num(-1){}
      Sensor( int sensor):num(sensor){}
 
      Sensor( const Sensor &s ):num(s.num), levels(s.levels){}
      float getLevel(int level)const;
      
   private:
      friend class KvParam;
      void set(float value, int level);
   };

private:
   std::string name_;
   int         levelScale_;
   int         id_;
   std::map<int, Sensor>  sensors_;

   Sensor* sensorRef(int sensor, bool addIfNotFound);
   //void copy(const KvParam &src);
   friend class DataElement;
public:
   KvParam();
   KvParam( const KvParam &param)
      : name_( param.name_ ), levelScale_(param.levelScale_), id_( param.id_ ), sensors_(param.sensors_){};

   KvParam( KvParamList &paramList, const char *name, int id, int levelScale=0 );
   KvParam( KvParamList &paramList, const KvParam &param );

   operator float()const;
   //operator double()const{ return static_cast<double>(value_);}

   KvParam& operator=( const KvParam &rhs )=delete;
   KvParam& operator=( float rhs );

   //KvParam& operator=( int rhs );

   //Return reference to this so we can chane 
   //throw runtime_error if mustHaveSameId is true and they differ.
   KvParam& copy(const KvParam &src, bool mustHaveSameId=true);

   std::string name()const { return name_; }

  /** Get data from all sensors and levels.
   * Data from sensors with lower sensor number
   * has priority before data with higher sensor number.
   * Return map<level, measurement>
   */
   std::map<int, float> getBySensorsAndLevels()const;
   
   /**
    * Return the the value from the first sensor that has data for this level.
    */
   float getFirstValueAtLevel(int level)const;

   int id()const{ return id_;}

   //Is there any valid values in any sensor and levels
   bool hasValidValues()const;
   bool valid(int sensor=0, int level=0 )const;
   void value( float val, int sensor=0, int level=0 );
   float value( int sensor=0, int level=0 )const;
   int valAsInt(int sensor=0, int level=0)const;


   /**
    * transform converts all values by calling the func. ex to convert celcius to kelvin.
    * Return referance to this so we can chane calls.
    */ 
   KvParam& transform(float func (float));
   std::map<int, Sensor>::const_iterator sensorsBegin()const { return sensors_.begin();};
   std::map<int, Sensor>::const_iterator sensorsEnd()const{ return sensors_.end();};
   std::vector<int> sensors()const;
   Sensor getSensor(int sensor)const; 
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
