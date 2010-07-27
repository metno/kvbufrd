/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: bufr.h,v 1.8.2.3 2007/09/27 09:02:23 paule Exp $

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <kvalobs/kvPath.h>
#include <miconfparser/miconfparser.h>
#include <checkfile.h>
#include "InitLogger.h"
#include "ConfApp.h"


using namespace std;
struct Options {
   std::string confile;              //--conf
   std::string templatefile;         //--template
   std::string outconf;              //--out
};

void
getOptions(int argn, char **argv, Options &opt);

void
use(int exitstatus);


int
main( int argn, char **argv )
{
   Options opt;
   std::string confFile;
   miutil::conf::ConfSection *conf;

   InitLogger(argn, argv, "kvbufrconf");

   getOptions( argn, argv, opt );

   try{
      conf=miutil::conf::ConfParser::parse( opt.confile );
   }
   catch( const logic_error &ex ){
      LOGFATAL( ex.what() );
      return 1;
   }

   ConfApp app( argn, argv, conf );
   StInfoSysStationOutmessageList wmoList;
   TblStInfoSysStation station;
   StInfoSysSensorInfoList sensors;

   app.loadStationOutmessage( wmoList );

   for( StInfoSysStationOutmessageList::const_iterator it=wmoList.begin(); it != wmoList.end(); ++it ) {
      if( ! app.loadStationData( it->stationid(), station, sensors ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }

      cerr << it->stationid() << ", " << station.wmono() << ", " << station.name() << ", " << it->couplingDelay() << ", " << it->productcoupling() << ", " << it->priorityPrecip() << endl;
      cerr << "SENSORS:";

      for( StInfoSysSensorInfoList::const_iterator sit=sensors.begin(); sit != sensors.end(); ++sit )
         cerr << " " << sit->paramid()<< "(" << sit->physicalHeight() << ")";

      cerr << endl;
   }
}

void
getOptions(int argn, char **argv, Options &opt)
{
    struct option long_options[]={{"help", 0, 0, 'h'},
                                  {"conf", 1, 0, 'c'},
                                  {"template", 1, 0, 't'},
                                  {"out", 1, 0, 'o'},
                                  {0,0,0,0}};

    int c;
    int index;
    string error;
    string confileAtCommandLine;

    while(true){
        c=getopt_long(argn, argv, "hc:t:o:", long_options, &index);

        if(c==-1)
            break;

        switch(c){
        case 'h':
           use(0);
           break;

        case 'c':
            opt.confile=optarg;
            break;
        case 't':
           opt.templatefile=optarg;
           break;
        case 'o':
           opt.outconf=optarg;
           break;
        case '?':
            cerr << "Unknown option : <" << (char)optopt << "> unknown!" << endl;
            use(1);
            break;
        case ':':
            cerr << optopt << " missing argument!" << endl;
            use(1);
            break;
        default:
            cerr << "?? option caharcter: <" << (char)optopt << "> unknown!" << endl;
            use(1);
        }
    }

    if( opt.confile.empty() ) {
       if( checkfile( "./kvbufrconf.conf", R_OK, error ) )
         opt.confile = "./kvbufrconf.conf";
       else if( checkfile( string(kvPath("sysconfdir")+"/kvbufrconf.conf"), R_OK, error ) )
          opt.confile = kvPath("sysconfdir")+"/kvbufrconf.conf";
    } else if( ! checkfile( opt.confile, R_OK, error ) ) {
       confileAtCommandLine = opt.confile;
       opt.confile.erase();
    }

    if( opt.confile.empty() ) {
       cerr << "ERROR: Can't find any confile. " << endl
            << "  searched: ";

       if( ! confileAtCommandLine.empty() )
          cerr << "'" << confileAtCommandLine << "'";
       else
          cerr << "'" << "./kvbufrconf.conf" << "', '" << kvPath("sysconfdir")+"/kvbufrconf.conf" << "'.";

       cerr << endl;
       use( 1 );
    }

    if( !opt.templatefile.empty() && ! checkfile( opt.templatefile, R_OK, error ) ) {
       cerr << "template: " <<  error << endl;
       use( 1 );
    }

    ostringstream logmsg;
    logmsg << "Using configuration file: " << opt.confile << endl
           << "Reading template file:    " << (opt.templatefile.empty()?string("(none)"):opt.templatefile ) << endl
           << "outfile:                  " << (opt.outconf.empty()?string("(screen)"):opt.outconf ) << endl
           << endl;

    LOGINFO( logmsg.str() );
}


void
use(int exitstatus)
{
  cerr << "\n\tuse" << endl
       <<"\t   kvbufrconf [--help|-h] [--template|-t templatefile] [--conf|-c confile] [--out|o outfile] " << endl;

  exit(exitstatus);
}
