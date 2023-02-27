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

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "checkfile.h"
#include "kvbufrconfOptions.h"

using namespace std;


std::string Options::getType()const
{
   switch(type){
      case NON: return "<undefined>";
      case PRECIP: return "precip";
      case SVV: return "svv";
      case SHIP: return "ship";
      case SYNOP: return "synop";
      case SYNOP_WIGOS: return "synop_wigos";
      case BSTATIONS: return "bstations";
      case MBOUY: return "moored buoys";
      default:
         return "<unknown type>";
   }
}

void
getOptions(int argn, char **argv, Options &opt)
{
   struct option long_options[]={{"help", 0, 0, 'h'},
                                 {"conf", 1, 0, 'c'},
                                 {"template", 1, 0, 't'},
								         {"debug",1, 0, 'd' },
                                 {"out", 1, 0, 'o'},
                                 {"svv", 0, 0, 0 },
                                 {"precip", 0, 0, 0 },
                                 {"ship", 0, 0, 0 },
                                 {"bstations", 0, 0, 0},
                                 {"synop", 0, 0, 0},
                                 {"synop-wigos", 0, 0, 0},
                                 {"mbouy", 0, 0, 0},
                                 {0,0,0,0}};

   int c;
   int index;
   string error;
   string confileAtCommandLine;

   opt.type = Options::NON;
   opt.nIsTypes = 0;

   while(true){
      c=getopt_long(argn, argv, "hc:t:o:d:", long_options, &index);

      if(c==-1)
         break;

      switch(c){
         case 0:
            if( strcmp( long_options[index].name, "svv") == 0) {
               opt.type= Options::SVV; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "precip") == 0 ) {
                opt.type= Options::PRECIP; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "ship") == 0 ) {
                opt.type= Options::SHIP; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "bstations") == 0 ) {
                opt.type= Options::BSTATIONS; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "synop") == 0 ) {
                opt.type= Options::SYNOP; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "synop-wigos") == 0 ) {
                opt.type= Options::SYNOP_WIGOS; ++opt.nIsTypes;
            } else if( strcmp( long_options[index].name, "mbouy") == 0 ) {
               cerr << "Option: --mbuoy\n";
                opt.type= Options::MBOUY; ++opt.nIsTypes;
            } else {
               use( 1 );
            }
            break;
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

         case 'd':
        	   opt.debug=atoi( optarg );
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
            break;
      }
   }

   if( opt.nIsTypes > 1  )
      use( 1 );

   if( opt.type == Options::NON )
       opt.type = Options::SYNOP;

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
         << "bufr type:                " << opt.getType() << endl 
         << endl;

   LOGINFO( logmsg.str() );
}


void
use(int exitstatus)
{
   cerr << "\n\tuse" << endl
        <<"\t   kvbufrconf [--help|-h] [TYPE] [--template|-t templatefile] [--conf|-c confile] [--out|-o outfile] " << endl
        <<"\n\n"
        <<"\t TYPE may be one of\n"
        <<"\t\t--synop Generate a configuration file for SYNOP stations.\n"
        <<"\t\t--synop-wigos Generate a configuration file for SYNOP stations, but with wigos identifier.\n"
        <<"\t\t--svv Generate a configuration file for SVV stations.\n"
        <<"\t\t--precip Generate a configuration file for precipitations stations.\n"
        <<"\t\t--ship Generate a configuration file for SHIP stations.\n"
        <<"\t\t--bstations Generate a configuration file for bstations.\n"
        <<"\t\t  bstations use the same BUFR template as SVV stations.\n"
        <<"\t\t--mbuoy moored buoys. (NOT IMPLEMENTED)\n"
        <<"\n\t\tIf no TYPE is given the --synop option is assumed.\n"
        <<"\n";

   exit(exitstatus);
}
