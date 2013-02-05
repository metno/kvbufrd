/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: InitLogger.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "InitLogger.h"
#include "parseMilogLogLevel.h"

using namespace milog;
using namespace std;

namespace{
    void compactArgv( int &argn, char **argv, int dest, int src );
}

void
InitLogger(int &argn, char **argv, const std::string &logname, bool doTraceToScreen )
{
   std::string dummy;
   InitLogger( argn, argv, logname, dummy, doTraceToScreen );
}


void
InitLogger(int &argn, char **argv, const std::string &logname, std::string &logfile, bool doTraceToScreen )
{
   string       filename;
   LogLevel     traceLevel=milog::NOTSET;
   LogLevel     logLevel=milog::NOTSET;
   FLogStream   *fs;
   StdErrStream *trace;
   int destI;
   int srcI;

   filename = kvPath("logdir") + "/" + logname +".log";

   for(int i=0; i<argn; i++){
      destI = i;
      srcI = i;
      if(strcmp("--tracelevel", argv[srcI])==0){
         srcI++;

         if( srcI < argn ){
            traceLevel=parseMilogLogLevel( argv[srcI] );
            srcI++;

            if(traceLevel==milog::NOTSET){
               traceLevel=milog::DEBUG;
            }
         }
      }else if(strcmp("--loglevel", argv[srcI])==0){
         srcI++;

         if( srcI<argn ){
            logLevel=parseMilogLogLevel( argv[srcI] );
            srcI++;

            if(logLevel==milog::NOTSET){
               logLevel=milog::INFO;
            }
         }
      }

      //Remove the used args.
      compactArgv( argn, argv, destI, srcI );
   }

   if( traceLevel == milog::NOTSET && logLevel == milog::NOTSET ) {
      traceLevel=milog::INFO;
      logLevel=milog::INFO;
   } else if( traceLevel == milog::NOTSET ) {
      traceLevel = logLevel;
   } else {
      logLevel = traceLevel;
   }


   try{
      fs=new FLogStream(4);

      if( !fs->open( filename ) ) {
         std::cerr << "FATAL: Can't initialize the Logging system.\n";
         std::cerr << "------ Cant open the Logfile <" << filename << ">\n";
         delete fs;
         exit(1);
      }

      if( !LogManager::createLogger( logname, fs) ) {
         std::cerr << "FATAL: Can't initialize the Logging system.\n";
         std::cerr << "------ Cant create logger\n";
         exit(1);
      }

      fs->loglevel(logLevel);

      if( doTraceToScreen ) {
         trace=new StdErrStream();

         if(!LogManager::addStream( logname, trace ) ) {
            std::cerr << "FATAL: Can't initialize the Logging system.\n";
            std::cerr << "------ Cant add filelogging to the Logging system\n";
            exit(1);
         }

         trace->loglevel(traceLevel);
      }

      LogManager::setDefaultLogger( logname );
   }
   catch(...){
      std::cerr << "FATAL: Can't initialize the Logging system.\n";
      std::cerr << "------ OUT OF MEMMORY!!!\n";
      exit(1);
   }

   logfile = filename;
   LOGINFO( "Logging to file <" << filename << ">!" );
}

namespace{
   void compactArgv( int &argn, char **argv, int dest, int src  )
   {
      int n = src-dest;

      if( n<=0 )
         return;

      int i, ii;
      for( i=dest, ii=dest+n; ii<argn; i++, ii++ )
         argv[i] = argv[ii];

      argn -= n;
   }
}
    
	       
