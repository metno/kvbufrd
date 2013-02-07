/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfo.cc,v 1.10.2.8 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <limits.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <miutil/replace.h>
#include "StationInfo.h"

namespace b=boost;
using namespace std;

namespace {
std::string printOut( int i )
{
   if( i == INT_MAX  || i == INT_MIN  )
      return "";

   std::ostringstream o;
   o << i;
   return o.str();
}

std::string printOut( float i )
{
   if( i == FLT_MAX || i == FLT_MIN )
      return "";

   std::ostringstream o;
   o << i;
   return o.str();
}


/**
 *
 * @throw std::out_of_range, boost::bad_lexical_cast
 */
string::size_type
splitToInt( const string &valToSplit,
            const char *splitAt,
            vector<int> &result,
            int valueUpperLimit )
{
   int iVal;
   vector<string> sVals;
   string::size_type i = valToSplit.find_first_of( splitAt );
   result.clear();

   if( i == string::npos ) {
      result.push_back( b::lexical_cast<int>( b::trim_copy( valToSplit ) ) );
   } else {
      b::split( sVals, valToSplit, b::is_any_of( string(1, valToSplit[i]) ), b::token_compress_on );

      for( int i=0; i<sVals.size(); ++i ) {
         iVal = b::lexical_cast<int>( b::trim_copy( sVals[i] ) );
         if( iVal < 0 || iVal > valueUpperLimit ) {
            ostringstream err;
            err << "Valid range [0," << valueUpperLimit << "].";
            throw out_of_range( err.str() );
         }
         result.push_back( iVal );
      }
   }

   return i;
}
}

MsgTime::
MsgTime( bool minutesForEachHour )
: minAtHour_( 0 )
{
   hours_.set( 24, minutesForEachHour );

   if( ! minutesForEachHour )
      minAtHour_ = new Minutes[1];
}

MsgTime::
MsgTime()
: minAtHour_( 0 )
{
   hours_.set( 24, true );
   minAtHour_ = new Minutes[1];
}

MsgTime::
MsgTime( const MsgTime &mt )
: hours_( mt.hours_ ), minAtHour_(  0 )
{

   if( ! mt.minAtHour_ )
      return;

   int n = count();

   if( n == 0 )
      n=1; //We have at least one minute element.

   minAtHour_ = new Minutes[n];

   for( int i=0; i<n; ++i )
      minAtHour_[i]=mt.minAtHour_[i];
}

MsgTime::
~MsgTime()
{
   if( minAtHour_ )
      delete[] minAtHour_;
}

MsgTime&
MsgTime::
operator=( const MsgTime &rhs )
{
   if( &rhs == this )
      return *this;

   if( minAtHour_ ) {
      delete[] minAtHour_;
      minAtHour_ = 0;
   }

   hours_ = rhs.hours_;

   int n = rhs.count();

   if( rhs.minAtHour_ ) {
      minAtHour_ = new Minutes [ n ];

      for( int i=0 ; i < n; ++i )
         minAtHour_[i] = rhs.minAtHour_[i];
   }
   return *this;
}

bool
MsgTime::
operator==(const MsgTime &rhs)const
{
   if( hours_ != rhs.hours_)
      return false;

   if( count() != rhs.count() )
      return false;

   if( minAtHour_ && rhs.minAtHour_ ) {
      int n = count();
      for( int i=0; i<n; ++i ) {
         if( minAtHour_[i] != rhs.minAtHour_[i] )
            return false;
      }
      return true;
   }

   if( ! minAtHour_ && ! rhs.minAtHour_ )
      return true;

   return false;
}

int
MsgTime::
count()const
{
   int n=0;

   for( int i=0; i<24; ++i )
      if( hours_.test(i) ) ++n;

   return n;
}

bool
MsgTime::
msgForTime( const miutil::miTime & t )const
{
   if( ! hours_.test( t.hour() ) )
      return false;

   Minutes min=minAtHour( t.hour() );
   return min.test( t.min() );
}

bool
MsgTime::
minForEachHour()const
{
   return hours_.test( 24 );
}

void
MsgTime::
reallocateAndSet( int hour, int min )
{
   if( ! minForEachHour() )
      return;

   if( hours_.test( hour ) )
      return;

   if( ! minAtHour_ ) {
      minAtHour_ = new Minutes[1];
      hours_.set( hour, true );
      minAtHour_[0].set( min, true );
      return;
   }

   int index=-1;

   for( int i=0; i<hour; ++i )
      if( hours_.test( i ) ) ++index;

   int oldN = count();
   int copyI;
   Minutes *newMin = new Minutes[oldN+1];
   hours_.set( hour, true );

   //copy the old minutes up to the index
   for( copyI=0; copyI<=index; ++copyI  )
      newMin[copyI] = minAtHour_[copyI];

   newMin[copyI].set( min, true ); //The new hour.

   //Continue to copy the rest of the minutes from the old.
   for( ; copyI < oldN; ++copyI )
      newMin[copyI+1]=minAtHour_[copyI];

}

int
MsgTime::
getIndexForHour( int h )const
{
   if( ! minForEachHour() )
      return 0;

   if( ! hours_.test( h ) )
      return -1;

   int index=-1;

   for( int i=0; i<=h; ++i )
      if( hours_.test( i ) ) ++index;

   return index;
}

void
MsgTime::
setAllHours()
{
   bool saved = hours_[24];
   hours_.set();
   hours_.set( 24, saved );
}

void
MsgTime::
setHour( int h, bool flag )
{
   if( h < 0 || h > 23 )
      throw out_of_range( "Valid range [0,23]");

   hours_.set( h, flag );
}

void
MsgTime::
decodeHourSpec( const std::string &hh_ )
{

   int n;
   int nStart=0;
   vector<int> iVals;
   string hh = boost::trim_copy( hh_ );

   if( hh.empty() )
      throw logic_error("No value for the <hour>");

   if( hh == "SS" ) {
      n = 3;
   } else if( hh == "HH" || hh=="*" ) {
      setAllHours();
      return;
   } else {
      string::size_type i = splitToInt( hh, ",/", iVals, 23 );

      if( i == string::npos ) {
         setHour( iVals[0], true );
         return;
      } else if( hh[i] == ',' ) {
         for( int k=0; k < iVals.size(); ++k )
            setHour( iVals[k], true );
         return;
      } else if( hh[i] == '/' ) {
         if( iVals.size() != 2 )
            throw logic_error("Invalid use of '/' in hour spec.");
         nStart = iVals[0];
         n = iVals[1];
         if( n == 0 )
            throw out_of_range("Hour spec: The step > 0.");
      }
   }

   for( int i = nStart; i<24; i+=n )
      setHour( i, true );
}

void
MsgTime::
decodeMinSpec( const std::string &mm_ )
{
   int n;
   int nStart=0;
   vector<int> iVals;
   string mm = boost::trim_copy( mm_ );

   if( mm.empty() )
      throw logic_error("No value for the <minute>");

   if( mm == "*" ) {
      minAtHour_[0].set();
      return;
   }

   string::size_type i = splitToInt( mm, ",/", iVals, 59 );

   if( i == string::npos ) {
      minAtHour_[0].set( iVals[0], true );
      return;
   } else if( mm[i] == ',' ) {
      for( int k=0; k < iVals.size(); ++k )
         minAtHour_[0].set( iVals[k], true );
      return;
   } else if( mm[i] == '/' ) {
      if( iVals.size() != 2 )
         throw logic_error("Invalid use of '/' in minute spec.");
      nStart = iVals[0];
      n = iVals[1];

      if( n == 0 )
         throw out_of_range("Minute spec: The step > 0.");
   }

   for( i = nStart; i<60; i+=n )
      minAtHour_[0].set( i, true  );
}


MsgTime::Minutes
MsgTime::
minAtHour( int h )const
{
   if( h < 0 || h > 23 )
      return Minutes();

   int index = getIndexForHour( h );

   if( index < 0 )
      return Minutes();

   return minAtHour_[index];
}

void
MsgTime::
invertHours()
{
   for( int i=0; i<24; ++i )
      hours_.flip( i );
}

void
MsgTime::
setMsgForTime( int hour, int min, bool flag )
{
   int index=getIndexForHour( hour );

   if( index < 0 && flag ) {
      //We only reallocate and create an minute set if
      //the flag is true.
      reallocateAndSet( hour, min );
      return;
   }

   minAtHour_[index].set( min, flag );
}

void
MsgTime::
setMsgForTime( const std::string &timespec )
{

   string sHH, sMM;
   string::size_type i;

   hours_.reset();

   if( minAtHour_ ) {
      delete minAtHour_;
      minAtHour_ = 0;
   }

   minAtHour_ = new Minutes[1];
   minAtHour_[0].reset();

   i=timespec.find(":");

   if( i == string::npos ) {
      sHH = timespec;
   } else {
      sHH = timespec.substr(0, i);
      sMM = timespec.substr(i+1);
      b::trim( sMM );
   }

   b::trim( sHH );

   decodeHourSpec( sHH );

   if( ! sMM.empty() )
      decodeMinSpec( sMM );
   else
      minAtHour_[0].set( 0, true );
}


StationInfo::
StationInfo():
height_( INT_MAX ),
heightVisability_( INT_MAX ),
heightTemperature_( INT_MAX ),
heightPressure_( INT_MAX ),
heightPrecip_( INT_MAX ),
heightWind_( INT_MAX ),
heightWindAboveSea_( INT_MAX ),
latitude_( FLT_MAX ),
longitude_( FLT_MAX ),
wmono_( 0 ),
stationID_( 0 ),
copyIsSet_( false ),
cacheReloaded48_(true),
ignore( false )
{
}

StationInfo::
StationInfo( int wmono ):
height_( INT_MAX ),
heightVisability_( INT_MAX ),
heightTemperature_( INT_MAX ),
heightPressure_( INT_MAX ),
heightPrecip_( INT_MAX ),
heightWind_( INT_MAX ),
heightWindAboveSea_( INT_MAX ),
latitude_( FLT_MAX ),
longitude_( FLT_MAX ),
wmono_( wmono ),
stationID_( INT_MIN ),
copyIsSet_( false ),
cacheReloaded48_(true),
ignore( false )
{
}

StationInfo::
StationInfo(const StationInfo &i)
{
   delayConf = i.delayConf;
   loglevel_ = i.loglevel_;
   code_ = i.code_;
   height_ = i.height_;
   heightVisability_ = i.heightVisability_;
   heightTemperature_ = i.heightTemperature_;
   heightPressure_ = i.heightPressure_;
   heightPrecip_ = i.heightPrecip_;
   heightWind_ = i.heightWind_;
   heightWindAboveSea_ = i.heightWindAboveSea_;
   latitude_ = i.latitude_;
   longitude_ = i.longitude_;
   wmono_=i.wmono_;
   name_ = i.name_;
   stationID_ = i.stationID_;
   callsign_ = i.callsign_;
   definedStationid_=i.definedStationid_;
   typepriority_=i.typepriority_;
   precipitation_=i.precipitation_;
   delayList_=i.delayList_;
   list_=i.list_;
   copyIsSet_ = i.copyIsSet_;
   copy_=i.copy_;
   copyto_=i.copyto_;
   owner_=i.owner_;
   delayUntil_=i.delayUntil_;
   cacheReloaded48_=i.cacheReloaded48_;
   ignore = i.ignore;
}

StationInfo::
~StationInfo()
{
}

void
StationInfo::
height( int h, bool ifUnset )
{
   if( !ifUnset || height_ == INT_MAX )
      height_ = h;
}

int
StationInfo::
heightAdd( int ammount )const
{
   if( height_ != INT_MAX )
      return height_ + ammount;

   return height_;
}


int
StationInfo::
heightVisability() const
{
   return heightVisability_;
}

int
StationInfo::
heightTemperature()const
{
    return  heightTemperature_;
}

int
StationInfo::
heightPressure() const
{
   return heightPressure_;
}

int
StationInfo::
heightPrecip()const
{
   return heightPrecip_;
}

int
StationInfo::
heightWind()const
{
   return heightWind_;
}
int
StationInfo::
heightWindAboveSea()const
{
   return heightWindAboveSea_;
}


void
StationInfo::
latitude( float lat, bool ifUnset )
{
   if( !ifUnset || latitude_ == FLT_MAX )
      latitude_ = lat;
}

void
StationInfo::
longitude( float lon, bool ifUnset )
{
   if( !ifUnset || longitude_ == FLT_MAX )
      longitude_ = lon;
}

void
StationInfo::
name( const std::string &n, bool ifUnset )
{
   if( ! ifUnset || name_.empty() )
      name_ = n;
}

int
StationInfo::
stationID()const
{
   return stationID_;
}


bool      
StationInfo::
hasDefinedStationId(long stid)const
{
   CITLongList it=definedStationid_.begin();

   for( ; it!=definedStationid_.end(); it++){
      if(stid==*it)
         return true;
   }

   return false;
}

StationInfo::TLongList 
StationInfo::
typepriority(int hour)const
{ 
   TLongList   ret;
   CITTypeList it=typepriority_.begin();

   //This is an error, we silent ignore it, and returns all typeids.
   if(hour>23)
      hour=-1;

   for(;it!=typepriority_.end(); it++)
      if(hour<0 || it->hour(hour))
         ret.push_back(it->typeID());

   return ret;
}


std::list<int> 
StationInfo::
continuesTypeids(const std::list<int> &ctList)const
{
   std::list<int>::const_iterator ctit=ctList.begin();
   CITTypeList it;
   std::list<int> retList;

   for(;ctit!=ctList.end(); ctit++){
      for(it=typepriority_.begin(); it!=typepriority_.end(); it++){
         if(it->typeID()==static_cast<long>(*ctit)){
            retList.push_back(*ctit);
            break;
         }
      }
   }

   return retList;
}

StationInfo::TLongList 
StationInfo::
mustHaveTypes(int hour)const
{
   TLongList   ret;
   CITTypeList it=typepriority_.begin();

   //This is an error, we silent ignore it, and returns all typeids.
   if(hour>23)
      hour=-1;

   for(;it!=typepriority_.end(); it++)
      if(it->mustHaveType() && (hour<0 || it->hour(hour)))
         ret.push_back(it->typeID());

   return ret;
}



bool 
StationInfo::
hasTypeId(int typeID, int hour)const
{
   CITTypeList it=typepriority_.begin();

   //This is an error, we silent ignore it, and returns all typeids.
   if(hour>23)
      hour=-1;

   for( ; it!=typepriority_.end(); it++){
      if(typeID==it->typeID() && (hour<0 || it->hour(hour)))
         return true;
   }

   return false;
}

bool 
StationInfo::
mustHaveType( int typeid_, int hour )const
{
   TLongList   ret;
   CITTypeList it=typepriority_.begin();

   //This is an error, we silent ignore it, and returns all typeids.
   if(hour>23)
      hour=-1;

   for(;it!=typepriority_.end(); it++) {
      if( typeid_ == it->typeID() ) {
         if(it->mustHaveType() && (hour<0 || it->hour(hour)))
            return true;
         else
            return false;
      }
   }

   return false;
}

bool
StationInfo::
msgForTime( const miutil::miTime &t)const
{
   if( t.undef() )
      return false;

   TLongList tp=typepriority( t.hour() );

   //Check to see if we have any types for this hour.
   if(tp.size()==0)
      return false;

   for(CITDelayList it=delayList_.begin();
         it!=delayList_.end();
         ++it)
      if(it->skipMsgSpec())
         return it->msgForThisTime( t );

   //TODO: For now. Only return true if the obstime is on a full hour.

   return t.min()==0;
   //return true;
}

std::string
StationInfo::
codeToString()const
{
   if( code_.empty() )
      return "";

   ostringstream o;

   IntList::const_iterator it = code_.begin();
   o << *it;
   ++it;

   for( ; it != code_.end(); ++it )
      o << " " << *it;

   return o.str();
}

int
StationInfo::
delay( const miutil::miTime &t,
       bool &force,  bool &relativToFirst)const
{
   if( t.undef() )
      return 0;

   bool         stime=t.hour()%3==0; //Is it a WMO standard time.
   CITDelayList it=delayList_.begin();

   relativToFirst=false;

   if(it==delayList_.end())
      return false;

   for( ;it!=delayList_.end(); it++){
      if(it->hour()==t.hour()){
         break;
      }else if(it->hour()<0){
         if(stime){
            if(it->hour()==DelayInfo::STIME){
               break;
            }else if(it->hour()==DelayInfo::FSTIME || it->hour()==DelayInfo::FHTIME){
               relativToFirst=true;
               break;
            }
         }if ( it->hour() == DelayInfo::FHTIME){
            relativToFirst=true;
         }else if(it->hour()==DelayInfo::HTIME)
            break;
      }
   }

   if(it!=delayList_.end()){
      force=it->force();
      return it->delay();
   }

   return 0;
}

std::string
StationInfo::
toIdentString()const
{
   ostringstream o;
   if( wmono_ > 0 )
      o << "wmo_" << wmono_;
   else if ( stationID_ > 0 )
      o << "id_"<< stationID_;
   else if( ! callsign_.empty() )
      o << "callsign_" << callsign_;
   else
      o << "id_UNKNOWN";

   return o.str();

}
bool 
StationInfo::
equalTo(const StationInfo &st)
{
   if(&st==this)
      return true;

   if(wmono_==st.wmono_                 &&
         stationID_ == st.stationID_       &&
         callsign_ == st.callsign_         &&
         name_ == st.name_                 &&
         code_ == st.code_                 &&
         definedStationid_==st.definedStationid_ &&
         typepriority_==st.typepriority_   &&
         precipitation_==st.precipitation_ &&
         delayList_==st.delayList_         &&
         list_==st.list_                   &&
         copy_==st.copy_                   &&
         copyto_==st.copyto_               &&
         owner_==st.owner_                 &&
         loglevel_==st.loglevel_           &&
         heightPrecip_ == st.heightPrecip_ &&
         heightPressure_ == st.heightPressure_ &&
         heightTemperature_ == st.heightTemperature_ &&
         heightVisability_ == st.heightVisability_ &&
         heightWind_ == st.heightWind_ &&
         heightWindAboveSea_ == st.heightWindAboveSea_ &&
         height_ == st.height_ &&
         latitude_ == st.latitude_ &&
         longitude_ == st.longitude_ )
      return true;
   else
      return false;
}


bool
StationInfo::
operator<(const StationInfo &rhs )const
{
   if( ( wmono_ < rhs.wmono_ ) ||
         ( wmono_ == rhs.wmono_ && stationID_ < rhs.stationID_ ) ||
         ( wmono_ == rhs.wmono_ && stationID_ == rhs.stationID_ && callsign_ < rhs.callsign_ ) )
      return true;
   else
      return false;
}

bool
StationInfo::
operator==(const StationInfo &rhs) const
{
   if( wmono_ == rhs.wmono_ &&
         stationID_ == rhs.stationID_ &&
         callsign_ == rhs.callsign_ )
      return true;
   else
      return false;
}

std::string 
StationInfo::
keyToString(const std::string &key)
{
   ostringstream ost;

   if( key == "callsign" ) {
      return callsign_;
   } else if( key == "code") {
      for( IntList::const_iterator it=code_.begin();
            it != code_.end(); ++it ) {
         if( it != code_.begin() )
            ost << " ";
         ost << *it;
      }
      return ost.str();
   } else if(key=="wmono"){
      ost << wmono();
      return ost.str();
   }else  if(key=="name"){
      return name();
   }else  if(key=="stationid"){
      bool first=true;
      for(StationInfo::CITLongList it=definedStationid_.begin();
            it!=definedStationid_.end(); it++){
         if(first)
            first=false;
         else
            ost << " ";

         ost << *it;
      }
      return ost.str();
   }else if(key=="typepriority"){
      bool first=true;

      for(StationInfo::CITTypeList it=typepriority_.begin();
            it!=typepriority_.end();it++){
         if(first)
            first=false;
         else
            ost << " ";

         ost << it->typeID();
      }

      return ost.str();
   }else if(key=="mustHaveTypes"){
      bool first=true;
      for(StationInfo::CITTypeList it=typepriority_.begin();
            it!=typepriority_.end();it++){
         if(!it->mustHaveType())
            continue;

         if(first)
            first=false;
         else
            ost << " ";

         ost << it->typeID();
      }

      return ost.str();
   }else if(key=="precipitation"){
      bool first=true;

      for(StationInfo::CITStringList it=precipitation_.begin();
            it!=precipitation_.end();it++){
         if(first)
            first=false;
         else
            ost << " ";

         ost << *it;
      }

      return ost.str();
   }else if(key=="delay"){
      if( !delayConf.empty() ) {
         string val = delayConf;
         miutil::replace( val, "(", "");
         miutil::replace( val, ")", "");
         miutil::replace( val, "\"", "");
         miutil::replace( val, ",", " ");
         return val;
      }

      bool first=true;

      for(StationInfo::CITDelayList it=delayList_.begin();
            it!=delayList_.end();it++){
         if(first)
            first=false;
         else
            ost << " ";
         ost << *it;
      }

      string val=ost.str();
      string::size_type ii=val.find_first_of('\"');

      while(ii!=string::npos){
         val.erase(ii, 1);
         ii=val.find_first_of('\"');
      }

      return val;
   }else if(key=="list"){
      ost << list_;
      return ost.str();
   }else if(key=="owner"){
      ost << owner_;
      return ost.str();
   }else if(key=="loglevel"){
      ost << loglevel_;
      return ost.str();
   } else if( key == "height" ) {
      ost << height_;
      return ost.str();
   } else if( key == "height_visibility" ) {
      ost << heightVisability();
      return ost.str();
   } else if( key == "height_precip" ) {
      ost << heightPrecip();
      return ost.str();
   } else if( key == "height_pressure" ) {
      ost << heightPressure();
      return ost.str();
   } else if( key == "height_temperature" ) {
      ost << heightTemperature();
      return ost.str();
   } else if( key == "height_wind" ) {
      ost << heightWind();
      return ost.str();
   } else if( key == "height_wind_above_sea" ) {
       ost << heightWindAboveSea();
       return ost.str();
   } else if( key == "latitude" ) {
      ost << latitude_;
      return ost.str();
   } else if( key == "longitude" ) {
      ost << longitude_;
      return ost.str();
   }


   return string();
}

std::ostream&
operator<<( std::ostream &o, const MsgTime &mt )
{
   if( ! mt.minForEachHour() ) {
      if( mt.count() == 24 ) {
         o << "*";
      } else if( mt.count() == 0 ) {
         o << "(none)";
         return o;
      } else {
         bool first=true;
         for( int i=0; i<24; ++i ) {
            if( mt.hours_.test( i ) ) {
               o << (first?"":",") << i;
               first=false;
            }
         }
      }
      bool first=true;
      o << ":";
      for( int i=0; i<60; ++i ) {
         if( mt.minAtHour_[0].test( i ) ) {
            o << (first?"":",") << i;
            first=false;
         }
      }
   } else {
      MsgTime::Minutes m;
      bool newLine=false;

      for( int i = 0; i<24; ++i ) {
         if( newLine ) o << endl;
         m = mt.minAtHour( i );

         if( m.count() == 0 )
            continue;

         newLine = true;
         bool first=true;

         o << i << ":";
         for( int i=0; i<60; ++i ) {
            if( m.test( i ) ) {
               o << (first?"":",") << i;
               first=false;
            }
         }
      }
   }
   return o;
}


std::ostream& 
operator<<(std::ostream& ost,
           const StationInfo& sd)
{
   ost << "                name: " << sd.name() << endl;
   ost << "                code:";
   for(StationInfo::IntList::const_iterator it=sd.code_.begin();
         it!=sd.code_.end(); it++)
      ost << " " << *it;

   ost << endl;

   ost << "         StationInfo: " << sd.wmono() << endl;

   ost << "defined stationid(s): ";


   for(StationInfo::CITLongList it=sd.definedStationid_.begin();
         it!=sd.definedStationid_.end(); it++)
      ost << *it << " ";

   ost << endl;


   ost << "        typepriority:";
   for(StationInfo::CITTypeList it=sd.typepriority_.begin();
         it!=sd.typepriority_.end();it++)
      ost << " " << *it;

   ost << endl;
   ost << "      stationid: " << sd.stationID() << endl;
   ost << "       callsign: " << sd.callsign() << endl;;
   ost << "          wmono: " ;

   if( sd.wmono()<0 )
      ost << "NA";
   else
      ost << sd.wmono();
   ost << endl;

   ost << "  mustHaveTypes: ";
   for(StationInfo::CITTypeList it=sd.typepriority_.begin();
         it!=sd.typepriority_.end();it++)
      if(it->mustHaveType())
         ost << it->typeID() << " ";

   ost << endl;

   ost << "  precipitation: ";
   for(StationInfo::CITStringList it=sd.precipitation_.begin();
         it!=sd.precipitation_.end();it++)
      ost << *it << " ";

   ost << endl;

   ost << "          delay: ";
   if( ! sd.delayConf.empty() ) {
      string val = sd.delayConf;
      miutil::replace( val, "(", "");
      miutil::replace( val, ")", "");
      miutil::replace( val, "\"", "");
      miutil::replace( val, ",", " ");
      ost << val;
   } else {
      for(StationInfo::CITDelayList it=sd.delayList_.begin();
            it!=sd.delayList_.end();it++)
         ost << *it << " ";
   }

   ost << endl;

   ost << "     delayUntil: ";
   if(sd.delayUntil_.undef())
      ost << "UNDEF";
   else
      ost << sd.delayUntil_;

   ost << endl;
   ost << "             latitude: " << printOut( sd.latitude_) << endl;
   ost << "            longitude: " << printOut( sd.longitude_ )<< endl;
   ost << "               height: " << printOut( sd.height_ ) << endl;
   ost << "    height_visibility: " << printOut( sd.heightVisability() ) << endl;
   ost << "        height_precip: " << printOut( sd.heightPrecip() )<< endl;
   ost << "      height_pressure: " << printOut( sd.heightPressure() ) << endl;
   ost << "   height_temperature: " << printOut( sd.heightTemperature() )<< endl;
   ost << "          height_wind: " << printOut( sd.heightWind() ) << endl;
   ost << "height_wind_above_sea: " << printOut( sd.heightWindAboveSea() ) << endl;
   ost << "                 list: " << sd.list_ << endl;
   ost << "                 copy: " << (sd.copy_?"TRUE":"FALSE") << endl;
   ost << "               copyto: " << sd.copyto_ << endl;
   ost << "                owner: " << sd.owner_ << endl;
   ost << "           delayLogic: " << (!sd.delayList_.empty()?"TRUE":"FALSE")
      	      << endl;
   ost << "             loglevel: " << sd.loglevel_ << endl;

   return ost;
}

std::ostream& 
operator<<(std::ostream& ost,
           const DelayInfo& sd)
{
   if(sd.undef()){
      ost << "\"*** UNDEFINED ***\"";
      return ost;
   }

   if(sd.skipMsgSpec()){
      ost << *sd.msgtimes_;
      return ost;
   }

   if(sd.hour()>=0){
      ost << "\"" << sd.hour() << ":" << sd.delay() << "\"";
   }else{
      if(sd.force()){
         ost << "\"F";
         if(sd.hour()==DelayInfo::STIME){
            ost << "S:";
         }else{
            ost << "H:";
         }
      }else{
         if(sd.hour()==DelayInfo::STIME){
            ost << "\"SS:";
         }else if(sd.hour()==DelayInfo::FSTIME){
            ost << "\"fS:";
         }else if(sd.hour()==DelayInfo::FHTIME){
            ost << "\"fH:";
         }
         else{
            ost << "\"HH:";
         }
      }

      ost << sd.delay();

      if(sd.force()){
         ost << " Force: true\"";
      }else{
         ost << " Force: false\"";
      }
   }

   return ost;
}


std::ostream& 
operator<<(std::ostream& ost, const StationInfo::Type& t)
{
   ost << t.typeid_ << ":[";

   for(size_t i=0; i<t.hours.size(); i++)
      ost << (t.hours[i]?'1':'0');

   ost << "]";
   return  ost;
}


StationList::
StationList()
{
}

StationList::
~StationList()
{
}

StationInfoList
StationList::
findStation( int stationid )const
{
   StationInfoList ret;
   for( StationList::const_iterator it=begin();
         it != end(); ++it ) {
      if((*it)->hasDefinedStationId(stationid)){
         ret.push_back( *it );
      }
   }

   return ret;
}

StationInfoList
StationList::
findStationByWmono( int wmono )const
{
   StationInfoList ret;

   if( wmono > 0 ) {
      for( StationList::const_iterator it=begin();
            it != end(); ++it ) {
         if( (*it)->wmono() == wmono ){
            ret.push_back( *it );
         }
      }
   }

   return ret;
}

StationInfoList
StationList::
findStationById( int id )const
{

   StationInfoList ret;

   if( id > 0 ) {
      for( StationList::const_iterator it=begin();
            it != end(); ++it ) {
         if( (*it)->stationID() == id ){
            ret.push_back( *it );
         }
      }
   }

   return ret;
}

StationInfoList
StationList::
findStationByCallsign( const std::string &callsign )const
{
   StationInfoList ret;

   if( ! callsign.empty() ) {
      for( StationList::const_iterator it=begin();
            it != end(); ++it ) {
         if( (*it)->callsign() == callsign ){
            ret.push_back( *it );
         }
      }
   }

   return ret;
}



StationInfoCompare::
StationInfoCompare( const StationList &removedStations,
                    const StationList &newStations,
                    const StationList &changedStations )
: removedStations_( removedStations ),
  newStations_( newStations ),
  changedStations_( changedStations )
{
}

StationInfoCompare::
StationInfoCompare()
{
}

StationInfoCompare::
StationInfoCompare( const StationInfoCompare &s )
: removedStations_( s.removedStations_ ),
  newStations_( s.newStations_ ),
  changedStations_( s.changedStations_ )
{
}


StationInfoPtr
StationInfoCompare::
findStation( const StationList &stationList, StationInfoPtr station )
{
   for( StationList::const_iterator it=stationList.begin(); it != stationList.end(); ++it ) {
      if( **it == *station ){
         return *it;
      }
   }

   return StationInfoPtr();
}

StationInfoCompare&
StationInfoCompare::
operator=( const StationInfoCompare &rhs )
{
   if( &rhs != this ) {
      removedStations_ = rhs.removedStations_;
      newStations_ = rhs.newStations_;
      changedStations_ = rhs.changedStations_;
   }

   return *this;
}

StationInfoCompare
StationInfoCompare::
compare( const StationList &oldConf, const StationList &newConf  )
{
   StationList removedStations;
   StationList newStations;
   StationList changedStations;
   StationInfoPtr station;

   //Find all the removed stations.
   for( StationList::const_iterator it=oldConf.begin(); it != oldConf.end(); ++it ) {
      station = findStation( newConf, *it );

      if( !station )
         removedStations.push_back( *it );
   }

   //Find all the new and changed stations.
   for( StationList::const_iterator it=newConf.begin(); it != newConf.end(); ++it ) {
      station = findStation( oldConf, *it );

      if( !station )
         newStations.push_back( *it );
      else if( ! station->equalTo( *(*it) ) )
         changedStations.push_back( *it );
   }

   return StationInfoCompare( removedStations, newStations, changedStations );
}


