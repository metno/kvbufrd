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
#include <string.h>
#include <string>
#include <milog/milog.h>
#include <unistd.h>
#include "InitLogger.h"
#include <iostream>
#include <kvalobs/kvPath.h>

using namespace milog;
using namespace std;

namespace{
    LogLevel getLogLevel(const char *c);
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
   LogLevel     traceLevel=milog::DEBUG;
   LogLevel     logLevel=milog::INFO;
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
            traceLevel=getLogLevel( argv[srcI] );
            srcI++;

            if(traceLevel==milog::NOTSET){
               traceLevel=milog::DEBUG;
            }
         }
      }else if(strcmp("--loglevel", argv[srcI])==0){
         srcI++;

         if( srcI<argn ){
            logLevel=getLogLevel(argv[srcI]);
            srcI++;

            if(logLevel==milog::NOTSET){
               logLevel=milog::INFO;
            }
         }
      }

      //Remove the used args.
      compactArgv( argn, argv, destI, srcI );
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

   LogLevel getLogLevel(const char *str)
   {
      if(strcmp("FATAL", str)==0){
         return milog::FATAL;
      }else if(strcmp("ERROR", str)==0){
         return milog::ERROR;
      }else if(strcmp("WARN", str)==0){
         return milog::WARN;
      }else if(strcmp("DEBUG", str)==0){
         return milog::DEBUG;
      }else if(strcmp("DEBUG1", str)==0){
         return milog::DEBUG1;
      }else if(strcmp("DEBUG2", str)==0){
         return milog::DEBUG2;
      }else if(strcmp("DEBUG3", str)==0){
         return milog::DEBUG3;
      }else if(strcmp("DEBUG4", str)==0){
         return milog::DEBUG4;
      }else if(strcmp("DEBUG5", str)==0){
         return milog::DEBUG5;
      }else if(strcmp("DEBUG6", str)==0){
         return milog::DEBUG6;
      }else if(strcmp("INFO", str)==0){
         return milog::INFO;
      }else if(strcmp("0", str)==0){
         return milog::FATAL;
      }else if(strcmp("1", str)==0){
         return milog::ERROR;
      }else if(strcmp("2", str)==0){
         return milog::WARN;
      }else if(strcmp("3", str)==0){
         return milog::INFO;
      }else if(strcmp("4", str)==0){
         return milog::DEBUG;
      }else if(strcmp("5", str)==0){
         return milog::DEBUG1;
      }else if(strcmp("6", str)==0){
         return milog::DEBUG2;
      }else if(strcmp("7", str)==0){
         return milog::DEBUG3;
      }else if(strcmp("8", str)==0){
         return milog::DEBUG4;
      }else if(strcmp("9", str)==0){
         return milog::DEBUG5;
      }else if(strcmp("10", str)==0){
         return milog::DEBUG6;
      }else{
         return milog::NOTSET;
      }
   }
}
    
	       
