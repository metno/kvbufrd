/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfoParse.cc,v 1.12.2.6 2007/09/27 09:02:22 paule Exp $                                                       

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

#include <float.h>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <milog/milog.h>
#include <miutil/trimstr.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "StationInfo.h"
#include "StationInfoParse.h"
#include "parseMilogLogLevel.h"


using namespace std;
using namespace miutil::conf;


namespace {
const string NoteSection="__NOTE_INTERNAL__";
const string IdType = "idtype";

string
getIdAndType( miconf::ConfSection *conf )
{

   ostringstream ost;
   ValElementList idTypeValue;

   if( ! conf )
	   return "";

   idTypeValue = conf->getValue( NoteSection+"."+IdType );

   for(ValElementList::iterator it=idTypeValue.begin();
         it != idTypeValue.end(); ++it ) {
      if( it != idTypeValue.begin() )
         ost << *it;
      else
         ost << " (" << *it << ")";
   }

   return ost.str();
}

}

bool
StationInfoParse::DefaultVal::
valid()const{
   //   if(copyto.empty() || owner.empty() || list.empty() || delay.empty()){
   //      ost << "Missing key: ";
   //
   //      if(copyto.empty())
   //         ost << "copyto";
   //      else if (owner.empty())
   //         ost << "owner";
   //      else if(list.empty())
   //         ost << "list";
   //      else
   //         ost << "delay";
   //
   //      LOGWARN(ost.str() << "!" << endl);
   //      //return false;
   //   }

   return true;
}

void
StationInfoParse::
parseStationDefSections( miconf::ConfSection *conf,
                        std::list<StationInfoPtr> &stationList,  bool useDefaultValues )
{
   ConfSection *sec;
   string      id;
   StationInfo *info;
   string      idType;

   list<string> sect=conf->getSubSections();
   string::size_type i;

   for(list<string>::iterator it=sect.begin();
         it!=sect.end(); it++){
      i=it->find("wmo_");

      if( (i=it->find("wmo_")) != string::npos ) {
         id = it->substr(i+4);
         idType = "WMO";
      } else if( (i=it->find("id_")) != string::npos ) {
         id = it->substr(i+3);
         idType = "ID";
      } else if( (i=it->find("callsign_")) != string::npos ) {
         id = it->substr(i+9);
         idType = "CALLSIGN";
      } else if( (i=it->find("wsi_")) != string::npos ) {
         id = it->substr(i+4);
         idType = "WSI"; //WIGOS id
      }else {
         continue;
      }

      if(id == "default" )
         continue;

      ConfSectionList allSections=conf->getAllSection(*it);

      if( allSections.size() > 1 ) {
         LOGINFO("Multiple definitions for <"<<*it<<">. This is ok.");
      }

      for( ConfSectionList::iterator cit = allSections.begin();
            cit != allSections.end(); ++cit ) {
         //sec=conf->getSection(*it);
         sec = *cit;

         if(!sec){
            LOGERROR("ERROR: Missing section for <" << *it << ">!" << endl);
            continue;
         }
         try {
            ConfSection *noteSection=new ConfSection();
            noteSection->addValue( IdType, ValElement(idType), true );
            noteSection->appendValue( IdType, ValElement(id) );
            sec->addSection( NoteSection, noteSection );
         }
         catch( ... ) {
            LOGERROR("FATAL: NOMEM: Cant create internal section <" << NoteSection << ">!" << endl);
         }

         curErr.str("");
         curSectionName = *it;

         info = parseSection( sec, id, useDefaultValues );

         string sErr( curErr.str() );

         if(!info){
            errors << "ERRORS while parsing section <" << *it << ">, ignoring the section!" << endl
                  << "Reason: " << sErr;
            LOGERROR("Error while parsing section <" << *it << ">, ignoring the section!" << endl
                     << "Reason: " << sErr );
            continue;
         } else if( ! sErr.empty() ) {
            errors << "WARNINGS while parsing section <" << *it << ">!" << endl
                  << "  file: " << sec->getFilename() << " line: " << sec->getLineno() << "." << endl
                  << "Reason: " << sErr;
            LOGWARN("Warnings while parsing section <" << *it << ">."<< endl
                    << "  file: " << sec->getFilename() << " line: " << sec->getLineno() << "." << endl
                    << "Reason: " << sErr );
         }

         stationList.push_back(StationInfoPtr(info));
      }
   }
}


bool
StationInfoParse::parse( miconf::ConfSection *conf,
                        std::list<StationInfoPtr> &stationList,  bool useDefaultValues )
{
   string defaultConfName("wmo_default");
   ConfSection *defaultConf=0;

   stationList.clear();

   //Only let useDafault controll
   //ignoreMissingValues if it is not explicit
   //set to true.
   if( ! ignoreMissingValues ) {
       if( useDefaultValues ) {
           ignoreMissingValues = false;
       } else {
           ignoreMissingValues = true;
       }
   }

   defaultConf=conf->getSection( defaultConfName );

   if( !defaultConf ) {
      defaultConfName="default";
      defaultConf=conf->getSection( defaultConfName );
   }

   if( !defaultConf && useDefaultValues ){
      useDefaultValues = false;
      LOGWARN("Missing section 'wmo_default' or 'default' section in the configuration file!");
   }

   if( useDefaultValues && !doDefault( defaultConf ) ){
      LOGFATAL("Fatal errors in default section '" << defaultConfName << "' in the configuration file!");
      return false;
   }

   list<string> sect=conf->getSubSections();

   if( defaultConf ) {
      LOGDEBUG("Sections: (" << sect.size() << ")" << endl <<
               "default: " << endl << *defaultConf << endl);
   } else {
      LOGDEBUG("Sections: (" << sect.size() << ")" << endl <<
               "Not using default values. " << endl);
   }

   parseStationDefSections( conf, stationList, useDefaultValues );
   return true;
}

StationInfo* 
StationInfoParse::
parseSection( miconf::ConfSection *stationConf,
             const std::string &id,  bool useDefaultValues )
{
   const char *keywords[]={"stationid", "delay", "precipitation",
                           "typepriority", "list",
                           "copy", "copyto", "owner", "loglevel",
                           "latitude", "longitude", "height",
                           "height_visibility", "height_precip",
                           "height_pressure", "height_temperature",
                           "height_wind","height_wind_above_sea", "name", "callsign", "wmono",
                           "station_id", "wigos_id",
                           "code", "buoy_type",
                           0 };

   list<std::string>           keys;
   list<std::string>::iterator it;
   int                         i;
   StationInfo                 *st=0;
   ValElementList              value;
   string                      idType;
   ValElementList              idTypeValue;
   bool                        ok;

   idTypeValue = stationConf->getValue( NoteSection+"."+IdType );

   //   cerr << "Note: "<< NoteSection+"."+IdType << " Size: " << idTypeValue.size()
   //        << " Value: " << (idTypeValue.size()>0?idTypeValue[0].valAsString():"N/A" ) << endl;
   if( idTypeValue.size() > 0 )
      idType = idTypeValue[0].valAsString();

   keys=stationConf->getKeys();


   for(it=keys.begin(); it!=keys.end(); it++){
      for(i=0; keywords[i]; i++){
         if(keywords[i]==*it)
            break;
      }

      if(!keywords[i]){
         LOGERROR("Unknown key <" << *it << "> in '"<< idType <<"' section <" << id
                  << ">!" << endl);
      }
   }

   try{
      st=new StationInfo();
   }
   catch(...){
      LOGFATAL("NOMEM" << endl);
      return 0;
   }

   if( idType == "WMO" ) {
      st->wmono_= atoi( id.c_str() );
      st->sectionType_=StationInfo::ST_WMO;
   } else if( idType == "ID" ) {// idType == "ID"
      st->stationID_ = atoi( id.c_str() );
      st->sectionType_=StationInfo::ST_STATIONID;
   } else if( idType == "CALLSIGN"){
      st->callsign_ = id;
      st->sectionType_=StationInfo::ST_CALLSIGN;
   } else { //idType == "WSI")
      st->wsiId_ = id;
      st->sectionType_=StationInfo::ST_WSI;
   }

   if( stationConf->ignoreThisSection() )
       st->ignore = true;

   for(i=0; keywords[i]; i++){
      value=stationConf->getValue(keywords[i]);

      if(value.empty() && useDefaultValues ){
         if( strcmp( keywords[i], "precipitation") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!" << endl);
            st->precipitation_=defVal.precipitation;
         }else if( strcmp( keywords[i], "list") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!" << endl);
            st->list_=defVal.list;
         }else if( strcmp( keywords[i], "copy") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!" << endl);
            st->copy_=defVal.copy;
         }else if( strcmp( keywords[i], "copyto") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in " << idType << "  section <"
                      << id << ">! Using default value!" << endl);
            st->copyto_=defVal.copyto;
         }else if( strcmp( keywords[i], "owner") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!" << endl);
            st->owner_=defVal.owner;
         }else if( strcmp( keywords[i],"delay") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" <<keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!");
            st->delayList_=defVal.delay;
         }else if( strcmp( keywords[i], "loglevel") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" <<keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!");
            st->loglevel_=defVal.loglevel;
         }else if( strcmp( keywords[i], "code") == 0 ){
            LOGDEBUG6("NO VALUE: for key <" <<keywords[i] << "> in " << idType << " section <"
                      << id << ">! Using default value!");
            st->code(defVal.code, true);
         } else if( strcmp( keywords[i], "height" ) == 0 ||
               strcmp( keywords[i], "height_precip" ) == 0 ||
               strcmp( keywords[i], "height_pressure" ) == 0 ||
               strcmp( keywords[i], "height_temperature" ) == 0 ||
               strcmp( keywords[i], "height_visibility" ) == 0 ||
               strcmp( keywords[i], "height_wind" ) == 0 ||
               strcmp( keywords[i], "height_wind_above_sea" ) == 0 ||
               strcmp( keywords[i], "latitude" ) == 0 ||
               strcmp( keywords[i], "longitude" ) == 0  ||
               strcmp( keywords[i], "buoy_type" ) == 0  ||
               strcmp( keywords[i], "name" ) == 0 ||
               strcmp( keywords[i], "callsign" ) == 0 ||
               strcmp( keywords[i], "wmono" ) == 0 ||
               strcmp( keywords[i], "station_id" ) == 0 ||
               strcmp( keywords[i], "wigos_id" ) == 0
               ) {
            LOGDEBUG( "NO VALUE: for key <" << keywords[i] << "> in " << idType << " section <"
                      << id << ">!. Ignore!");
         } else {
            curErr << "NO VALUE: Mandatory key: <" << keywords[i] << ">! And no default value!" << endl
                  << "    Filename: " << stationConf->getFilename() << " line: " << stationConf->getLineno() << endl;

            delete st;
            return 0;
         }
      }else{
         ok=true;

         if( strcmp( keywords[i], "stationid" ) == 0 ){
            ok=doStationid(keywords[i], value, *st);
         }else if( strcmp( keywords[i], "delay" ) == 0 ){
            ok=doDelay(keywords[i], value, *st, stationConf, !ignoreMissingValues );
         }else if( strcmp( keywords[i], "precipitation" ) == 0 ){
            ok=doPrecip(keywords[i], value, *st);
         }else if( strcmp( keywords[i], "typepriority" ) == 0 ){
            ok=doTypePri(keywords[i], value, *st);
         }else if( strcmp( keywords[i], "owner" ) == 0 ){
            st->owner_=doDefOwner( value );
            ok=ignoreMissingValues || !st->owner_.empty();
         }else if( strcmp( keywords[i], "list" ) == 0 ){
            st->list_=doDefList( value );
            ok=ignoreMissingValues || !st->list_.empty();
         }else if( strcmp( keywords[i], "copy" ) == 0 ){
            st->copy_=doDefCopy(value, &st->copyIsSet_ );
         }else if( strcmp( keywords[i], "copyto" ) == 0 ){
            st->copyto_=doDefCopyto( value );
         }else if( strcmp( keywords[i], "loglevel" ) == 0 ){
            st->loglevel_=doDefLogLevel( value );
         } else if( strcmp( keywords[i], "height" ) == 0 ) {
            doInt( st->height_, value );
         } else if( strcmp( keywords[i], "height_precip" ) == 0 ) {
            doInt( st->heightPrecip_, value );
         }else if( strcmp( keywords[i], "height_pressure" ) == 0 ) {
            doFloat( st->heightPressure_, value );
         }else if( strcmp( keywords[i], "height_temperature" ) == 0 ) {
            doInt( st->heightTemperature_, value );
         }else if( strcmp( keywords[i], "height_visibility" ) == 0 ) {
            doInt( st->heightVisability_, value );
         }else if( strcmp( keywords[i], "height_wind" ) == 0 ) {
            doInt( st->heightWind_, value );
         }else if( strcmp( keywords[i], "height_wind_above_sea" ) == 0 ) {
             doInt( st->heightWindAboveSea_, value );
         } else if( strcmp( keywords[i], "latitude" ) == 0 ) {
            doFloat( st->latitude_, value );
         }else if( strcmp( keywords[i], "longitude" ) == 0 ) {
            doFloat( st->longitude_, value );
         } else if( strcmp( keywords[i], "buoy_type" ) == 0 ) {
            doInt(st->buoyType_, value );
         }else if( strcmp( keywords[i], "code") == 0 ){
            if( value.size() > 1 ) {
                curErr << "code: can only have one value. The code has #" << value.size() <<"." << endl;
               ok=false;
            } else {
               st->code(value.valAsInt(0), true );
            }
         }else if( strcmp( keywords[i], "name" ) == 0 ) {
            st->name( value.valAsString() );
         } else if( strcmp( keywords[i], "wmono" ) == 0 ) {
            doInt(st->wmono_, value);
         } else if( strcmp( keywords[i], "wigos_id" ) == 0 ) {
            st->wsiId_=value.valAsString();
         }

         if(!ok){
            curErr << "    Filename: " << stationConf->getFilename() << " line: " << stationConf->getLineno() << endl;
            delete st;
            return 0;
         }
      }
   }

   if( !ignoreMissingValues  && st->definedStationid_.empty() ){
      curErr << "MISSING KEY <stationid> (mandatory key)!" << endl
            << "    Filename: " << stationConf->getFilename() << " line: " << stationConf->getLineno() << endl;

      delete st;
      return 0;
   }

   if( !ignoreMissingValues && st->typepriority_.empty() ){
      curErr << "MISSING KEY <typepriority> (mandatory key)!" << endl
            << "    Filename: " << stationConf->getFilename() << " line: " << stationConf->getLineno() << endl;
      delete st;
      return 0;
   }

   //TODO: At the moment we hardcode the BUFR 'code' value to
   //0 (SYNOP) if it is NOT given and we have a wmo number.
   if( st->wmono() > 0 ) {
      st->code(0, false);
   }

   if(st->delayList_.empty() &&
      ( st->typepriority_.size() > 1 || st->definedStationid_.size() > 1 ) ){
      st->delayList_.push_back(DelayInfo(DelayInfo::FSTIME, 5, false) );
      st->delayConf="fS:05";
      LOGDEBUG("Setting default 'delay' to: " << st->delayList_.front() << endl);
   }


   return st;
}

StationInfoPtr
StationInfoParse::
defaultVal()const
{
   StationInfoPtr p = StationInfoPtr( new StationInfo() );
   p->precipitation_=defVal.precipitation;
   p->list_=defVal.list;
   p->copy_=defVal.copy;
   p->copyto_=defVal.copyto;
   p->owner_=defVal.owner;
   p->delayList_=defVal.delay;
   p->loglevel_=defVal.loglevel;
   p->code_=defVal.code;

   return p;
}

bool 
StationInfoParse::
doDefault(miutil::conf::ConfSection *stationConf)
{
   list<std::string>           keys;
   list<std::string>::iterator it;
   ValElementList              value;
   bool ok;
   keys=stationConf->getKeys();

   for(it=keys.begin(); it!=keys.end(); it++){
      value=stationConf->getValue(*it);

      if(value.empty()){
         LOGDEBUG("NOVALUE: for key <" << *it << "> in <" << curSectionName <<"> section! Ignoring it.");
         continue;
      }

      ok = true;

      if(*it=="list"){
         defVal.list=doDefList( value );
         ok = ! defVal.list.empty();
      }else if(*it=="owner"){
         defVal.owner=doDefOwner( value );
         ok = ! defVal.owner.empty();
      }else if(*it=="precipitation"){
         defVal.precipitation=doDefPrecip( value );
         ok = ! defVal.precipitation.empty();
      }else if(*it=="copy"){
         defVal.copy=doDefCopy( value, &ok );
      }else if(*it=="copyto"){
         defVal.copyto=doDefCopyto( value );
         ok = ! defVal.copyto.empty();
      }else if(*it=="delay"){
         defVal.delay=doDefDelay( value, defVal.delayConf, stationConf );
         ok = ! defVal.delay.empty();
      }else if(*it=="loglevel"){
         defVal.loglevel=doDefLogLevel( value );
      }else if(*it=="code"){
         defVal.code = doDefCode( value ); //Default value 0 is SYNOP.
         ok = defVal.code >= 0; 
      }else{
         LOGWARN("UNKNOWN KEY: in <"<< curSectionName << "> section! Ignoring it.");
      }

      if( ! ok )
         return false;
   }

   return true;
}


milog::LogLevel 
StationInfoParse::
doDefLogLevel( miconf::ValElementList &vl)
{
   string val;
   milog::LogLevel ll;
   IValElementList it=vl.begin();

   if(it==vl.end())
      return milog::INFO;

   val=it->valAsString();

   ll = parseMilogLogLevel( val );

   if( ll == milog::NOTSET ) {
      ll = milog::INFO;
      LOGERROR("Invalid loglevel value <" << val << "> in section <" << curSectionName << ">. Setting default loglevel to INFO.");
   }

   return ll;
}


int
StationInfoParse::
doDefCode( miconf::ValElementList &vl )
{
   if( vl.size()>0) {
      return vl.begin()->valAsInt();
   }

   return -1;
}



std::string
StationInfoParse::
doDefList( miconf::ValElementList &vl )
{
   IValElementList it=vl.begin();

   if(it==vl.end())
      return string();

   if(it->type()!=STRING){
      curErr << "INVALID TYPE: key <list>, expecting STRING." << endl;
      return string();
   }

   string val=it->valAsString();

   if(val.size()!=2){
      curErr << "INVALID FORMAT: key <list> must have a size of two (2) char!" <<
            "." << endl;
      return string();
   }

   return val;
}

std::string
StationInfoParse::
doDefOwner( miconf::ValElementList &vl )
{
   IValElementList it=vl.begin();

   if(it==vl.end())
      return string();

   if(it->type()!=STRING){
      curErr << "INVALID TYPE: key <owner>, expecting STRING."<< endl;
      return string();
   }

   string val=it->valAsString();

   if(val.size()!=4){
      curErr << "INVALID FORMAT: key <owner> must have a size of four (4) char!" << endl;
      return string();
   }

   return val;
}

std::list<std::string>
StationInfoParse::
doDefPrecip( miconf::ValElementList &vl )
{
   const char *validval[]={"RA", "RR", "RRRtr", "RR_01", "RR_1",
                           "RR_3", "RR_6","RR_12","RR_24", "RR_N", 0};
   int i;
   std::list<std::string> RR;
   std::list<std::string>::iterator itRR;

   IValElementList it=vl.begin();

   for( ; it!=vl.end(); it++, i++){
      if(it->type()!=STRING){
         curErr << "INVALID TYPE: key <precipitation> element number: "
               << i << "." << endl;
         continue;
      }

      for(i=0; validval[i]; i++){
         if(it->valAsString()==validval[i])
            break;
      }

      if(!validval[i]){
         curErr << "INVALID VALUE: key <precipitation> element number: "
               << i << "." << endl;
         continue;
      }

      itRR=RR.begin();

      for( ; itRR!=RR.end(); itRR++){
         if(*itRR==it->valAsString())
            break;
      }

      if(itRR==RR.end())
         RR.push_back(it->valAsString());

   }

   return RR;
}

bool
StationInfoParse::
doDefCopy( miconf::ValElementList &vl, bool *copyIsSet )
{
   IValElementList it=vl.begin();

   if( copyIsSet )
      *copyIsSet = false;

   if(it==vl.end())
      return false;

   if(it->type()!=STRING){
      curErr << "INVALID TYPE: key <copy>, expecting STRING." << endl;
      return false;
   }

   string val=it->valAsString();
   char ch;

   if( val.empty() )
      return false;

   ch = val[0];

   if( copyIsSet )
      *copyIsSet = true;

   if(ch=='t' || ch=='T' )
      return true;
   else if(ch=='f' || ch=='F')
      return false;
   else{
      if( copyIsSet )
         *copyIsSet = false;

      curErr << "WRONG VALUE: key <copy> valid values \"true\" or \"false\"!" << endl;
      return false;
   }
}

std::string
StationInfoParse::
doDefCopyto( miconf::ValElementList &vl )
{
   IValElementList it=vl.begin();


   if(it==vl.end())
      return string();

   if(it->type()!=STRING){
      curErr << "INVALID TYPE: key <copyto>, expecting STRING." << endl;
      return string();
   }

   return it->valAsString();
}

StationInfo::TDelayList 
StationInfoParse::
doDefDelay(const miconf::ValElementList &vl,
           std::string &delayConf,
           miconf::ConfSection *conf )
{
   StationInfo::TDelayList dl;
   CIValElementList it=vl.begin();
   string::size_type i;
   char              sign;
   string            val;
   string            sHH, sMM;
   bool              force=false;
   int               hh;
   int               mm;


   for(int element=1 ; it!=vl.end(); it++, element++){
      if(it->type()!=STRING){
         curErr << "INVALID TYPE: key <delay> element number: "
               << element;

         if( conf )
        	 curErr << " in section <" << getIdAndType( conf )<< ">";

         curErr << ", expecting STRING!" << endl;
         continue;
      }

      val=it->valAsString();

      if( val.size() >  0 && (val[0]=='-' || val[0]=='+') ) {
         sign = val[0];
         val.erase( 0, 1 );
      } else {
         sign = 0;
      }

      if( sign ){
         MsgTime msgTime;

         try {
            msgTime.setMsgForTime( val );
         }
         catch( const std::exception &ex ) {
            curErr << "Invalid time spec '" << sign << val << "'. " << ex.what()<< endl;
            continue;
         }

         if( sign == '-' )
            msgTime.invertHours();

         //Look up to see if there is any SKIP spec
         //allready parsed. If so add this hour to the
         //spec. This means that we only have one SKIP spec
         //in the DelayInfo list for the station. The SKIP
         //spec is also at the start of the list.
         StationInfo::ITDelayList itd=dl.begin();

         if(itd!=dl.end())
            if(!itd->skipMsgSpec())
               itd=dl.end();

         //We have do not a SKIP spec, add one at the front.
         if(itd==dl.end()){
            dl.push_front(DelayInfo(DelayInfo::SKIP));
            itd=dl.begin();
         }

         //We all ready have a SKIP spec, replace it with this one.
         itd->setMsgTime( msgTime );
         continue;
      }


      i=val.find(":");
      if( i == string::npos ) {
         sHH = val;
         sMM.erase();
      } else {
         sHH=val.substr(0, i);
         sMM=val.substr(i+1);
      }

      if(sHH.empty() || sMM.empty()){
         curErr << "INVALID FORMAT: key <delay> element number: "
               << element << ", format \"hh:mm\"!" << endl;
         continue;
      }

      string::size_type ii;

      for(ii=0; ii<sHH.length(); ii++)
         if(!isdigit(sHH[ii]))
            break;

      force=false;

      if( sign == '+' ) {
         if( sHH == "HH" )

            continue;
      }

      //    cerr << "ii=" << ii << " sHH=" << sHH << " sMM=" << sMM << endl;

      if(ii<sHH.length()){
         if(sHH=="FS" || sHH=="SS" || sHH=="FH" || sHH=="HH"){
            if(sHH[0]=='F')
               force=true;

            if(sHH[1]=='S')
               hh=DelayInfo::STIME;
            else
               hh=DelayInfo::HTIME;
         }else if(sHH=="fS"){
            hh=DelayInfo::FSTIME;
         }else if(sHH=="fH"){
            hh=DelayInfo::FHTIME;
         }else{
            curErr << "INVALID FORMAT: key <delay> element number: "
                  << element << ", format \"hh:mm\", where hh is a number [0,23] or the "
                  << "one of the values FH, HH, FS or SS. mm is a number"
                  << "[0,59]!" << endl;
            continue;
         }
      }else{
         hh=atoi(sHH.c_str());

         if(hh<0 || hh>23){
            curErr << "INVALID FORMAT: key <delay> element number: "
                  << element << ", format \"hh:mm\", where hh is a number [0,23] or "
                  << "one of the values FH, HH, FS or SS. mm is a number"
                  << " [0,59]!" << endl;
            continue;
         }
      }

      for(ii=0; ii<sMM.length(); ii++)
         if(isdigit(sMM[ii]))
            break;

      if(ii>=sMM.length()){
         curErr << "INVALID FORMAT: key <delay> element number: "
               << element << ", format \"hh:mm\", where hh is a number [0,23] or the "
               << "one of the values FH, HH, FS or SS. mm is a number [0,59]!"
               << endl;
         continue;
      }

      mm=atoi(sMM.c_str());

      if(mm<0 || mm>59){
         curErr << "INVALID FORMAT: key <delay> element number: "
               << element << ", format \"hh:mm\", where hh is a number [0,23] or the "
               << "one of the values FH, HH, FS or SS. mm is a number"
               << " [0,59]!" << endl;
         continue;
      }

      StationInfo::ITDelayList itd=dl.begin();

      for( ; itd!=dl.end(); itd++){
         if(hh>=itd->hour())
            break;
      }

      if(itd==dl.end()){
         dl.push_back(DelayInfo(hh, mm, force));
      }else if(hh==itd->hour()){
         curErr << "DUPLICATE ELEMENT: key <delay> element number: "
               << element << "."  << endl;
         continue;
      }else{
         dl.insert(itd, DelayInfo(hh, mm, force));
      }

   }

   if( !dl.empty()  ) {
      ostringstream o;
      o << "(";

      for( it=vl.begin(); it != vl.end(); ++it ) {
         if( it->type() != STRING )
            continue;

         val = it->valAsString();
         miutil::trimstr( val );

         if( !val.empty() && val[0]!='"' )
            val.insert( 0, "\"" );

         if( !val.empty() && val[val.length()-1]!='"' )
            val += "\"";

         if( it != vl.begin() )
            o << ",";

         o << val;
      }
      o << ")";
      delayConf = o.str();
   }

   return dl;
}



bool
StationInfoParse::
doStationid(const std::string &key,
            miconf::ValElementList &vl,
            StationInfo &st)
{
   int i=1;
   bool error;
   int id;
   StationInfo::ITLongList itl;
   IValElementList it=vl.begin();

   for( ; it!=vl.end(); it++, i++){
      if(it->type()!=INT){
         curErr << "INVALID TYPE: key <" << key << "> element number: "
               << i << endl;
         continue;
      }

      id = it->valAsInt();

      for( itl = st.definedStationid_.begin();
            itl != st.definedStationid_.end(); itl++){

         if(*itl== id )
            break;
      }

      if( itl == st.definedStationid_.end() )
         st.definedStationid_.push_back( id );
   }

   if( !ignoreMissingValues && st.definedStationid_.empty()){
      curErr << "NOVALUE: key <" << key << "> has now valid values!" << endl;
      return false;
   }

   return true;
}

bool
StationInfoParse::
doDelay( const std::string &key,
         miconf::ValElementList &vl, StationInfo &st,
         miconf::ConfSection *conf,
         bool mayUseDefaultValues )
{

   st.delayList_=doDefDelay(vl, st.delayConf, conf );

   if( st.delayList_.empty() && mayUseDefaultValues ){
      curErr << "No value for <delay>, using default!";
      st.delayList_=defVal.delay;
      st.delayConf = defVal.delayConf;
   }

   return ( vl.empty() && st.delayList_.empty() ) || ( !vl.empty() && !st.delayList_.empty() );
}


bool
StationInfoParse::
doPrecip(const std::string &key,
         miconf::ValElementList &vl, StationInfo &st)
{  
   std::list<std::string> RR=doDefPrecip( vl );

   st.precipitation_=RR;

   return ( vl.empty() && st.precipitation_.empty() ) || ( !vl.empty() && !st.precipitation_.empty() );
}

bool
StationInfoParse::
doTypePri(const std::string &key,
          miconf::ValElementList &vl, StationInfo &st)
{
   bool error=false;
   int i;
   bool mustHaveType;
   long mytype;
   string smytype;

   IValElementList it=vl.begin();

   for(i=0; it!=vl.end(); it++, i++){
      mustHaveType=false;

      if(it->type()!=INT && it->type()!=STRING){
         curErr << "INVALID TYPE: key <" << key << "> element number: "
               << i << "!" <<  endl;
         error=true;
         continue;
      }

      if(it->type()==STRING){
         string val=it->valAsString();
         string::size_type ii=val.find_first_of("*");

         if(ii!=string::npos){
            mustHaveType=true;
            val.erase(0, ii+1);
         }

         ii=val.find_first_of(":");

         if(ii==string::npos){
            smytype=val;
         }else{
            smytype=val.substr(0, ii);
            val.erase(0, ii+1);
         }


         if(sscanf(smytype.c_str(), "%ld", &mytype)!=1){
            curErr << "INVALID TYPE: key <" << key << "> element number: "
                  << i << "! Not an integer!"  <<  endl;
            error=true;
            continue;
         }
      }else{
         mytype=it->valAsInt();
      }

      StationInfo::ITTypeList itl=st.typepriority_.begin();

      for( ; itl!=st.typepriority_.end(); itl++){
         if(itl->typeID()==mytype)
            break;
      }

      if(itl==st.typepriority_.end()){
         StationInfo::Type  type(mytype);

         if(mustHaveType)
            type.mustHaveType(true);

         st.typepriority_.push_back(type);
      }
   }


   //Check if we have at least one mustHaveType. If not set
   //all types to mustHaveTypes.

//   mustHaveType=false;
//
//   for(StationInfo::ITTypeList itl=st.typepriority_.begin();
//         itl!=st.typepriority_.end();
//         itl++){
//      if(itl->mustHaveType()){
//         mustHaveType=true;
//         break;
//      }
//   }
//
//   if(!mustHaveType){
//      for(StationInfo::ITTypeList itl=st.typepriority_.begin();
//            itl!=st.typepriority_.end();
//            itl++)
//         itl->mustHaveType(true);
//   }

   if(error)
      return false;

   return ignoreMissingValues || !st.typepriority_.empty();
}

void
StationInfoParse::
doInt( int &i, const miconf::ValElementList &val )
{
   if( val.empty() ) {
      i = INT_MAX;
      return;
   }

   try{
      i = val.begin()->valAsInt();
   }
   catch( ... ) {
      i = INT_MAX;
   }
}

void
StationInfoParse::
doIntList( std::list<int> &iList, const miconf::ValElementList &val )
{
   int code;
   iList.clear();

   BOOST_FOREACH( ValElement it, val ) {
      if( it.type() != miutil::conf::INT )
         continue;

      code = it.valAsInt();

      if( find( iList.begin(), iList.end(), code ) == iList.end() )
         iList.push_back( code );
   }
}


void
StationInfoParse::
doFloat( float &f, const miconf::ValElementList &val )
{
   if( val.empty() ) {
      f = FLT_MAX;
      return;
   }

   try {
      f = val.begin()->valAsFloat();
   }
   catch( ... ) {
      f = FLT_MAX;
   }
}

