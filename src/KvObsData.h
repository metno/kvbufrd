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


#ifndef SRC_KVOBSDATA_H_
#define SRC_KVOBSDATA_H_

#include <map>
#include <list>
#include <tuple>
#include <memory>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvData.h"
#include "kvalobs/kvTextData.h"

namespace kvalobs {

typedef std::tuple<std::list<kvalobs::kvData>,std::list<kvalobs::kvTextData>> ObsDataElement;
typedef std::map<int, ObsDataElement> KvObsData;
typedef std::shared_ptr<KvObsData> KvObsDataPtr;

class KvObsDataMap : public std::map<long, std::map<boost::posix_time::ptime, std::map<long, ObsDataElement>>>
{
 public:
  //< stationid, typeid, obstime
  typedef std::tuple<long, long, boost::posix_time::ptime> Index;
  typedef std::list<Index> IndexList;
  KvObsDataMap();
  ~KvObsDataMap();

  void add( long staionid, long typeID, const boost::posix_time::ptime &obstime, std::list<kvalobs::kvData> &&data);
  void add( long staionid, long typeID, const boost::posix_time::ptime &obstime, std::list<kvalobs::kvTextData> &&data);

  void add( long staionid, long typeID, const boost::posix_time::ptime &obstime, const std::list<kvalobs::kvData> &data);
  void add( long staionid, long typeID, const boost::posix_time::ptime &obstime, const std::list<kvalobs::kvTextData> &data);

  void add(const std::list<kvalobs::kvData> &data);
  void add(const std::list<kvalobs::kvTextData> &data);

  /**
   * Returns a list of KvObsData, where each element in the list
   * contains data for only one stationid and obstime. But there may
   * be more than one typid.
   * Each list is sorted by typeid and the return list is sorted by stationid and obstime
   * in increasing order, ie lowest stationid and obstime first.
   */
  std::list<KvObsDataPtr> getKvObsData();
  IndexList getAllIndex()const;
  KvObsDataPtr getAt(const Index &index);
  ObsDataElement& get(const Index &index);

  friend std::ostream &operator<<(std::ostream &o, const KvObsDataMap::Index &i);  
};

typedef std::shared_ptr<KvObsDataMap> KvObsDataMapPtr;

std::ostream &operator<<(std::ostream &o, const KvObsDataMap::Index &i);

std::ostream &operator<<(std::ostream &o, const KvObsData &od);

void sortObsDataElementByObstime(ObsDataElement *data);
void sortKvObsDataByObstime(KvObsData *data);

}

#endif /* SRC_KVOBSDATA_H_ */
