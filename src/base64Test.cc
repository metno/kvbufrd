#include <fstream>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/crc.hpp>
#include <sstream>
#include "base64.h"

using namespace std;


boost::uint16_t
crc( const std::string &buf )
{
   boost::crc_32_type crcChecker;

   crcChecker.process_bytes( buf.data(),  buf.length() );
   return crcChecker.checksum();
}


bool
readFile( const std::string &name, std::string &buf )
{
   ostringstream ost;
   char ch;
   int i=0;
   ifstream inf( name.c_str() );

   if( ! inf ) {
      cerr << "grmf .... \n";
      return false;
   }

   while( inf.good()  ) {
      ch = inf.get();

      if( inf.good() ) {
         i++;
         cerr << "("<<(isprint( ch )?ch:'.' )<<")";
         ost << ch;
      }
   }
   cerr << endl << "i: " << i << endl;
   buf = ost.str();
   return inf.eof();
}


const char *mybufr=
"DQ0KWkNaQw0NCklTTkQ5OSBIWUJSIDIxMTEwMA0NCkJVRlIAALkEAAAWAABYAAAAAAAAAA4A"
"B9oKFQsAAAAADQAAAYDHTwQZCyoAAIoAAgGkJ6knKaqnIhAQEBAQEBAQEBAQEA+1SqwH9pMC"
"VJ5gEAQIDE8ZPMPc///////4BkNTa0RcMAZP/4Bkf///////////+AAD9//////H////9///"
"79H/wDI////////8AyP////////////8D6IE/sMQcxf////98T/hWf//////gD///8A3Nzc3"
"AAAADQoNDQoKCgoKCgoKTk5OTg0K";

int
main( int argn, char **argv )
{
   string filename("1492-2206.bufr");
   string buf;
   string b64;
   string db64;

   if( ! readFile( filename, buf ) ) {
      cerr << "Failed to read file: " << filename << endl;
      return 1;
   }

   ofstream of("base64Test.out");

   if( ! of ) {
      cerr << "Cant open output file!" << endl;
      return 1;
   }

   miutil::encode64( buf.data(), buf.size(), b64 );

   cerr << "Base64: ------" << endl
        << b64 << endl << " -------" << endl;;

   miutil::decode64( b64, db64 );
   of.write( db64.data(), db64.size() );
   of.close();

   cerr << "Filesize: " << buf.size() << endl;
   cerr << "crc: " << crc( buf ) << endl;

   miutil::decode64( mybufr, db64 );

   of.open("1003-2206.bufr");
   of.write( db64.data(), db64.size() );
   of.close();
}
