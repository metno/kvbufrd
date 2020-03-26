#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "base64.h"

using namespace std;

bool ReadFile(const std::string &file, std::string &content) {
  ifstream fs(file.c_str());
  ostringstream ost;
  char ch;

  if (!fs) {
    return false;
  }

  while (fs.get(ch)) {
    ost.put(ch);
  }

  if (!fs.eof()) {
    fs.close();
    return false;
  }

  fs.close();
  content = ost.str();

  return true;
}

void use( int ret) {
  cerr << "Use: \n\tkvbase64 [-e] [-h ] <file> " << endl ;
  cerr << "\n\t-e encode to base64" << endl;
  cerr << "\n\t-h print this help" << endl << endl;
  exit(ret);
}


std::string&
replace(std::string &str, const std::string &what,
                const std::string &with) {
  std::string::size_type pos = str.find(what);

  while (pos != std::string::npos) {
    str.replace(pos, what.length(), with);
    pos = str.find(what, pos + with.length());
  }

  return str;
}


int 
main( int argn, const char *argv[]) {
  string content;
  string buf;
  bool encode=false;
  int argsNeeded=2;


  for(int i=0; i<argn; ++i ) {
    cerr << i << ": " << "'"<< argv[i] << "'\n";
    if( string(argv[i]) == "-e" ) {
      cerr << "Oh yes.\n\n";
      encode=true;
      argsNeeded++;
    } else if( argv[i] == "-h" ) {
     use(0);
   }
  }

  cerr << "argsNeeded: " << argsNeeded << endl;

  if( argn < argsNeeded ) {
    use(1);
  }

  string file=argv[argsNeeded-1];
  cerr << "File:  '" << file << "'" << endl;

  if( ! ReadFile(file, content ) ) {
    cerr << "Failed to read file <" << file << endl << endl;
    return 1;
  }

  if( ! encode ) { 
    cerr << "--- decode ---" << endl;
    replace(content, "\\r\\n","\n" );  
    cerr << "-- content BEGIN ----\n";
    cerr << content << "\n";
    cerr << "-- content END ----\n";
    miutil::decode64(content, buf);
  } else {
    cerr << "--- encode ---" << endl;
    miutil::encode64(content.data(), content.size(), buf);
  }

  cout << buf;
  cerr << endl << endl;
}
