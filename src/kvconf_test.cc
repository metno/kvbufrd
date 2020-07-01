#include <iostream>
#include <fstream>
#include <string.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <kvalobs/miutil/timeconvert.h>
#include "StationInfoParse.h"
#include "CommandPriority2Queue.h"
#include "kvevents.h"

using std::cerr;
using std::string;
using std::endl;
using std::get;
namespace pt=boost::posix_time;

StationList *parseConf(const char *theConf ) {
  using namespace miutil::conf;

  std::fstream in(theConf);
  StationInfoParse theParser;
  StationList *stationList( new StationList());
  miutil::conf::ConfSection *conf;
  miutil::conf::ConfParser parser(in, true);

  
  try{
    conf=parser.parse();
    if( ! conf ) {
      cerr << "Failed to parse the configuration.... [\n" 
        << theConf << "\n" 
        << parser.getError() << "\n"
        << "]\n";  
      exit(1);
    }
  }
  catch( const std::logic_error &ex ){
    cerr << "Failed to parse the configuration: [\n" << conf << "\n]\nException: " << ex.what() <<"\n";
    exit(1);
  }


  if (!theParser.parse(conf, *stationList)) {
    cerr << "Cant parse the configuration.\n";
    exit(1);
  }

  cerr << theParser.getErrors() << endl;
  return stationList;
}
//50070


void testPriorityQue(StationList *stations) 
{
  threadutil::CommandPriority2Queue que;
  long sid=50070;
  auto station_ = stations->findStation(sid);
  

  if( station_.empty() ) {
    cerr << "Failed to find station " << sid << "\n";
    exit(0);
  }
  auto station = *station_.begin();
  cerr << station->toIdentString() << endl;

  auto now=pt::second_clock::universal_time(); 
  auto d1 = now+pt::hours(1);
  auto d2 = now+pt::hours(2);
  auto d3 = now+pt::hours(3);
  auto d4 = now-pt::hours(1);


  que.postAndBrodcast(new ObsEvent(d2, station));
  que.postAndBrodcast(new ObsEvent(d1, station));
  que.postAndBrodcast(new ObsEvent(d4, station));
  auto event = new ObsEvent(d3, station);
  event->delayInQue(3000);
  que.postAndBrodcast(event);

  que.printQueue(cerr);

  cerr << "-------------- 1 ------------------\n";
  auto cmd = que.get(1);

  que.printQueue(cerr);

  cerr << "-------------- 2 ------------------\n";
  //sleep(3);
  cmd = que.get(1);
  cerr << "DelayInQue: " << dynamic_cast<ObsEvent*>(cmd)->timeInQue() << "ms" << "\n";
  que.printQueue(cerr);

  cerr << "-------------- 3 ------------------\n";
  cmd = que.get(1);
  cerr << "DelayInQue: " << dynamic_cast<ObsEvent*>(cmd)->timeInQue() << "ms" << "\n";
  que.printQueue(cerr);

  cerr << "-------------- 4 ------------------\n";
  cmd = que.get(1);
  
  while( !cmd ) {
    cerr << " --- nullptr ----\n";
    //sleep(1);
    que.printQueue(cerr);
    cmd = que.get(1);
  }

  cerr << "DelayInQue: " << dynamic_cast<ObsEvent*>(cmd)->timeInQue().total_milliseconds() << "ms" << "\n";

  cerr << "que size: " << que.size() << "\n";
  que.printQueue(cerr);
}


void listStations(StationList *stations) 
{
  int nWmo=0;
  int nTids1=0;
  for ( auto s : *stations) {
    if( s->code() == 0 ) {
      if( s->typepriority().size() == 1 )
        nTids1++; 
      cerr << s->wmono() << " #sids: " << s->definedStationID().size() << " (";
      for( auto sid : s->definedStationID() ) {
        cerr << " " << sid;
      }
      cerr << " )";
      
      cerr << " #tids: " << s->typepriority().size() << " :"  ;
      for( auto tid : s->typepriority()) {
        cerr << " " << tid;
      }
      cerr << "  ("<< s->toIdentString() << ")";
      cerr << "\n";
      nWmo++;
    }
  }

  cerr << "Number of wmo stations: " << nWmo << " with 1 tid: "  << nTids1 << " (" << nWmo-nTids1<< ").\n";
}



int
main( int argn, const char *argv[]){
  using namespace std;

  milog::Logger::logger().logLevel(milog::INFO);

  if( argn < 2 ) {
  cerr << "\n\tMissing conf file.\n\n";
    return 1;
  }
  cerr << "Conffile: '" << argv[1] << "'\n"; 
  auto stations = parseConf(argv[1]);

  testPriorityQue( stations);

  //listStations(stations);

  /*
  int sid=20926;
  int tid=22;
  auto sts = stations->findStation(sid, tid);

  cout << "Number of stations witt sid: " << sid << " tid: " << tid <<   ": " << sts.size() << endl;
  for( auto st : sts ) {
    cout << *st << endl;

    cerr << "\n ---------\n" << endl;
  }
*/

/*
  for( auto st : *stations ) {
    cout << *st << endl;

    cerr << "\n ---------\n" << endl;
  }
  */
}

