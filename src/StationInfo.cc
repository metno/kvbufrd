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
#include "StationInfo.h"

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
}


StationInfo::
StationInfo():
   height_( INT_MAX ),
   heightVisability_( INT_MAX ),
   heightTemperature_( INT_MAX ),
   heightPressure_( INT_MAX ),
   heightPrecip_( INT_MAX ),
   heightWind_( INT_MAX ),
   latitude_( FLT_MAX ),
   longitude_( FLT_MAX ),
  	wmono_(-1),
  	cacheReloaded48_(true)
{
}

StationInfo::
StationInfo(const StationInfo &i)
{
   height_ = i.height_;
   heightVisability_ = i.heightVisability_;
   heightTemperature_ = i.heightTemperature_;
   heightPressure_ = i.heightPressure_;
   heightPrecip_ = i.heightPrecip_;
   heightWind_ = i.heightWind_;
   latitude_ = i.latitude_;
   longitude_ = i.longitude_;
   wmono_=i.wmono_;
   name_ = i.name_;
  	stationid_=i.stationid_;
  	typepriority_=i.typepriority_;
  	precipitation_=i.precipitation_;
  	delayList_=i.delayList_;
  	list_=i.list_;
  	copy_=i.copy_;
  	copyto_=i.copyto_;
  	owner_=i.owner_;
  	delayUntil_=i.delayUntil_;
  	cacheReloaded48_=i.cacheReloaded48_;
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
   if( heightVisability_ != INT_MAX )
      return heightVisability_;

   //Average height of male norwegians military recruits was 179.9 cm in 2006. ssb.no
   return 1.8;
}

int
StationInfo::
heightTemperature()const
{
   if( heightTemperature_ != INT_MAX )
      return  heightTemperature_;


   return 2;
}

int
StationInfo::
heightPressure() const
{
   if( heightPressure_ != INT_MAX )
      return heightPressure_;

   return heightAdd( 2 );
}

int
StationInfo::
heightPrecip()const
{
   if( heightPrecip_ != INT_MAX )
      return heightPrecip_;

   return 2;
}

int
StationInfo::
heightWind()const
{
   if( heightWind_ != INT_MAX )
      return heightWind_;

   return 10;
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

bool      
StationInfo::
hasStationId(long stid)const
{
  	CITLongList it=stationid_.begin();

  	for( ; it!=stationid_.end(); it++){
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
bufrForTime(int hh)const
{
	TLongList tp=typepriority(hh);
	
	//Check to see if we have any types for this hour.
	if(tp.size()==0)
		return false;	

	for(CITDelayList it=delayList_.begin();
		 it!=delayList_.end(); 
		 ++it)
  		if(it->skipBufrSpec())
  			return it->bufrForThisHour(hh);
	
    return true;
}

bool 
StationInfo::
delay(int hour, int &minute, 
      bool &force,  bool &relativToFirst)const
{
   bool         stime=hour%3==0; //Is it a bufr time.
   CITDelayList it=delayList_.begin();

   relativToFirst=false;
   minute=-1;

   if(it==delayList_.end())
      return false;

   for( ;it!=delayList_.end(); it++){
      if(it->hour()==hour){
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
      minute=it->delay();
      return true;
   }

   return false;
}

bool 
StationInfo::
equalTo(const StationInfo &st)
{
  	if(&st==this)
    	return true;

  	if(wmono_==st.wmono_                 &&
  	   name_ == st.name_                  &&
       stationid_==st.stationid_         &&
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
       height_ == st.height_ &&
       latitude_ == st.latitude_ &&
       longitude_ == st.longitude_ )
     	return true;
  	else
    	return false;
}



std::string 
StationInfo::
keyToString(const std::string &key)
{
  	ostringstream ost;

  	if(key=="wmono"){
    	ost << wmono();
    	return ost.str();
  	}else  if(key=="name"){
  	   return name();
  	}else  if(key=="stationid"){
    	bool first=true;
    	for(StationInfo::CITLongList it=stationid_.begin(); 
			it!=stationid_.end(); it++){
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
  	} else if( key == "height-visibility" ) {
      ost << heightVisability();
      return ost.str();
   } else if( key == "height-precip" ) {
      ost << heightPrecip();
      return ost.str();
   } else if( key == "height-pressure" ) {
      ost << heightPressure();
      return ost.str();
   } else if( key == "height-temperature" ) {
      ost << heightTemperature();
      return ost.str();
   } else if( key == "height-wind" ) {
      ost << heightWind();
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
operator<<(std::ostream& ost,
	   const StationInfo& sd)
{
  	ost << "StationInfo: " << sd.wmono() << endl;
  	ost << "           name: " << sd.name() << endl;
  	ost << "      stationid: ";
  
  	for(StationInfo::CITLongList it=sd.stationid_.begin(); 
    	it!=sd.stationid_.end(); it++)
    	ost << *it << " ";
    	
  	ost << endl;

  	ost << "   typepriority: ";
  	for(StationInfo::CITTypeList it=sd.typepriority_.begin(); 
      	it!=sd.typepriority_.end();it++)
    	ost << *it << " ";

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
  	for(StationInfo::CITDelayList it=sd.delayList_.begin();
      	it!=sd.delayList_.end();it++)
    	ost << *it << " ";

	ost << endl;

  	ost << "     delayUntil: ";
  	if(sd.delayUntil_.undef())
    	ost << "UNDEF";
  	else 
    	ost << sd.delayUntil_;

  	ost << endl;
   ost << "           latitude: " << printOut( sd.latitude_) << endl;
   ost << "          longitude: " << printOut( sd.longitude_ )<< endl;
  	ost << "             height: " << printOut( sd.height_ ) << endl;
  	ost << "  height-visibility: " << printOut( sd.heightVisability() ) << endl;
   ost << "      height-precip: " << printOut( sd.heightPrecip() )<< endl;
   ost << "    height-pressure: " << printOut( sd.heightPressure() ) << endl;
   ost << " height-temperature: " << printOut( sd.heightTemperature() )<< endl;
   ost << "        height-wind: " << printOut( sd.heightWind() ) << endl;
  	ost << "               list: " << sd.list_ << endl;
  	ost << "               copy: " << (sd.copy_?"TRUE":"FALSE") << endl;
  	ost << "             copyto: " << sd.copyto_ << endl;
  	ost << "              owner: " << sd.owner_ << endl;
  	ost << "        delayLogic: " << (!sd.delayList_.empty()?"TRUE":"FALSE")
      	<< endl;
  	ost << "           loglevel: " << sd.loglevel_ << endl;

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

   if(sd.skipBufrSpec()){
      bool first=true;
      for(int i=0; i<23; i++){
         if(!sd.bufrtimes_[i]){
            if(first){
               ost << "\"-" << i;
               first=false;
            }else{
               ost << ",-" << i;
            }
         }
      }

      if(!first)
         ost << "\"";

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

