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

#include <algorithm>
#include <float.h>
#include <iostream>
#include <fstream>
#include <miconfparser/miconfparser.h>
#include <miutil/trimstr.h>
#include <kvalobs/kvPath.h>
#include "Indent.h"
#include "ConfMaker.h"
#include "StationInfo.h"
#include "splitstr.h"
#include <StationInfoParse.h>

using namespace std;
using namespace kvalobs;

namespace {
   bool
   stationLessThan( StationInfoPtr s1, StationInfoPtr s2)
   {
      return s1->wmono() < s2->wmono();
   }

}


StationInfoPtr
ConfMaker::
findStation( int wmono )
{
   for( std::list<StationInfoPtr>::const_iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
      if( (*it)->wmono() == wmono )
         return *it;
   }

   for( std::list<StationInfoPtr>::const_iterator it=templateStationList.begin(); it!=templateStationList.end(); ++it ) {
      if( (*it)->wmono() == wmono ) {
         stationList.push_back( *it );
         return *it;
      }
   }

   return StationInfoPtr();
}

bool
ConfMaker::
parseTemplate( miutil::conf::ConfSection *templateConf )
{
   StationInfoParse stationInfoParser;

   stationList.clear();

   return stationInfoParser.parse( templateConf, templateStationList, false );
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
findSensor( const StInfoSysSensorInfoList &sensors, TblStInfoSysSensorInfo &sensor, int paramid )const
{
   for( StInfoSysSensorInfoList::const_iterator it=sensors.begin(); it != sensors.end(); ++it ) {
      if( it->paramid() == paramid && it->hlevel() == 0 && it->operational() ) {
         sensor = *it;
         return true;
      }
   }
   return false;
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

         if( end != string::npos && station->typepriority_.empty() ) {
            toParse << key << "=" << val << endl;
         }
      } else if( key == "stationid" && station->stationid_.empty()) {
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

   if( !station->delayList_.empty() )
      return true;

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

   toParse << "delay=" << o.str();

   miutil::conf::ConfParser parser;
   miutil::conf::ConfSection *result;
   miutil::conf::ValElementList valElement;
   StationInfoParse stationInfoParser;
   bool ok=true;
   result = parser.parse( toParse );

   if( ! result ) {
      cerr << "decodeCouplingDelay: Result (NULL): <" << parser.getError() << ">" << endl;
      return false;
   }

   valElement = result->getValue( "delay" );

   if( valElement.size() > 0 ) {
      if( ! stationInfoParser.doDelay( "delay", valElement, *station, false ) ) {
         LOGWARN( "Coupling delay: Failed to parse: <" << o.str() << ">" << endl );
         ok = false;
      }
   }

   delete result;
   return !station->delayList_.empty() && ok;
}

bool
ConfMaker::
decodePrecipPriority( const std::string &val_, StationInfoPtr station)
{
   string val( val_ );
   stringstream toParse;
   ostringstream o;
   vector<string> values;
   int nValues=0;
   miutil::trimstr( val );

   if( ! station->precipitation_.empty() )
      return false;

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

   toParse << "precipitation=" << o.str();

   miutil::conf::ConfParser parser;
   miutil::conf::ConfSection *result;
   miutil::conf::ValElementList valElement;
   StationInfoParse stationInfoParser;
   bool ok=true;
   result = parser.parse( toParse );

   if( ! result ) {
       cerr << "precipitationPriority: Result (NULL): <" << parser.getError() << ">" << endl;
       return false;
    }

    valElement = result->getValue( "precipitation" );

    if( valElement.size() > 0 ) {
       if( ! stationInfoParser.doPrecip( "precipitation", valElement, *station ) ) {
          LOGWARN( "precipitationPriority: Failed to parse: <" << o.str() << ">" << endl );
          ok = false;
       }
    }

    delete result;
    return !station->precipitation_.empty() && ok;

}

bool
ConfMaker::
decodeWindheight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station )
{
   const int FF( 81 );
   TblStInfoSysSensorInfo sensor;

   if( station->heightWind_ != INT_MAX )
      return false;

   if( ! findSensor( sensors, sensor,  FF ) ) {
      LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->stationID().begin()
                << " has no wind sensor.");
      return false;
   }

   if( sensor.physicalHeight() == INT_MAX )
      return false;

   station->heightWind_ = sensor.physicalHeight();
   return true;
}

bool
ConfMaker::
decodePrecipHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station )
{
   int RR[]={ 104, 106, 119, 107, 108, 120, 109, 125, 126, 110, 0};
   TblStInfoSysSensorInfo sensor;

   if( station->heightPrecip_ != INT_MAX )
       return false;

   for( int i=0; RR[i]; ++i ) {
      if(  findSensor( sensors, sensor,  RR[i] ) ) {
         if( sensor.physicalHeight() != INT_MAX ) {
            station->heightPrecip_ = sensor.physicalHeight();
            return true;
         }
      }
   }

   LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->stationID().begin()
             << " has no precipitation sensors.");


   return false;
}

bool
ConfMaker::
decodePressureHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station )
{
   int PP[]={ 173 , 171, 0};
   TblStInfoSysSensorInfo sensor;

   if( station->heightPressure_ != INT_MAX )
       return false;

   for( int i=0; PP[i]; ++i ) {
      if(  findSensor( sensors, sensor,  PP[i] ) ) {
         if( sensor.physicalHeight() != INT_MAX && station->height_ != INT_MAX) {
            station->heightPressure_ = sensor.physicalHeight() + station->height_;
            return true;
         }
      }
   }

   LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->stationID().begin()
             << " has no pressure sensors.");


   return false;
}

bool
ConfMaker::
decodeTemperatureHeight( const StInfoSysSensorInfoList &sensors, StationInfoPtr station )
{
   const int TA( 211 );
   TblStInfoSysSensorInfo sensor;

   if( station->heightTemperature_ != INT_MAX )
       return false;

   if( ! findSensor( sensors, sensor,  TA ) ) {
      LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->stationID().begin()
                << " has no temperature sensor.");
      return false;
   }

   if( sensor.physicalHeight() == INT_MAX )
      return false;


   station->heightTemperature_ = sensor.physicalHeight();

   return true;

}

std::string
ConfMaker::
stationIdToConfString( StationInfoPtr station )const
{
   ostringstream o;
   StationInfo::TLongList stations = station->stationID();

   if( stations.empty() )
      return "";

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

   if( types.empty() )
      return "";

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
precipPriorityToConfString( StationInfoPtr station )const
{
   ostringstream o;
   StationInfo::TStringList precip = station->precipitation();

   if( precip.empty() )
      return "";

   o << "precipitation=";
   o << "(";

   for(StationInfo::TStringList::iterator it=precip.begin();
         it!=precip.end(); it++)
   {
      if( it != precip.begin() )
         o << ",";

      o << "\"" << *it << "\"";
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
   string tmp;
   string precip = precipPriorityToConfString( station );

   o << "wmo_"<< station->wmono() << " {" << endl;
   indent.incrementLevel();

   tmp = station->name();
   if( ! tmp.empty() )
      o << indent.spaces() << "name=\"" << station->name() << "\"" << endl;

   if( station->height() != INT_MAX )
      o << indent.spaces() << "height=" << station->height() << endl;

   if( station->heightWind_ != INT_MAX )
      o << indent.spaces() << "height_wind=" << station->heightWind_ << endl;

   if( station->heightPrecip_ != INT_MAX )
      o << indent.spaces() << "height_precip=" << station->heightPrecip_ << endl;

   if( station->heightTemperature_ != INT_MAX )
         o << indent.spaces() << "height_temperature=" << station->heightTemperature_ << endl;

   if( station->heightPressure_ != INT_MAX )
      o << indent.spaces() << "height_pressure=" << station->heightPressure_ << endl;

   tmp = stationIdToConfString( station );
   if( !tmp.empty() )
      o << indent.spaces() << tmp << endl;

   tmp = typepriorityToConfString( station );
   if( ! tmp.empty() )
      o << indent.spaces() << tmp << endl;

   if( ! station->delayConf.empty() )
      o << indent.spaces() << "delay="  << station->delayConf << endl;

   if( !precip.empty() )
      o << indent.spaces() << precip << endl;

   if( station->isCopySetInConfSection() )
      o << indent.spaces() << "copy=" << (station->copy()?"true":"false") << endl;

   if( ! station->copyto_.empty() )
      o << indent.spaces() << "copyto=\"" << station->copyto() << "\"" << endl;

   if( ! station->owner_.empty() )
      o << indent.spaces() << "owner=\"" << station->owner() << "\"" << endl;

   if( ! station->list_.empty() )
      o << indent.spaces() << "list=\"" << station->list() << "\"" << endl;

   if( station->latitude() != FLT_MAX ) {
      o << indent.spaces() << "latitude=" << station->latitude() << endl;
      o << indent.spaces() << "longitude=" << station->longitude() << endl;
   }

   indent.decrementLevel();
   o << indent.spaces() << "}" << endl;
   return o.str();
}

bool
ConfMaker::
doConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
   StationInfoPtr pStation;
   StInfoSysStationOutmessageList tblWmoList;
   TblStInfoSysStation tblStation;
   TblStInfoSysNetworkStation networkStation;
   StInfoSysSensorInfoList tblSensors;
   int nValues;
   bool newStation;;

   if( templateConf ) {
      if( ! parseTemplate( templateConf ) ) {
         LOGFATAL("Failed to parse the template file.");
         return false;
      }
   }

   app.loadStationOutmessage( tblWmoList );

   for( StInfoSysStationOutmessageList::const_iterator it=tblWmoList.begin(); it != tblWmoList.end(); ++it ) {
      nValues = 0;
      if( ! app.loadStationData( it->stationid(), tblStation, tblSensors, networkStation ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }

      if( tblStation.wmono() == kvDbBase::INT_NULL ) {
         LOGWARN( "Station: " << it->stationid() << " Missising wmono.")
         continue;
      }

      pStation = findStation( tblStation.wmono() );

      if( ! pStation ) {
         newStation = true;
         pStation.reset( new StationInfo( tblStation.wmono()) );
         stationList.push_back( pStation );
      } else {
         newStation = false;
      }

      if( !networkStation.name().empty() ) {
         pStation->name( networkStation.name() );
         nValues++;
      }

      if( tblStation.hs() != INT_MAX ) {
         pStation->height( tblStation.hs() );
         nValues++;
      }

      if( decodeProductCoupling( it->productcoupling(), pStation  ) )
         nValues++;

      if( pStation->stationID().empty() ) {
         pStation->stationid_.push_back( it->stationid() );
         nValues++;
      }

      if( decodeWindheight( tblSensors, pStation ) )
         nValues++;

      if( decodePrecipHeight( tblSensors, pStation ) )
         nValues++;

      if( decodeTemperatureHeight( tblSensors, pStation ) )
         nValues++;

      if( decodePressureHeight( tblSensors, pStation ) )
         nValues++;

      if( decodeCouplingDelay( it->couplingDelay(), pStation ) )
         nValues++;

      if( decodePrecipPriority( it->priorityPrecip(), pStation ) )
         nValues++;

      if( tblStation.lat() != FLT_MAX && tblStation.lon() != FLT_MAX ) {
         pStation->latitude_ = tblStation.lat();
         pStation->longitude_ = tblStation.lon();
         nValues++;
      }

      if( nValues == 0  && !newStation ) {
         //There is no metadat for the station in stinfosys.
         LOGWARN("No useful metadat values for wmono: " << tblStation.wmono() << " was found in stinfosys."<< endl );
      }
   }

   stationList.sort( stationLessThan );



   if( outfile.empty() ) {
      for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
         cout << doStationConf( *it ) << endl;

      return true;
   }

   string filename( outfile );
   ofstream out;

   if( outfile[0]!='/' && outfile[0]!='.' )
      filename = kvPath( "sysconfdir" ) + "/" + outfile ;

   out.open( filename.c_str(), ios_base::out | ios_base::trunc );

   if( ! out.is_open() ) {
      LOGFATAL("Cant create file <" << filename << ">.");
      return false;
   }

   for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
      out << doStationConf( *it ) << endl;

   if( ! out.good() ) {
      LOGFATAL("ERROR while writing configuration to file <" << filename << ">.");
      return false;
   }

   out.close();
   return true;
}
