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

#include <decodeutility/kvalobsdataserializer.h>
#include <decodeutility/kvalobsdataparser.h>
#include "KvObsData.h"
#include "kvevents.h"
#include <miconfparser/confsection.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <string>
#include <sstream>
#include <map>

namespace miconf = miutil::conf;
namespace pt= boost::posix_time;
using kvalobs::subscribe::DataSubscriber;
using std::string;


KvDataConsumer::KvDataConsumer(
  const std::string& domain,
  const std::string& brokers,
  std::shared_ptr<threadutil::CommandQueue> newDataQue)
  : DataSubscriber(
      [this](const kvalobs::serialize::KvalobsData& data) { newData(data, ""); },
      domain,
      brokers)
  , que(newDataQue)
{
  setDebugWriter(debugMsgWriter);
}


void 
KvDataConsumer::debugMsgWriter(const std::string &message, const kvalobs::serialize::KvalobsData &d) {
  //IDLOGINFO("kafka", "---------------- BEGIN --------------\n\n" << message << "\n---------------- END --------------");
}

namespace {
  void logReceived(const std::list<kvalobs::kvData> &data, const std::string &producer_) {
    using namespace std;
    string producer(producer_);

    if( producer.empty() ) {
      producer="<unkown>";
    }

    ostringstream o;
    map<string, int> received;
    for( auto &e : data){
      o << e.stationID()<<"/" << e.typeID() << "/"  << pt::to_kvalobs_string(e.obstime(),'T');
      received[o.str()]++;
      o.str("");
    }

    if( received.size()>0) {
      for( auto &e : received) {
        o << e.first << " #" << e.second << " params, producer: " << producer;
        IDLOGINFO("kafka_received", o.str());
        o.str("");
      }
    }
  }
}



void
KvDataConsumer::newData(const kvalobs::serialize::KvalobsData& dataIn, const std::string &msg)
{
  using std::get;
  std::list<kvalobs::kvData> data;
  std::list<kvalobs::kvTextData> textData;  
  
  dataIn.data(data);
  dataIn.data(textData);

  if( data.empty() && textData.empty()) {
    IDLOGINFO("kafka", "Empty data received\nXML [" << msg << "]");
    return;
  }

  logReceived(data, dataIn.producer());
  
  kvalobs::KvObsDataMapPtr map(new kvalobs::KvObsDataMap());
  map->add(data);
  map->add(textData);

  try {
    auto event = new DataEvent(map);
    event->inCommingMessage(msg);
    //std::cerr << "KvDataConsumer: new message received and posted on incomming queue.\n";
    que->postAndBrodcast(event);
  } 
  catch(const threadutil::QueSuspended &e) {
    LOGINFO("KvDataConsumer: que suspended!");
  }
  catch (...) {
  }
}

void KvDataConsumer::data(const char * msg, unsigned length) {
  unsigned l=25;

  if(length < 25 ) {
    l=length;
  }

  std::string tmp(msg, length);
  std::string::size_type i=tmp.find_first_not_of(" \t\n\r");
  tmp = tmp.substr(i);
  i=tmp.find("<?xml ");

  if( i!=0) {
    tmp=std::string(msg, length);
  } else {
    tmp.erase();
  }

  if ( tmp.size() != 0 ) {
    IDLOGERROR("kafka", "New (" << length<< "): " << string(msg, l) << "\n" << tmp);
  }
  

  try {
    std::string message(msg, length);
    kvalobs::serialize::KvalobsData d;
    kvalobs::serialize::KvalobsDataParser::parse(message, d);
    //IDLOGINFO("kafka", message);
    newData(d, message);
  }
  catch( const std::exception &e) {
    IDLOGERROR("kafka", "Exception: " << e.what() << "\n"  << string(msg, length) );
  }
}

void
KvDataConsumer::error(int code, const std::string& msg)
{
  IDLOGERROR("kafka", "code: " << code << "\n" << msg);
  kvalobs::subscribe::DataSubscriber::error(code, msg);
}
