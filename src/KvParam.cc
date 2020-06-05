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
#include "DataElementList.h"
#include <algorithm>
#include <math.h>
using std::get;
using std::make_tuple;
using std::map;
using std::remove_if;

KvParamList::KvParamList() {}

int
KvParamList::numberOfValidParams() const
{
  int n = 0;
  for (const_iterator it = begin(); it != end(); ++it) {
    if ((*it)->value() != FLT_MAX)
      ++n;
  }

  return n;
}

void
KvParam::Sensor::set(float value, int level)
{
  levels[level] = value;
}

void
KvParam::Sensor::clean()
{
  auto it = levels.begin();
  while (it != levels.end()) {
    if (it->second == FLT_MAX || it->second == -32767) {
      it = levels.erase(it);
    } else {
      ++it;
    }
  }
}

float
KvParam::Sensor::getLevel(int level) const
{
  auto it = levels.find(level);
  if (it == levels.end())
    return FLT_MAX;
  return it->second;
}

std::tuple<float, int>
KvParam::Sensor::getFirstValue() const
{
  auto it = levels.begin();
  if (it == levels.end())
    return make_tuple(FLT_MAX, 0);
  return make_tuple(it->second, it->first);
}

KvParam::KvParam()
  : id_(INT_MAX)
  , levelScale_(1)
{
  KvParamList* params = DataElement::pParams.get();

  if (params)
    params->params.push_back(this);
}

KvParam::KvParam(KvParamList& paramList,
                 const char* name,
                 int id,
                 int levelScale)
{
  levelScale_ = powf(static_cast<float>(10), static_cast<float>(levelScale));
  name_ = name;
  id_ = id;
  paramList.params.push_back(this);
}

KvParam::KvParam(KvParamList& paramList, const KvParam& param)
  : name_(param.name_)
  , levelScale_(param.levelScale_)
  , id_(param.id_)
  , sensors_(param.sensors_)
{
  paramList.params.push_back(this);
}

/*
KvParam&
KvParam::copy(const KvParam& src, bool mustHaveSameId)
{
  if (this == &src)
    return *this;

  if (mustHaveSameId && id_ != src.id_) {
    std::ostringstream o;
    o << "KvParam: Expecting same id for src (" << src.id_ << " and this id ("
      << id_ << ").";
    throw std::runtime_error(o.str());
  }

  name_ = src.name_;
  levelScale_ = src.levelScale_;
  id_ = src.id_;
  sensors_ = src.sensors_;
  return *this;
}

/*
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
*/

KvParam&
KvParam::operator=(float rhs)
{
  value(rhs, 0, 0);
  return *this;
}

KvParam::operator float() const
{
  return value(0, 0);
}

int
KvParam::size() const
{
  int n = 0;
  for (auto& s : sensors_) {
    n += s.second.levels.size();
  }
  return n;
}

KvParam::Sensor*
KvParam::sensorRef(int sensor, bool addIfNotFound)
{
  auto it = sensors_.find(sensor);
  if (it == sensors_.end()) {
    if (addIfNotFound) {
      sensors_[sensor] = Sensor(sensor);
    } else {
      return nullptr;
    }
  }
  return &sensors_[sensor];
}

void
KvParam::clean()
{
  auto it = sensors_.begin();
  while (it != sensors_.end()) {
    it->second.clean();
    if (it->second.size() == 0) {
      it = sensors_.erase(it);
    } else {
      ++it;
    }
  }
}

std::map<int, float>
KvParam::getBySensorsAndLevels() const
{
  std::map<int, float> res;
  for (auto s : sensors_) {
    for (auto& l : s.second.levels) {
      auto il = res.find(l.first);
      if (il == res.end()) {
        res[l.first] = l.second;
      }
    }
  }

  return std::move(res);
}

float
KvParam::getFirstValueAtLevel(int level) const
{
  for (auto s : sensors_) {
    for (auto& l : s.second.levels) {
      if (l.first == level && l.second != -32767) {
        return l.second;
      }
    }
  }
  return FLT_MAX;
}

bool
KvParam::hasValidValues() const
{
  for (auto s : sensors_) {
    for (auto& l : s.second.levels) {
      if (l.second != FLT_MAX && l.second != -32767) {
        return true;
      }
    }
  }
  return false;
}

bool
KvParam::valid(int sensor, int level) const
{
  return value(sensor, level) != FLT_MAX;
}

void
KvParam::value(float val, int sensor, int level)
{
  if (val == -32767) {
    return;
  }

  auto s = sensorRef(sensor, true);
  auto l = level * levelScale_;
  s->set(val, l);
}

float
KvParam::value(int sensor, int level) const
{
  auto s = const_cast<KvParam*>(this)->sensorRef(sensor, false);
  if (!s)
    return FLT_MAX;
  return s->getLevel(level);
}

int
KvParam::valAsInt(int sensor, int level) const
{
  auto v = value(sensor, level);
  if (v == FLT_MAX)
    return INT_MAX;
  return static_cast<int>(v + 0.5);
}

KvParam&
KvParam::transform(float func(float))
{
  for (auto& s : sensors_) {
    for (auto& l : s.second.levels) {
      if (l.second != FLT_MAX && l.second != -32767) {
        l.second = func(l.second);
      }
    }
  }
  return *this;
}

std::vector<int>
KvParam::sensors() const
{
  auto v = std::vector<int>(sensors_.size());
  int i = 0;
  for (auto& s : sensors_) {
    v[i] = s.second.num;
    i++;
  }
  return std::move(v);
}

KvParam::Sensor
KvParam::getSensor(int sensor) const
{
  auto it = sensors_.find(sensor);
  if (it == sensors_.end())
    return Sensor(sensor);
  return it->second;
}

std::tuple<float, int>
KvParam::getFirstValueForSensor(int sensor) const
{
  const_cast<KvParam*>(this)->clean();
  auto it = sensors_.find(sensor);

  if (it == sensors_.end()) {
    return std::make_tuple(FLT_MAX, 0);
  }

  return it->second.getFirstValue();
}

std::tuple<float, int, int>
KvParam::getFirstValue() const
{
  auto it = sensors_.begin();
  if (it == sensors_.end()) {
    return make_tuple(FLT_MAX, 0, 0);
  }
  auto v = it->second.getFirstValue();
  return make_tuple(get<0>(v), it->first, get<1>(v));
}

std::ostream&
KvParam::print(std::ostream& o, bool printEmpty) const
{
  if (sensors_.empty()) {
    if (printEmpty) {
      o << name_ << ", id " << id_ << ", scale " << levelScale_
        << " : #values (non)";
    }
    return o;
  }

  o << name_ << ", id " << id_ << ", scale " << levelScale_ << " : #values "
    << size() << "\n";

  for (auto& s : sensors_) {
    for (auto& l : s.second.levels) {
      o << "  " << s.first << ", " << l.first << " : " << l.second << "\n";
    }
  }
  return o;
}

std::ostream&
operator<<(std::ostream& o, const KvParam& p)
{
  return p.print(o, false);
}