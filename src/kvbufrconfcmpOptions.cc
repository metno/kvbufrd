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

#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "checkfile.h"
#include "kvbufrconfcmpOptions.h"

using namespace std;

void
getOptions(int argn, char **argv, Options &opt)
{
    struct option long_options[]={{"help", 0, 0, 'h'},
                                  {"newconf", 1, 0, 'n'},
                                  {"oldconf", 1, 0, 'o'},
                                  {"verbose", 2, 0, 'v'},
                                  {"maxchanged", 1, 0, 'c'},
                                  {"maxremoved", 1, 0, 'r'},
                                  {0,0,0,0}};

    int c;
    int index;
    string error;
    string confileAtCommandLine;
    int prevOption;

    while(true){
        c=getopt_long(argn, argv, "hn:o:v::c:r:", long_options, &index);

        if(c==-1)
            break;

        switch(c){
        case 'h':
           use(0);
           break;

        case 'c':
            opt.maxChanged= atoi( optarg );
            break;
        case 'r':
           opt.maxRemoved = atoi( optarg );
           break;
        case 'n':
           opt.newconfile=optarg;
           break;
        case 'o':
           opt.oldconfile=optarg;
           break;
        case 'v':
           if( optarg )
              opt.verboseLevel=atoi( optarg );
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
            cerr << "?? option charcter: <" << (char)optopt << "> unknown!" << endl;
            use(1);
        }
    }

    if( opt.newconfile.empty() ) {
       cerr << "Missing the newconfiguration file!" << endl;
       use(1);
    }
}


void
use(int exitstatus)
{
  cerr << "\n\tuse" << endl
       <<"\tkvbufrconfcmp [--help|-h] --newconf|-n newconfile] --oldconf|-o oldconfile] [--verbose|-v level] \n"
       <<"\t    [--maxremoved|-r value] [--maxchanged|-c value]" << endl;

  exit(exitstatus);
}
