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

#ifndef SRC_KVDATACONSUMER_H_
#define SRC_KVDATACONSUMER_H_

#include <string>
#include "decodeutility/kvalobsdata.h"
#include "kvsubscribe/DataSubscriber.h"
#include "miconfparser/miconfparser.h"
#include "dnmithread/CommandQue.h"
#include "App.h"

class KvDataConsumer : public kvalobs::subscribe::DataSubscriber {
  KvDataConsumer()=delete;
  KvDataConsumer(const KvDataConsumer &)=delete;
  KvDataConsumer& operator=(const KvDataConsumer &)=delete;

 public:
  KvDataConsumer( const std::string &domain, const std::string &brokers, std::shared_ptr<dnmi::thread::CommandQue> newDataQue );
  void newData(const kvalobs::serialize::KvalobsData &data);

 private:
  std::shared_ptr<dnmi::thread::CommandQue> que;
};

#endif
