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
#include <iostream>
#include <sstream>
#include <puTools/miTime.h>
#include <miconfparser/miconfparser.h>
#include <kvalobs/kvPath.h>
#include "StationInfoParse.h"
#include "InitLogger.h"
#include "kvbufrconfcmpOptions.h"
#include "checkfile.h"

using namespace std;

struct StartStop {
   miutil::miTime start;
   int traceLevel;
   StartStop( int traceLevel ): traceLevel( traceLevel ) {
      start = miutil::miTime::nowTime();
      LOGINFO( "Starting at: " << start );

      if( traceLevel > 1 )
         cerr << "Starting at: " << start << endl;
   }
   ~StartStop() {
      miutil::miTime stop( miutil::miTime::nowTime() );
      int elapsedTime( miutil::miTime::secDiff( start, stop ) );
      LOGINFO( "Stoping at: " << stop << ". Elapsed time: " <<  elapsedTime << "s." );

      if( traceLevel > 1 )
         cerr << "Stoping at: " << stop << ". Elapsed time: " <<  elapsedTime << "s." << endl;
   }
};

void
checkOptions( Options &opt );

void
searchFileAndExitIfNotFound( const std::list<std::string> &paths,
                             const std::string &defaultName,
                             std::string &file,
                             const char *errmsg);

int
main( int argn, char **argv )
{
   Options opt;
   std::string logfile;
   ostringstream msg;
   miutil::conf::ConfSection *newconf;
   miutil::conf::ConfSection *oldconf;
   StationList newStationList;
   StationList oldStationList;
   StationInfoParse theParser;
   StationList tmpStations;

   InitLogger(argn, argv, "kvbufrconfcmp", logfile,  false );
   getOptions( argn, argv, opt );

   StartStop startStop( opt.verboseLevel );

   if( opt.verboseLevel > 0 )
      cerr << "Logging to file: " << logfile << endl;

   checkOptions( opt );

   msg << "New configuration file: " << opt.newconfile << endl
       << "Old configuration file: " << opt.oldconfile << endl;
   LOGINFO( msg.str() );

   if( opt.verboseLevel > 1 )
      cerr << msg.str();

   msg.str("");
   msg << "Accepting max removed station: " << opt.maxRemoved << endl
       << "Accepting max changed station: " << opt.maxChanged << endl;

   LOGINFO( msg.str() );

   if( opt.verboseLevel > 0 )
      cerr << msg.str();

   try{
      newconf=miutil::conf::ConfParser::parse( opt.newconfile );
   }
   catch( const logic_error &ex ){
      cerr << "Failed to parse new configuration file: " << opt.newconfile << " Reason: " << ex.what() << endl;
     LOGFATAL( "Failed to parse new configuration file: " << opt.newconfile << " Reason: " << ex.what() );
     return 1;
   }


   try{
      oldconf = miutil::conf::ConfParser::parse( opt.oldconfile );
   }
   catch( const logic_error &ex ){
      cerr << "Failed to parse old configuration file: " << opt.oldconfile << " Reason: " << ex.what() << endl;
      LOGFATAL( "Cant parse old configuration file '" << opt.oldconfile << "'. Reason: " << ex.what() );
      return 1;
   }

   if( !theParser.parse( oldconf, oldStationList, false ) ) {
      cerr << "Cant parse the station sections from the old configuration file: " << opt.oldconfile << endl;
      LOGFATAL( "Cant parse the station sections from the old configuration file: " << opt.oldconfile );
      return 1;
   }

   if( !theParser.parse( newconf, newStationList, false ) ) {
      cerr << "Cant parse the station sections from the new configuration file: " << opt.newconfile << endl;
      LOGFATAL( "Cant parse the station sections from the new configuration file: " << opt.newconfile );
      return 1;
   }

   StationInfoCompare conf = StationInfoCompare::compare( oldStationList, newStationList );
   ostringstream ost;

   tmpStations = conf.removedStations();
   ost << "Removed station(s): " << tmpStations.size();

   if( tmpStations.size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=tmpStations.begin(); it != tmpStations.end(); ++it ) {
         if( it != tmpStations.begin() )
            ost << ",";

         ost  << (*it)->wmono();
      }
      ost << ")";
   }

   ost << endl;

   tmpStations = conf.newStations();
   ost << "New station(s): " << tmpStations.size();

   if( tmpStations.size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=tmpStations.begin(); it != tmpStations.end(); ++it ) {
         if( it != tmpStations.begin() )
            ost << ",";

         ost << (*it)->wmono();
      }
      ost << ")";
   }
   ost << endl;

   tmpStations = conf.changedStations();
   ost << "Changed station(s): " << tmpStations.size();

   if( tmpStations.size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=tmpStations.begin(); it != tmpStations.end(); ++it ) {
         if( it != tmpStations.begin() )
            ost << ",";
         ost << (*it)->wmono();
      }
      ost << ")";
   }

   ost << endl;


   LOGINFO( ost.str() );

   if( opt.verboseLevel > 0 )
      cerr << ost.str() << endl;

   if( conf.removedStations().size() > opt.maxRemoved || conf.changedStations().size() > opt.maxChanged ) {
      LOGINFO( "To many changes between the new and old configuration.");
      cerr << "To many changes between the new and old configuration." << endl;
      return 2;
   }


   return 0;
}


void
searchFileAndExitIfNotFound( const std::list<std::string> &paths,
                             const std::string &defaultName,
                             std::string &file,
                             const char *errmsg)
{
   ostringstream logmsg;
   string filename;
   string error;
   list<string> dirs;

   if( file.empty() && defaultName.empty() ) {
      logmsg << "Missing file to search for!" << endl;
      LOGFATAL( logmsg.str() );

      cerr << logmsg.str() << endl;
      exit( 1 );
   }

   if( file.empty() || (file[0] != '/' && file[0] != '.') ) {
      for( std::list<std::string>::const_iterator it = paths.begin(); it != paths.end() && filename.empty(); ++it ) {
         filename = *it + "/";

         if( file.empty() )
            filename += defaultName;
         else
            filename += file;

         dirs.push_back( filename );

         if( !checkfile( filename, R_OK, error ) )
            filename.erase();
      }
   } else {
     filename = file;
     dirs.push_back( filename );

     if( !checkfile( filename, R_OK, error) )
        filename.erase();
   }

   if( filename.empty() ) {
      logmsg.str("");
      logmsg << errmsg << endl
             << "Searched this paths: " << endl;
      for( list<string>::iterator it=dirs.begin(); it != dirs.end(); ++it )
         logmsg << "   " << *it << endl;

      LOGFATAL( logmsg.str() );
      cerr << logmsg.str() << endl;

      exit( 1 );
   }

   file = filename;
}

void
checkOptions( Options &opt )
{
   list<string> paths;
   char cwd[1024];

   if( getcwd( cwd, 1024) )
      paths.push_back( cwd );

   paths.push_back( kvPath("sysconfdir") );

   searchFileAndExitIfNotFound( paths, "kvbufrd.conf", opt.oldconfile, "No readable old configuration file was found." );
   searchFileAndExitIfNotFound( paths, "", opt.newconfile, "No readable new configuration file was found." );
}
