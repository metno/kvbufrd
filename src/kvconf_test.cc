#include <iostream>
#include <fstream>
#include <string.h>
#include "StationInfoParse.h"

using std::cerr;
using std::string;
using std::endl;
using std::get;

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

  int nWmo=0;
  int nTids1=0;
  for ( auto s : *stations) {
    if( s->code() == 0 ) {
      if( s->typepriority().size() == 1 )
        nTids1++; 
      cerr << s->wmono() << " #tids: " << s->typepriority().size() << " :"  ;
      for( auto tid : s->typepriority()) {
        cerr << " " << tid;
      }
      cerr << "  ("<< s->toIdentString() << ")";
      cerr << "\n";
      nWmo++;
    }
  }

  cerr << "Number of wmo stations: " << nWmo << " with 1 tid: "  << nTids1 << " (" << nWmo-nTids1<< ").\n";
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

