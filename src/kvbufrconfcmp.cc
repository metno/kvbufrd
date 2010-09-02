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

#include <miconfparser/miconfparser.h>
#include "StationInfoParse.h"
#include "InitLogger.h"
#include "kvbufrconfcmpOptions.h"


using namespace std;


int
main( int argn, char **argv )
{
   Options opt;
   std::string confFile;
   miutil::conf::ConfSection *newconf;
   miutil::conf::ConfSection *oldconf;
   StationList newStationList;
   StationList oldStationList;
   StationInfoParse theParser;

   InitLogger(argn, argv, "kvbufrconfcmp");

   getOptions( argn, argv, opt );

   try{
      newconf=miutil::conf::ConfParser::parse( opt.newconfile );
   }
   catch( const logic_error &ex ){
     // LOGFATAL( ex.what() );
      return 1;
   }

   if( !opt.oldconfile.empty() ) {
      try{
         oldconf = miutil::conf::ConfParser::parse( opt.oldconfile );
      }
      catch( const logic_error &ex ){
   //      LOGFATAL( "Cant parse templatefile '" << opt.oldconfile << "'. Reason: " << ex.what() );
         return 1;
      }
   }

   if( !theParser.parse( oldconf, oldStationList ) ) {
      cerr << "Failed to parse the file : " << opt.oldconfile << endl;
      return 1;
   }

   if( !theParser.parse( newconf, newStationList ) ) {
      cerr << "Failed to parse the file : " << opt.newconfile << endl;
      return 1;
   }

   StationInfoCompare conf = StationInfoCompare::compare( oldStationList, newStationList );
   ostringstream ost;

   ost << "Removed station(s): " << conf.removedStations().size();

   if( conf.removedStations().size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=conf.removedStations().begin(); it != conf.removedStations().end(); ++it ) {
         ost << "," << (*it)->wmono();
      }
      ost << ")";
   }

   ost << endl;

   ost << "New station(s): " << conf.newStations().size();

   if( conf.newStations().size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=conf.newStations().begin(); it != conf.newStations().end(); ++it ) {
         ost << "," << (*it)->wmono();
      }
      ost << ")";
   }

   ost << endl;

   ost << "Changed station(s): " << conf.changedStations().size();

   if( conf.changedStations().size() > 0 ) {
      ost << " (";
      for( StationList::iterator it=conf.changedStations().begin(); it != conf.changedStations().end(); ++it ) {
         ost << "," << (*it)->wmono();
      }
      ost << ")";
   }

   ost << endl;

   cerr << ost.str() << endl;

   return 0;
}

