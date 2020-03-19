/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

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
#include <algorithm>
#include "KvObsData.h"
#include <miutil/timeconvert.h>

using std::make_tuple;
using std::for_each;
using kvalobs::kvData;
using kvalobs::kvTextData;
namespace pt=boost::posix_time;

namespace kvalobs {

KvObsDataMap::KvObsDataMap()
{
}

KvObsDataMap::~KvObsDataMap()
{}

void KvObsDataMap::add( long stationid, long typeID, const boost::posix_time::ptime &obstime, std::list<kvalobs::kvData> &&data)
{
  ObsDataElement &mydata = get(make_tuple(stationid, typeID, obstime));
  std::get<0>(mydata)=data;
}

void KvObsDataMap::add( long stationid, long typeID, const boost::posix_time::ptime &obstime, std::list<kvalobs::kvTextData> &&data)
{
  ObsDataElement &mydata = get(make_tuple(stationid, typeID, obstime));
  std::get<1>(mydata)=data;
}


void KvObsDataMap::add( long stationid, long typeID, const boost::posix_time::ptime &obstime, const std::list<kvalobs::kvData> &data)
{
  ObsDataElement &mydata = get(make_tuple(stationid, typeID, obstime));
  for(auto &d : data ){
    auto &&it=std::get<0>(mydata).begin();
    for( ; it!=std::get<0>(mydata).end(); ++it) {
      if( it->paramID() == d.paramID() && it->sensor()==d.sensor() && it->level() == d.level() &&it->stationID() == d.stationID() && it->typeID() == d.typeID() && it->obstime() == d.obstime())
        break;
    }
    if(it==std::get<0>(mydata).end())
      std::get<0>(mydata).push_back(d);
  }
}

void KvObsDataMap::add( long stationid, long typeID, const boost::posix_time::ptime &obstime, const std::list<kvalobs::kvTextData> &data)
{
  ObsDataElement &mydata = get(make_tuple(stationid, typeID, obstime));
  for(auto &d : data ){
    auto &&it=std::get<1>(mydata).begin();
    for( ; it!=std::get<1>(mydata).end(); ++it) {
      if( it->stationID() == d.stationID() && it->typeID() == d.typeID() && it->obstime() == d.obstime())
        break;
    }
    if(it==std::get<1>(mydata).end())
      std::get<1>(mydata).push_back(d);
  }
}

void KvObsDataMap::add(const std::list<kvalobs::kvData> &data){
  for( auto &it : data){
    ObsDataElement &mydata = get(make_tuple(it.stationID(), it.typeID(), it.obstime()));
    std::get<0>(mydata).push_back(it);
  }
}

void KvObsDataMap::add(const std::list<kvalobs::kvTextData> &data){
  for( auto &it : data){
    ObsDataElement &mydata = get(make_tuple(it.stationID(), it.typeID(), it.obstime()));
    std::get<1>(mydata).push_back(it);
  }
}


KvObsDataMap::IndexList KvObsDataMap::getAllIndex()const{
  IndexList indexs;

  for( auto &stationid: *this )
    for( auto &obstime : stationid.second)
      for(auto &type : obstime.second)
        indexs.push_back( make_tuple(stationid.first, type.first, obstime.first));
  return indexs;
}

ObsDataElement& KvObsDataMap::get(const Index &index)
{
  return (*this)[std::get<0>(index)][std::get<2>(index)][std::get<1>(index)];
}

KvObsDataPtr KvObsDataMap::getAt(const Index &index) {
  using std::get;
  KvObsDataPtr res;
  auto sid = get<0>(index);
  auto tid = get<1>(index);
  auto obstime = get<2>(index);

  auto sit=find(sid);
  if( sit == end() ) {
    return res;
  }

  auto obtit=sit->second.find(obstime);
  if( obtit==sit->second.end() ) {
    return res;
  }

  auto tit=obtit->second.find(tid);
  if(tit == obtit->second.end()) {
    return res;
  }

  res.reset(new KvObsData());
  //(*p)[sid]=std::make_tuple(get<0>(tit->second), get<1>(tit->second));
  (*res)[sid]=tit->second;
  return res;
}


std::list<KvObsDataPtr> KvObsDataMap::getKvObsData(){
  std::list<KvObsDataPtr> res; //=KvObsDataPtr(new KvObsData());
  for( auto &stationid: *this ) {
    for( auto &obstime : stationid.second){
      KvObsDataPtr s=KvObsDataPtr(new KvObsData());
      for(auto &type : obstime.second){
        for_each(std::get<0>(type.second).begin(), std::get<0>(type.second).end(),
            [&](const kvData &d) {std::get<0>((*s)[stationid.first]).push_back(d);});
        for_each(std::get<1>(type.second).begin(), std::get<1>(type.second).end(),
            [&](const kvTextData &d) {std::get<1>((*s)[stationid.first]).push_back(d);});
      }
      res.push_back(s);
    }
  }
  return res;
}

void sortObsDataElementByObstime(ObsDataElement *data){
  std::get<0>(*data).sort([](const kvData &d1,const kvData &d2){return d1.obstime() < d2.obstime();});
  std::get<1>(*data).sort([](const kvTextData &d1,const kvTextData &d2){return d1.obstime() < d2.obstime();});
}

void sortKvObsDataByObstime(KvObsData *data){
  for_each(data->begin(), data->end(),
           [](KvObsData::value_type &v){sortObsDataElementByObstime(&v.second);});
}


std::ostream &operator<<(std::ostream &o, const KvObsDataMap::Index &i) {
  using std::get;
  o << "Index(" << get<0>(i) << ", " << get<1>(i) << ", " << pt::to_kvalobs_string(get<2>(i)) << ")";
  return o;
}


std::ostream &operator<<(std::ostream &o, const KvObsData &od) {
  int i=0;
  for( auto &ode: od) {
    for( auto &d : std::get<0>(ode.second)) {
      o << i << " d: " << ode.first << " -> " << d << "\n"; 
    }
    for( auto &d : std::get<1>(ode.second)) {
      o << i << " t: " << ode.first << " -> " << d << "\n"; 
    }
    i++;
  }
  return o;
}

}
