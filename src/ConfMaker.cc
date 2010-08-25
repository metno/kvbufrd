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

//#include <kvalobs/kvDbBase.h>
#include <iostream>
#include <miconfparser/miconfparser.h>
#include <miutil/trimstr.h>
#include "Indent.h"
#include "ConfMaker.h"
#include "StationInfo.h"
#include "splitstr.h"
#include <StationInfoParse.h>

using namespace std;
using namespace kvalobs;

StationInfoPtr
ConfMaker::
findStation( int wmono )const
{
   for( std::list<StationInfoPtr>::const_iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
      if( (*it)->wmono() == wmono )
         return *it;
   }
   return StationInfoPtr();
}

   
ConfMaker::
ConfMaker( ConfApp &app_ )
   : app( app_ )
{
}

bool
ConfMaker::
setParams( const StInfoSysParamList &params_ )
{
   params = params_;
}

bool
ConfMaker::
add( int stationid, TblStInfoSysStation &station, StInfoSysSensorInfoList &sensors )
{
   bool newStation=false;

   if( station.wmono() == kvDbBase::INT_NULL )
      return true;

   StationInfoPtr stationInfo = findStation( station.wmono() );

   if( ! stationInfo ) {
      stationInfo.reset( new StationInfo( station.wmono()) );
      newStation = true;
   }
}
bool
ConfMaker::
decodeProductCoupling( const std::string &val_, StationInfoPtr station )
{
   vector<string> keyval;
   string key;
   string val;
   stringstream toParse;
   vector<string> line = miutil::splitstr(val_, '\n' );


   for( vector<string>::size_type i = 0; i < line.size(); ++i ) {
      keyval = miutil::splitstr( line[i], '=' );

      if( keyval.size()!=2 )
         continue;

      key = keyval[0];
      val = keyval[1];
      miutil::trimstr( key );
      miutil::trimstr( val );

      if( key.empty() || val.empty() )
         continue;

      if( !val.empty() && val[0] != '(')
         val.insert(0, "(");

      if( !val.empty() && val[val.length()-1] != ')' )
         val += ")";

      if( key == "typepriority" ) {
         string::size_type start=0;
         string::size_type end;

         while( start != string::npos ) {
            start = val.find_first_of("*", start );

            if( start == string::npos )
               continue;

            end = val.find_first_not_of("*0123456789", start );

            if( end == string::npos )
               break;

            val.insert( end, "\"");
            val.insert( start, "\"");
            start = end;
         }

         if( end != string::npos ) {
            toParse << key << "=" << val << endl;
         }
      } else if( key == "stationid") {
         toParse << key << "=" << val << endl;
      }
   }

   miutil::conf::ConfParser parser;
   miutil::conf::ConfSection *result;
   miutil::conf::ValElementList valElement;
   StationInfoParse stationInfoParser;
   bool error=false;
   result = parser.parse( toParse );

   if( ! result ) {
      cerr << "Result (NULL): <" << parser.getError() << ">" << endl;
      return false;
   }

   std::list<std::string> keys = result->getKeys();

   for(std::list<std::string>::iterator it = keys.begin(); it != keys.end(); ++it ) {
      valElement = result->getValue( *it );

      if( *it == "typepriority" ) {
         if( ! stationInfoParser.doTypePri( *it, valElement, *station ) ) {
            LOGWARN( "productcoupling: Failed to parse: <" << *it << ">" << endl );
            error = true;
         }
      } else if( *it == "stationid" ) {

         cerr << "ValElement: " << valElement << endl;
         if( ! stationInfoParser.doStationid( *it, valElement, *station ) ) {
            LOGWARN( "productcoupling: Failed to parse: <" << *it << ">" << endl );
            error = true;
         }


      }
   }

   delete result;
   return ! keys.empty() && ! error;
}

bool
ConfMaker::
decodeCouplingDelay( const std::string &val_, StationInfoPtr station)
{
   string val( val_ );
   stringstream toParse;
   ostringstream o;
   vector<string> values;
   int nValues=0;
   miutil::trimstr( val );


   //Remove ( and ) from start and end of the string, if any.
   if( !val.empty() && val[0]=='(' )
      val.erase( 0, 1);

   if( !val.empty() && val[val.length()-1]==')' )
      val.erase( val.length()-1, 1);

   values= miutil::splitstr( val, ',' );



   o << "(";

   for( vector<string>::size_type i = 0; i < values.size(); ++i ) {
      val = values[i];
      miutil::trimstr( val );

      if( val.empty() )
         continue;

      if( val[0] != '"' )
         val.insert( 0, "\"" );

      if( val[val.length()-1] != '"' )
         val += "\"";

      if( nValues>0 )
         o << ",";

      o << val;
      nValues++;
   }

   if( nValues == 0 )
      return true;

   o << ")" << endl;

   toParse << "delay=" << o.str() << endl;

   miutil::conf::ConfParser parser;
   miutil::conf::ConfSection *result;
   miutil::conf::ValElementList valElement;
   StationInfoParse stationInfoParser;
   bool error=false;
   result = parser.parse( toParse );

   if( ! result ) {
      cerr << "decodeCouplingDelay: Result (NULL): <" << parser.getError() << ">" << endl;
      return false;
   }

   valElement = result->getValue( "delay" );

   if( valElement.size() > 0 )
      station->delayConf = o.str();

   delete result;

   return true;
}

std::string
ConfMaker::
stationIdToConfString( StationInfoPtr station )const
{
   ostringstream o;
   StationInfo::TLongList stations = station->stationID();

   o << "stationid=";

   if( stations.size() == 1 ) {
      o << *stations.begin();
      return o.str();
   }

   o << "(";
   for(StationInfo::TLongList::iterator it=stations.begin();
       it!=stations.end(); it++)
   {
      if( it != stations.begin() )
         o << ",";

      o << *it;
   }

   o << ")";

   return o.str();
}

std::string
ConfMaker::
typepriorityToConfString( StationInfoPtr station )const
{
   ostringstream o;

   o << "typepriority=(";
   StationInfo::TLongList types = station->typepriority();

   for( StationInfo::TLongList::const_iterator it = types.begin(); it != types.end(); ++it ) {
      if( it != types.begin() )
         o << ",";

      if( station->mustHaveType( *it ) )
         o << "\"*" << *it << "\"";
      else
         o << *it;
   }

   o << ")";
   return o.str();
}


std::string
ConfMaker::
doStationConf( StationInfoPtr station )const
{
   ostringstream o;
   miutil::Indent indent;

   o << "wmo_"<< station->wmono() << " {" << endl;
   indent.incrementLevel();
   o << indent.spaces() << stationIdToConfString( station ) << endl;
   o << indent.spaces() << typepriorityToConfString( station ) << endl;

   if( ! station->delayConf.empty() )
      o << indent.spaces() << "delay="  << station->delayConf << endl;

   indent.decrementLevel();
   o << indent.spaces() << "}" << endl;
   return o.str();
}

bool
ConfMaker::
doConf()
{

   bool newStation;
   StationInfoPtr pStation;
   StInfoSysStationOutmessageList tblWmoList;
   TblStInfoSysStation tblStation;
   StInfoSysSensorInfoList tblSensors;

   app.loadStationOutmessage( tblWmoList );

   for( StInfoSysStationOutmessageList::const_iterator it=tblWmoList.begin(); it != tblWmoList.end(); ++it ) {
      if( ! app.loadStationData( it->stationid(), tblStation, tblSensors ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }

      if( tblStation.wmono() == kvDbBase::INT_NULL ) {
         LOGWARN( "Station: " << it->stationid() << " Missising wmono.")
         continue;
      }

      newStation = false;
      pStation = findStation( tblStation.wmono() );

      if( ! pStation ) {
         newStation = true;
         pStation.reset( new StationInfo( tblStation.wmono()) );
         stationList.push_back( pStation );
      }

      decodeProductCoupling( it->productcoupling(), pStation  );
      decodeCouplingDelay( it->couplingDelay(), pStation );

      if( pStation->stationID().empty() )
         pStation->stationid_.push_back( it->stationid() );

      continue;
      cerr << it->stationid() << ", " << tblStation.wmono() << ", " << tblStation.name() << ", " << it->couplingDelay() << ", " << it->productcoupling() << ", " << it->priorityPrecip() << endl;
      cerr << "SENSORS:";

      for( StInfoSysSensorInfoList::const_iterator sit=tblSensors.begin(); sit != tblSensors.end(); ++sit )
         cerr << " " << sit->paramid()<< "(" << sit->physicalHeight() << ")";

      cerr << endl;
   }

   for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
      cerr << doStationConf( *it ) << endl;

   return true;
}
