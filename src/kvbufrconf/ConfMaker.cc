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
#include <math.h>
#include <iostream>
#include <fstream>
#include <boost/assign/list_inserter.hpp>
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
      return *s1 < *s2;
   }
}


StationInfoPtr
ConfMaker::
findStation( int wmono, int stationid, const std::string &callsign,
             const std::list<int> &codeList, bool &newStation )
{
   newStation = false;

   for( std::list<StationInfoPtr>::const_iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
      if( (*it)->wmono() == wmono && (*it)->stationID() == stationid &&
           (*it)->callsign() == callsign && (*it)->code() == codeList ) {
         return *it;
      }
   }

   for( std::list<StationInfoPtr>::const_iterator it=templateStationList.begin(); it!=templateStationList.end(); ++it ) {
      if( (*it)->wmono() == wmono && (*it)->stationID() == stationid &&
          (*it)->callsign() == callsign && (*it)->code() == codeList) {
         StationInfoPtr p( new StationInfo( **it ) );
         stationList.push_back( p );
         return p;
      }
   }

   newStation = true;

   StationInfoPtr ptr;

   if( defaultVal ) {
      ptr.reset( new StationInfo( *defaultVal ) );
   } else {
      ptr.reset( new StationInfo() );
   }

   ptr->stationID_ = stationid;
   ptr->wmono_ = wmono;
   ptr->callsign_ = callsign;

   stationList.push_back( ptr );

   return ptr;
}

void
ConfMaker::
removeStation(StationInfoPtr station )
{
	for( std::list<StationInfoPtr>::iterator it=stationList.begin();
		 it!=stationList.end(); ++it )
	{
	      if( *it == station ) {
	    	  stationList.erase( it );
	    	  return;
	      }
	}
}

bool
ConfMaker::
parseTemplate( miutil::conf::ConfSection *templateConf )
{
   StationInfoParse stationInfoParser( true );

   stationList.clear();

   if( stationInfoParser.parse( templateConf, templateStationList, true ) ) {
      defaultVal = stationInfoParser.defaultVal();
      return true;
   } else {
      return false;
   }
}
   
ConfMaker::
ConfMaker( ConfApp &app_ )
   : app( app_ )
{
}

void
ConfMaker::
setParams( const StInfoSysParamList &params_ )
{
   params = params_;
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
   vector<string> line = miutil::splitstr(val_, ';' );

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
      } else if( key == "stationid" && station->definedStationid_.empty()) {
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
      LOGERROR("INTERNAL ERRORT: decodeProductCoupling: failed to parse '" << toParse.str() << ">. Result (NULL): <" << parser.getError() << ">.");
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
         if( ! stationInfoParser.doStationid( *it, valElement, *station ) ) {
            LOGWARN( "productcoupling: Failed to parse: <" << *it << ">. (ValElement: " << valElement << ")." );
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

   if( val_.empty() ) {
       if( !station->delayList_.empty() ) return true;
       else return false;
   }

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

   o << ")";

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
      if( ! stationInfoParser.doDelay( "delay", valElement, *station, result, false ) ) {
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
      LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->definedStationID().begin()
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

   LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->definedStationID().begin()
             << " has no precipitation sensors.");


   return false;
}

bool
ConfMaker::
decodePressureHeight( const StInfoSysSensorInfoList &sensors,
		              const TblStInfoSysStation &tblStation, StationInfoPtr station )
{
   int PP[]={ 173 , 171, 0};
   TblStInfoSysSensorInfo sensor;

   if( station->heightPressure_ != FLT_MAX )
       return false;

   if( tblStation.hp() == kvDbBase::FLT_NULL )
	   return false;

   for( int i=0; PP[i]; ++i ) {
      if(  findSensor( sensors, sensor,  PP[i] ) ) {
    	  station->heightPressure_ = tblStation.hp();
    	  return true;
      }
   }

   LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->definedStationID().begin()
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
      LOGDEBUG( "Station '" << station->name() << "' stationid: " << *station->definedStationID().begin()
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
   StationInfo::TLongList stations = station->definedStationID();

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

bool
ConfMaker::
precipConfFromParams( StationInfoPtr station,
		              const std::list<int> &precipParams,
		              int stationid, int typeid_ )
{
	StInfoSysObsObsPgmHList precip;
	string sprecip;
	//If we all ready have a precipitation, from a template, we just return
	if( station->hasPrecipitation() )
		return true;

	if( ! app.hasObsPgmHParamsids( precip, stationid, typeid_, precipParams) )
		return false;

	if( precip.empty() )
		return false;

	switch( precip.begin()->paramid() ) {
	case RA: sprecip="RA";break;
	case RR_1:sprecip="RR_1";break;
	case RR_3:sprecip="RR_3";break;
	case RR_6:sprecip="RR_6";break;
	case RR_12:sprecip="RR_12";break;
	case RR_24:sprecip="RR_24";break;
	default:
		break;
	}
	if( !sprecip.empty()) {
		station->precipitation_.push_back( sprecip );
		return true;
	} else {
		return false;
	}
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

   if( station->ignoreThisStation() )
       o << "!";

   if( station->wmono() > 0 )
      o << "wmo_"<< station->wmono() << " {" << endl;
   else if( ! station->callsign().empty() )
	   o << "callsign_"<< station->callsign() << " {" << endl;
   else
      o << "id_"<< station->stationID() << " {" << endl;

   indent.incrementLevel();

   if( ! station->code_.empty( ) ) {
      StationInfo::IntList::const_iterator it=station->code_.begin();
      o << indent.spaces() << "code=(" << *it;
      ++it;
      for( ; it!=station->code_.end(); ++it  )
         o << "," << *it;
      o << ")" << endl;
   }

   tmp = station->name();
   if( ! tmp.empty() )
      o << indent.spaces() << "name=\"" << station->name() << "\"" << endl;

   if( station->height() != INT_MAX )
      o << indent.spaces() << "height=" << station->height() << endl;

   if( station->heightWind_ != INT_MAX )
      o << indent.spaces() << "height_wind=" << station->heightWind_ << endl;

   if( station->heightWindAboveSea_ != INT_MAX )
       o << indent.spaces() << "height_wind_above_sea=" << station->heightWindAboveSea_ << endl;

   if( station->heightPrecip_ != INT_MAX )
      o << indent.spaces() << "height_precip=" << station->heightPrecip_ << endl;

   if( station->heightTemperature_ != INT_MAX )
         o << indent.spaces() << "height_temperature=" << station->heightTemperature_ << endl;

   if( station->heightPressure_ != FLT_MAX ) {
	   ios_base::fmtflags oldf=o.flags();
	   streamsize oldp=o.precision();

	   o.setf( ios::fixed, ios::floatfield );
	   o.precision( 1 );
	   o << indent.spaces() << "height_pressure=" << station->heightPressure_ << endl;
	   o.flags( oldf );
	   o.precision( oldp );
   }

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
doSVVConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
   StationInfoPtr pStation;
   StInfoSysStationOutmessageList tblWmoList;
   TblStInfoSysStation tblStation;
   StInfoSysNetworkStationList networkStations;
   StInfoSysSensorInfoList tblSensors;
   list<int> codeList;
   int nValues;
   bool newStation;;
   int wmono;

   if( templateConf ) {
      if( ! parseTemplate( templateConf ) ) {
         LOGFATAL("Failed to parse the template file.");
         return false;
      }
   }

   std::list<int> networksids;

   boost::assign::push_back( networksids)(33);
   boost::assign::push_back( codeList )( 1 );//BUFR template for SVV

   app.loadNetworkStation( networkStations, networksids );

   for( StInfoSysNetworkStationList::iterator it=networkStations.begin();
         it != networkStations.end(); ++it ) {
      nValues = 0;
      if( ! app.loadStationData( it->stationid(), tblStation, tblSensors ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }


      if( tblStation.wmono() == kvDbBase::INT_NULL )
         wmono = 0;
      else
         wmono = tblStation.wmono();

      pStation = findStation( wmono, tblStation.stationid(), "", codeList, newStation );

      if( ! it->name().empty() ) {
         pStation->name( it->name() );
         nValues++;
      }


      if( pStation->code_.empty() ) {
         pStation->code_ = codeList;
      }

      if( tblStation.hs() != INT_MAX ) {
         pStation->height( tblStation.hs() );
         nValues++;
      }

      if( decodeProductCoupling( "typepriority=505", pStation  ) )
         nValues++;

      //TODO: At the moment we hardcode the 'owner' and 'list'
      //values. Ideal this should come from stinfosys.

      if( pStation->owner().empty() )
         pStation->owner("SVVS");

      if( pStation->list().empty() ) {
         pStation->list( "99" );
      }

      if( pStation->definedStationID().empty() ) {
         pStation->definedStationid_.push_back( it->stationid() );
         nValues++;
      }

      if( decodeWindheight( tblSensors, pStation ) )
         nValues++;

      if( decodePrecipHeight( tblSensors, pStation ) )
         nValues++;

      if( decodeTemperatureHeight( tblSensors, pStation ) )
         nValues++;

      if( decodePressureHeight( tblSensors, tblStation, pStation ) )
         nValues++;

      if( decodeCouplingDelay( "+HH:00", pStation ) )
         nValues++;
//
//      if( decodePrecipPriority( it->priorityPrecip(), pStation ) )
//         nValues++;

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

   if( outfile.empty() ) {
      for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
         cout << doStationConf( *it ) << endl;

      return true;
   }

   stationList.sort( stationLessThan );

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

bool
ConfMaker::
doPrecipConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
   StInfoSysObsObsPgmHList obspgm;
   StationInfoPtr pStation;
   TblStInfoSysStation tblStation;
   StInfoSysNetworkStationList networkStations;
   StInfoSysSensorInfoList tblSensors;
   int nValues;
   bool newStation;;
   int wmono;
   list<int> codeList;

   if( templateConf ) {
      if( ! parseTemplate( templateConf ) ) {
         LOGFATAL("Failed to parse the template file.");
         return false;
      }
   }


   std::list<int> typeids;

   boost::assign::push_back( typeids)(302)(304);
   boost::assign::push_back( codeList)(2);  //BUFR template for PRECIP stations.

   app.loadObsPgmH( obspgm, typeids );

   for( StInfoSysObsObsPgmHList::iterator it=obspgm.begin();
         it != obspgm.end(); ++it ) {
      cout << *it << endl;
//      nValues = 0;
      if( ! app.loadStationData( it->stationid(), tblStation, tblSensors ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }


      if( tblStation.wmono() == kvDbBase::INT_NULL )
         wmono = 0;
      else
         wmono = tblStation.wmono();

      pStation = findStation( wmono, tblStation.stationid(), "", codeList, newStation );

      if( ! tblStation.name().empty() ) {
         pStation->name( tblStation.name() );
         nValues++;
      }


      if( pStation->code_.empty() ) {
         pStation->code_ = codeList;
      }

      if( tblStation.hs() != INT_MAX ) {
         pStation->height( tblStation.hs() );
         nValues++;
      }

      ostringstream otp;
      otp  << "typepriority="  << it->messageFormatid();
      if( decodeProductCoupling( otp.str(), pStation  ) )
         nValues++;

      //TODO: At the moment we hardcode the 'owner' and 'list'
      //values. Ideal this should come from stinfosys.

      if( pStation->owner().empty() ) {
         if( it->messageFormatid() == 302 )
            pStation->owner("NEDB");
         else if( it->messageFormatid() == 304 )
            pStation->owner("SKRD");
      }

      if( pStation->list().empty() ) {
         pStation->list( "99" );
      }

      if( pStation->definedStationID().empty() ) {
         pStation->definedStationid_.push_back( it->stationid() );
         nValues++;
      }

      if( decodeWindheight( tblSensors, pStation ) )
         nValues++;

      if( decodePrecipHeight( tblSensors, pStation ) )
         nValues++;

      if( decodeTemperatureHeight( tblSensors, pStation ) )
         nValues++;

      if( decodePressureHeight( tblSensors, tblStation, pStation ) )
         nValues++;

//      if( decodeCouplingDelay( "+HH:00", pStation ) )
//         nValues++;
//
//      if( decodePrecipPriority( it->priorityPrecip(), pStation ) )
//         nValues++;

      if( tblStation.lat() != FLT_MAX && tblStation.lon() != FLT_MAX ) {
         pStation->latitude_ = tblStation.lat();
         pStation->longitude_ = tblStation.lon();
         nValues++;
      }

      if( nValues == 0  && !newStation ) {
         //There is no metadat for the station in stinfosys.
         LOGWARN("No useful metadata values for wmono: " << tblStation.wmono() << " was found in stinfosys."<< endl );
      }

   }


   if( outfile.empty() ) {
      for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
         cout << doStationConf( *it ) << endl;

      return true;
   }

   stationList.sort( stationLessThan );

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


//void
//ConfMaker::
//doPlatformsAsShip(miutil::conf::ConfSection *templateConf )
//{
//
//}
//

void
ConfMaker::
setShipOwner( StationInfoPtr station,
		      const TblStInfoSysStation &infoSysStation,
		      StInfoSysNetworkStationList::iterator itNetworkStation )
{
	if( station->owner().empty() ) {
		string theName=boost::algorithm::trim_copy( itNetworkStation->name() );
		string::size_type ii=theName.find("KV");
		if( ii != string::npos && ii == 0)
			station->owner( "KYST" );
		else if( ! station->callsign().empty() && app.isPlatform( station->stationID() ) )
			station->owner( "PLAT" );
		else
			station->owner( "SHIP" );
	}
}

bool
ConfMaker::
setShipProductCouplingAndDelay( StationInfoPtr station )
{
	int ret=0;
	if( station->owner() != "PLAT" ) {
		if( decodeProductCoupling( "typepriority=11", station  ) )
			ret++;

		if( decodeCouplingDelay( "+HH:00", station ) )
			ret++;
	} else {
		std::list<int> reqTypeids;
		std::list<int> typeids;
		boost::assign::push_back( reqTypeids)(11)(22);

		typeids = app.hasParamDefForTypeids( station->stationID(), reqTypeids );

		if( typeids.size() == 2 ) {
			if( decodeProductCoupling( "typepriority=(11,22)", station  ) )
				ret++;
			if( decodeCouplingDelay( "(\"+HH:00\",\"HH:17\")", station ) )
				ret++;
		} else if( typeids.size() == 1 ) {
			ostringstream o;
			o << "typepriority=" << *typeids.begin() ;
			if( decodeProductCoupling( o.str(), station  ) )
				ret++;
			if( decodeCouplingDelay( "+HH:00", station ) )
				ret++;
		}
	}

	return ret != 0;
}


bool
ConfMaker::
doShipConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
	StationInfoPtr pStation;
	StInfoSysStationOutmessageList tblWmoList;
	TblStInfoSysStation tblStation;
	StInfoSysNetworkStationList networkStations;
	StInfoSysSensorInfoList tblSensors;
	list<int> codeList;
	int nValues;
	bool newStation;;
	string callsign;

	if( templateConf ) {
		if( ! parseTemplate( templateConf ) ) {
			LOGFATAL("Failed to parse the template file.");
			return false;
		}
	}

	std::list<int> networksids;

	boost::assign::push_back( networksids)(6);
	boost::assign::push_back( codeList )( 3 );//BUFR template for SHIP

	app.loadNetworkStation( networkStations, networksids );

	for( StInfoSysNetworkStationList::iterator it=networkStations.begin();
			it != networkStations.end(); ++it ) {
		nValues = 0;
		if( ! app.loadStationData( it->stationid(), tblStation, tblSensors ) ) {
			LOGINFO( "No metadata for station <" << it->stationid() << ">.");
			continue;
		}

		if( it->externalStationcode().empty() )
			continue;

		callsign = it->externalStationcode();

		pStation = findStation( 0, tblStation.stationid(), callsign, codeList, newStation );

		setShipOwner( pStation, tblStation, it );

		if( ! setShipProductCouplingAndDelay( pStation ) ) {
			removeStation( pStation );
			continue;
		}

		nValues++;
		if( ! tblStation.name().empty() ) {
			pStation->name( tblStation.name() );
			nValues++;
		}

		if( pStation->code_.empty() ) {
			pStation->code_ = codeList;
		}

		if( tblStation.hs() != INT_MAX ) {
			pStation->height( tblStation.hs() );
			nValues++;
		}

		if( pStation->list().empty() ) {
			pStation->list( "99" );
		}

		if( pStation->definedStationID().empty() ) {
			pStation->definedStationid_.push_back( it->stationid() );
			nValues++;
		}


		if( pStation->owner() == "PLAT" &&
			pStation->height() != INT_MAX && pStation->height() > 0  &&
		    pStation->heightWindAboveSea_ == INT_MAX ) {
		        pStation->heightWindAboveSea_ = 10;
		        pStation->heightWind_ = INT_MAX;
		        nValues++;
		} else if( decodeWindheight( tblSensors, pStation ) ) {
			nValues++;
		}

		if( decodePrecipHeight( tblSensors, pStation ) )
			nValues++;

		if( decodeTemperatureHeight( tblSensors, pStation ) )
			nValues++;

		if( decodePressureHeight( tblSensors, tblStation, pStation ) )
			nValues++;

		if( tblStation.lat() != FLT_MAX && tblStation.lon() != FLT_MAX ) {
			pStation->latitude_ = tblStation.lat();
			pStation->longitude_ = tblStation.lon();
			nValues++;
		}

		if( nValues == 0  && !newStation ) {
			//There is no metadat for the station in stinfosys.
			LOGWARN("No useful metadat values for callsign: " << callsign << " was found in stinfosys."<< endl );
		}
	}

	if( outfile.empty() ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
			cout << doStationConf( *it ) << endl;

		return true;
	}

	stationList.sort( stationLessThan );

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


bool
ConfMaker::
doBStationsConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
	ostringstream ost;
	StationInfoPtr pStation;
	TblStInfoSysStation tblStation;
	StInfoSysSensorInfoList tblSensors;
	StInfoSysStationList tblBStations;
	StInfoSysObsObsPgmHList obspgm;
	list<int> params;
	list<int> precipParams;
	list<int> codeList;
	int nValues;
	bool newStation;;
	int wmono;
	int stationid;
	int typeid_;

	if( templateConf ) {
		if( ! parseTemplate( templateConf ) ) {
			LOGFATAL("Failed to parse the template file.");
			return false;
		}
	}

	std::list<int> networksids;

	boost::assign::push_back( networksids)(33);
	boost::assign::push_back( codeList )( 1 );//BUFR template for SVV
	boost::assign::push_back( params )( 211 )(81); // TA (211), FF (81) used to find the type ids.
	boost::assign::push_back( precipParams )(RR_1)(RR_3)(RR_6)(RR_12)(RR_24)
				(RR_X)(RR_2)(RR_9)(RR_15)(RR_18);

	if( ! app.findBStations(  tblBStations ) )
		return false;

	for( StInfoSysStationList::iterator it=tblBStations.begin();
			it != tblBStations.end(); ++it ) {
		nValues = 0;

		stationid = it->stationid();
		if( ! app.findObsPgmHTypeids( obspgm, stationid, params ) )
			return false;

		if( obspgm.empty() )
			continue;

		//If multiple typeid is found, we use just the first.
		typeid_ = obspgm.begin()->messageFormatid();

//		{ //DEBUG
//			cerr << "stationid: " << it->stationid();
//			for( StInfoSysObsObsPgmHList::iterator pit = obspgm.begin(); pit != obspgm.end(); ++pit )
//				cerr << " " << pit->messageFormatid();
//			cerr << endl;
//			continue;
//		}

		if( ! app.loadStationData( stationid, tblStation, tblSensors ) ) {
			LOGINFO( "No metadata for station <" << it->stationid() << ">.");
			continue;
		}


		if( tblStation.wmono() == kvDbBase::INT_NULL )
			wmono = 0;
		else
			wmono = tblStation.wmono();

		pStation = findStation( wmono, stationid, "", codeList, newStation );

		if( ! it->name().empty() ) {
			pStation->name( it->name() );
			nValues++;
		}


		if( pStation->code_.empty() ) {
			pStation->code_ = codeList;
		}

		if( tblStation.hs() != INT_MAX ) {
			pStation->height( tblStation.hs() );
			nValues++;
		}

		ost.str("");
		ost << "typepriority=(" << typeid_ << ")";
//		for( StInfoSysObsObsPgmHList::iterator oit = obspgm.begin(); oit != obspgm.end(); ++oit ) {
//			if( oit != obspgm.begin() )
//				ost << ",";
//
//			ost << oit->messageFormatid();
//		}
//		ost << ")";

		if( decodeProductCoupling( ost.str(), pStation  ) )
			nValues++;

		//TODO: At the moment we hardcode the 'owner' and 'list'
		//values. Ideal this should come from stinfosys.

		if( pStation->owner().empty() )
			pStation->owner("BSTA");

		if( pStation->list().empty() ) {
			pStation->list( "99" );
		}

		if( pStation->definedStationID().empty() ) {
			pStation->definedStationid_.push_back( stationid );
			nValues++;
		}

		if( decodeWindheight( tblSensors, pStation ) )
			nValues++;

		if(  precipConfFromParams( pStation, precipParams, stationid, typeid_ ) )
			nValues++;

		if( decodePrecipHeight( tblSensors, pStation ) )
			nValues++;

		if( decodeTemperatureHeight( tblSensors, pStation ) )
			nValues++;

		if( decodePressureHeight( tblSensors, tblStation, pStation ) )
			nValues++;

		if( decodeCouplingDelay( "+HH:00", pStation ) )
			nValues++;
		//
		//      if( decodePrecipPriority( it->priorityPrecip(), pStation ) )
		//         nValues++;

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

	if( outfile.empty() ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
			cout << doStationConf( *it ) << endl;

		return true;
	}

	stationList.sort( stationLessThan );

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




bool
ConfMaker::
doConf( const std::string &outfile, miutil::conf::ConfSection *templateConf )
{
   StationInfoPtr pStation;
   StInfoSysStationOutmessageList tblWmoList;
   TblStInfoSysStation tblStation;
   TblStInfoSysNetworkStation networkStation;
   StInfoSysSensorInfoList tblSensors;
   list<int> codeList;
   int nValues;
   bool newStation;;

   if( templateConf ) {
      if( ! parseTemplate( templateConf ) ) {
         LOGFATAL("Failed to parse the template file.");
         return false;
      }
   }

   //TODO: Until stinfosys contains information about which
   //BUFR codes to use we hardcode it here to 0 (SYNOP)
   codeList.push_back(0);

   app.loadStationOutmessage( tblWmoList );

   for( StInfoSysStationOutmessageList::const_iterator it=tblWmoList.begin(); it != tblWmoList.end(); ++it ) {
      nValues = 0;


      if( app.isPlatformOrShip( it->stationid() ) )
    	  continue;

      if( ! app.loadStationData( it->stationid(), tblStation, tblSensors, networkStation ) ) {
         LOGINFO( "No metadata for station <" << it->stationid() << ">.");
         continue;
      }

      if( tblStation.wmono() == kvDbBase::INT_NULL ) {
         LOGWARN( "Station: " << it->stationid() << " Missising wmono.");
         continue;
      }

      pStation = findStation( tblStation.wmono(), 0, "", codeList, newStation );

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

      if( pStation->definedStationID().empty() ) {
         pStation->definedStationid_.push_back( it->stationid() );
         nValues++;
      }

      if( decodeWindheight( tblSensors, pStation ) )
         nValues++;

      if( decodePrecipHeight( tblSensors, pStation ) )
         nValues++;

      if( decodeTemperatureHeight( tblSensors, pStation ) )
         nValues++;

      if( decodePressureHeight( tblSensors, tblStation, pStation ) )
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
