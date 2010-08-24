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
#include "Indent.h"
#include "ConfMaker.h"
#include "StationInfo.h"
#include "splitstr.h"

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
decodeProductCoupling( const std::string &val )
{
   vector<string> keyval = miutil::splitstr(val, '\n' );
   if( keyval.size() > 1 ) {
      cerr << "["<<val << "]" << endl;
      for( vector<string>::size_type i = 0; i < keyval.size(); ++i )
         cerr << "***" << keyval[i] << "***" << endl;
      cerr << "*******************************\n";
   }


   return false;
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

   o << ")" << endl;
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

      if( pStation->stationID().empty() )
         pStation->stationid_.push_back( it->stationid() );

      decodeProductCoupling( it->productcoupling() );

      continue;
      cerr << it->stationid() << ", " << tblStation.wmono() << ", " << tblStation.name() << ", " << it->couplingDelay() << ", " << it->productcoupling() << ", " << it->priorityPrecip() << endl;
      cerr << "SENSORS:";

      for( StInfoSysSensorInfoList::const_iterator sit=tblSensors.begin(); sit != tblSensors.end(); ++sit )
         cerr << " " << sit->paramid()<< "(" << sit->physicalHeight() << ")";

      cerr << endl;
   }
/*
   for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it != stationList.end(); ++it )
      cerr << doStationConf( *it ) << endl;
*/

   return true;
}
