#include <iostream>
#include <string.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <decodeutility/kvalobsdataparser.h>
#include "CommandQueue.h"
#include "kvevents.h"
#include "Data.h"
#include <list>
#include "StationInfoParse.h"
#include <sstream>

extern const char *xml;
extern const char *conf;

namespace serialize=kvalobs::serialize;
namespace mithread=threadutil;
using std::cerr;
using std::string;
using std::endl;
using std::get;

StationList stationList;

void parseConf(const char *theConf ) {
  using namespace miutil::conf;
  std::istringstream in(theConf);
  StationInfoParse theParser;
  StationList tmpList;
  miutil::conf::ConfSection *conf;
  miutil::conf::ConfParser parser(in, true);

  try{
    conf=parser.parse();
    if( ! conf ) {
      cerr << "Failed to parse the configuration.... [\n" << theConf << "\n]\n";  
      exit(1);
    }
  }
  catch( const std::logic_error &ex ){
    cerr << "Failed to parse the configuration: [\n" << conf << "\n]\nException: " << ex.what() <<"\n";
    exit(1);
  }


  if (!theParser.parse(conf, tmpList)) {
    cerr << "Cant parse the configuration.\n";
    exit(1);
  }

  stationList = tmpList;
}


void DataReceiver_newData(const DataEvent &event);

void que_post(mithread::CommandBase *command) {
  auto event = dynamic_cast<DataEvent*>(command);

  if( !event) {
    cerr << "que_post: Not a DataEvent\n";
  }

  DataReceiver_newData(*event);


  delete command;
}


void DataReceiver_newData(const DataEvent &event) {
  DataKeySet dataInserted;
  auto data = event.data();

  cerr << "DataReceiver_newData: -- BEGIN -- \n";

  for (auto &index : data->getAllIndex()) {
    auto &it = data->get(index);
    std::list<kvalobs::kvData>::iterator dit = get<0>(it).begin();

    if (dit == get<0>(it).end()) {
      cerr << "DataReceiver_newData: Data received from kvalobs: Unexpected, NO Data!\n" 
      << "----- BEGIN -----\n"<<
        event.inCommingMessage()<<"\n----- END -----\n";
      continue;
    }
    
    std::list<Data> dataList;
    
    for (dit = get<0>(it).begin(); dit != get<0>(it).end(); dit++) {
      if (dataInserted.add(DataKey(*dit))) {
        dataList.push_back(Data(*dit));
      }
    }

    cerr << "DataReceiver_newData: -- (dataList) BEGIN -- \n";
    cerr << dataList ;
    cerr << "DataReceiver_newData: -- (dataList) END -- \n";
  }

  cerr << "DataReceiver_newData: -- END -- \n";
}



void KvDataConsumer_newData(const kvalobs::serialize::KvalobsData& dataIn, const std::string &msg)
{
  using std::get;
  std::list<kvalobs::kvData> data;
  std::list<kvalobs::kvTextData> textData;  
  
  dataIn.data(data);
  dataIn.data(textData);

  if( data.empty() && textData.empty()) {
    IDLOGINFO("kafka", "Empty data received");
    return;
  }

  kvalobs::KvObsDataMapPtr map(new kvalobs::KvObsDataMap());
  map->add(data);
  map->add(textData);

  try {
    auto event = new DataEvent(map);
    event->inCommingMessage(msg);
    que_post(event);
  } catch (...) {
  }
}


// -506 : 1
// 316: 

void KvDataConsumer_data(const char * msg, unsigned length) {
  cerr <<"KvDataConsumer_data: (in): " << xml << endl << endl;

  try {
    std::string message(msg, length);
    kvalobs::serialize::KvalobsData d;
    kvalobs::serialize::KvalobsDataParser::parse(message, d);
    //IDLOGINFO("kafka", message);

    cerr << "KvDataConsumer_data: -- data decoded BEGIN -- \n" << d << "\n-- data decoded END --\n\n";
    KvDataConsumer_newData(d, message);
  }
  catch( const std::exception &e) {
    cerr << "KvDataConsumer_data: Exception: " << e.what() << "\n"  << string(msg, length) << endl;
  }
}


int
main( int argn, const char *argv[]){
  

  std::list<kvalobs::kvData> data;
  serialize::KvalobsData sData(data);

  auto xml = serialize::KvalobsDataSerializer::serialize(data, "test");

  if( xml.empty() ) {
    cerr << "No xml\n\n";
  } else {
    cerr <<"Xml:\n" << xml << "\nEND\n\n";
  }

/*
  using namespace std;
  parseConf(conf);

  KvDataConsumer_data(xml, strlen(xml));
*/
}