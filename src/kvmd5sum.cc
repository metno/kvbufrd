#include <sstream>
#include <fstream>
#include <iostream>
#include "md5.h"
 
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
  cerr << "Use: \n\tkvmd5sum [-h ] <file> " << endl ;
  cerr << "\nCompute the md5sum of the file.\n";
  cerr << "\n\t-h print this help" << endl << endl;
  exit(ret);
}



//    cout << "md5 of 'grape': " << md5("grape") << endl;
int 
main( int argn, const char *argv[]) {
  string content;
  string buf;
  int argsNeeded=2;


  for(int i=0; i<argn; ++i ) {
   if( string(argv[i]) == "-h" ) {
     use(0);
   }
  }


  if( argn < argsNeeded ) {
    use(1);
  }

  string file=argv[argsNeeded-1];
  
  if( ! ReadFile(file, content ) ) {
    cerr << "Failed to read file <" << file << endl << endl;
    return 1;
  }

  cout << file << " \t " << md5(content)   << endl;
}
