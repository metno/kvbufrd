/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#include "KvDataConsumer.h"

#include <string>
#include "miconfparser/confsection.h"
#include "KvObsData.h"
#include "kvevents.h"

namespace miconf=miutil::conf;
using std::string;
using kvalobs::subscribe::DataSubscriber;

KvDataConsumer::KvDataConsumer(  const std::string &domain, const std::string &brokers, std::shared_ptr<dnmi::thread::CommandQue> newDataQue )
:DataSubscriber([this](const kvalobs::serialize::KvalobsData &data){newData(data);},domain,brokers),
 que(newDataQue){
}

void KvDataConsumer::newData(const kvalobs::serialize::KvalobsData &dataIn){
  std::list<kvalobs::kvData> data;
  kvalobs::KvObsDataMap map;

  dataIn.data(data);

  map.add(data);
  std::list<kvalobs::KvObsDataPtr> sortedData=map.getKvObsData();

  for( auto dp: sortedData) {
    try {
      new kvalobs::DataEvent( dp);
      que->post(new kvalobs::DataEvent( dp));
    }
    catch( ... ) {
    }
  }
  que->brodcast();
}


