/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfo.h,v 1.13.2.8 2007/09/27 09:02:22 paule Exp $                                                       

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
/* $Header: /var/lib/cvs/kvalobs/src/kvbufrd/StationInfo.h,v 1.13.2.8 2007/09/27 09:02:22 paule Exp $ */

#ifndef __StationInfo_h__
#define __StationInfo_h__

#include <bitset>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <miconfparser/miconfparser.h>
#include <milog/milog.h>
#include <list>
#include <string>
#include <puTools/miTime.h>

//#include "StationInfoParse.h"



class StationInfoParse; 

/**
 * parseStationInfo, parse a stationinfo section in the
 * configuration file.
 * 
 * <pre>
 * Recognized keys:
 *   stationid [list]: stationid, may be a list of stationid if
 *                     the wmo message is created from data from
 *                     diffrent stationid.
 *   delay [list]: a list of delay specifications. A spesification
 *                 may have the following diffrent forms.
 *                 
 *                 tt:mm  - tt is an hour in the range [0,23].
 *                          mm is an minute in the range [0, 59]
 *                 Ftt:mm - tt is an hour in the range [0,23].
 *                          mm is an minute in the range [0, 59],
 *                          F - force.
 *                 SS:mm  - SS specify that  mm is for all
 *                          bufrtimes, ie. 0,3,6,9,12,15,18 og 21
 *                 HH:mm  - HH specify that mm is for all hours.
 *                 FS:mm  - Force a delay for all bufrtimes.
 *                 FH:mm  - Force a delay for all hours.
 *                 fS:mm  - Delay max mm minutes after the first data is 
 *                          received for all bufrtimes.
 *                 fH:mm  - Delay max mm minutes after the first data is
 *                          received for all hours.
 *                 -SS    - Dont generate bufr for bufrtimes.
 *                 -tt    - Dont generate bufr for the hour tt, where
 * 	                        tt is in the range [0,23].
 * 		
 *                 
 *                 ex: delay=("6:10", "SS:03")
 *                     This means that we shall delay with 3 minutes for all 
 *                     bufrtimes except for the 6 termin that we shall delay
 *                     with 10 minutes.
 *
 *                     delay=("6:10", "SS:03", "HH:01")
 *                     This means that we shall delay all termins with 1 
 *                     minute, except for bufrtimes (except the 6 termin
 *                     that shal be delayed with 6 minutes) that shall delay
 *                     with  minutes.
 *                          
 *                     delay=("fS:03")
 *                     if the station is to receive data for multiple typeid's
 *                     delay the SYNOP production with 3 minutes after the 
 *                     first typeid is received. 
 * 
 *   precipitation [list]: Specify which parameter shall be used for
 *                         precipitation. Valid values "RA", "RR_01",
 *                         "RR_1", "RR_3", "RR_6","RR_12" og "RR_24".
 *                          
 *   typepriority [list]: A list of typeis's that is used to create a
 *                        wmo message.
 * </pre>
 */

class DelayInfo
{
  	//hour have several value:
  	// [0,23] the termin (hour) to delay.
  	//  -1 specify only bufrtimes ie. 0, 3, 6, 9, 12, 15, 18, 21
  	//  -2 specify all hours
  
  	char hour_;  
  	char delay_; //minutes
  	bool force_;
  	bool *msgtimes_;  //Create bufr for this times.

	void initMsgTimes(){
		
			//std::cerr << "initBufrTimes: " << int(hour_) << std::endl;
			if(!msgtimes_){
				try{
					msgtimes_=new bool[24];
				}
				catch(...){
					return;
				}
			}
		
			for(int i=0; i<24; i++)
				msgtimes_[i]=true;
	}

public:
  	enum {STIME=-1, HTIME=-2, FSTIME=-3, FHTIME=-3,SKIP_SYNOP=127, UNDEF=-128};

  	DelayInfo(int hour=UNDEF)
    	:hour_(hour), delay_(0), force_(false), msgtimes_(0){
    		if(hour==SKIP_SYNOP){
    			//std::cerr << "DelayInfo::CTOR: SKIP_SYNOP!" << std::endl;
    			initMsgTimes();
    		}
    	}
  	DelayInfo(char hour, char delay, bool force)
    	: hour_(hour), delay_(delay), force_(force), msgtimes_(0){}
  	DelayInfo(const DelayInfo &d)
    	:hour_(d.hour_),delay_(d.delay_),force_(d.force_), msgtimes_(0){
    		if(d.msgtimes_){
   				initMsgTimes();
    				    		
    			if(msgtimes_){
    				for(int i=0; i<24; i++)
    					msgtimes_[i]=d.msgtimes_[i];
    			}
    		}
    	}
    ~DelayInfo(){
    	if(msgtimes_)
    		delete msgtimes_;
    }

  	DelayInfo& operator=(const DelayInfo &rhs){
    	if(this!=&rhs){
      		hour_=rhs.hour_;
      		delay_=rhs.delay_;
      		force_=rhs.force_;
      
      		if(rhs.msgtimes_){
    			if(!msgtimes_)
    				initMsgTimes();
    				    		
    			if(msgtimes_){
    				for(int i=0; i<24; i++)
    					msgtimes_[i]=rhs.msgtimes_[i];
    			}
    		}else if(msgtimes_){
    			delete msgtimes_;
    			msgtimes_=0;
      		}
    			
    	}
    	return *this;
  	}
  
  
  	bool operator==(const DelayInfo &di){
    	return equal(di);
  	}

  	bool operator==(const DelayInfo &di)const{
    	return equal(di);
  	}
  

  	bool equal(const DelayInfo &di)const{
			if(this==&di)
        		return true;
       
       		if(hour_==di.hour_   &&
	     	   delay_==di.delay_ &&
	     	   force_==di.force_){
	   
	   			if(msgtimes_ || di.msgtimes_){
	   				if(msgtimes_ && di.msgtimes_){
	     	   			for(int i=0; i<24; i++){
    						if(msgtimes_[i]!=di.msgtimes_[i])
    							return false;
	     	   			}
	   				}else{
	   					return false;
	   				}
	   			}
	     	   	   	   	
	    		return true;
	    	}else{
	    		return false;
	    	}
  	}

	bool skipMsgSpec()const{ return msgtimes_!=0;}
  	bool undef()const { return hour_==UNDEF;}
  	int  hour()const{ return static_cast<int>(hour_); }
  	int  delay()const{ return static_cast<int>(delay_);}
  	bool force()const{ return force_;}
  
  	//Shall we generate bufr for this hour
  	bool msgForThisHour(int hour)const{
			if(!msgtimes_)
				return true;

  			if(hour<0 || hour>23)
  				return false;
  		
  			return msgtimes_[hour];
  	}
  
  	void msgForThisHour(int hour, bool flag){
  		//Must be a SKIP_SYNOP spec.
  		if(msgtimes_){
  			if(hour<0 || hour>23)
  				return;
  				  			
  			msgtimes_[hour]=flag;
  		}
  	}
  	
  	
  	friend std::ostream& operator<<(std::ostream& ost,
									  const DelayInfo& sd);
};

class StationInfo
{
 public:
 
 	/**
 	 * Type is a class to hold information about one typeid 
 	 * and for witch hours it shall be used and if it is a 
 	 * must have type, ie we cant create a SYNOP wuithout it.
 	 * 
 	 * We use a bitset to hold the time information, bit 0-23 is
 	 * for hour 00-23. Bit 24 is the must have information.
 	 *   
 	 */
 	class Type{
 			long typeid_;
 			std::bitset<25> hours;
 		
 		public:
 			Type():typeid_(0){
 				hours.set();
 				hours.set(24, false);
 			}
 	
 			Type(long t):typeid_(t){
 				hours.set();
 				hours.set(24, false);
 			}
 	
 			
 			Type(const Type &t):typeid_(t.typeid_), hours(t.hours){}

	  		Type& operator=(const Type &rhs){
	  			if(this!=&rhs){
	  				typeid_=rhs.typeid_;
	  				hours==rhs.hours;
	  			}
	  			
	  			return *this;
	  		}
 			
 			
 			bool operator==(const Type &t){
    			return equal(t);
  			}

  			bool operator==(const Type &t)const{
    			return equal(t);
  			}
  

  			bool equal(const Type &t)const{
				if(this!=&t){
					if(typeid_!=t.typeid_)
						return false;
						
					if(hours!=t.hours)
						return false;
						
					return true;
				}
				
				return false;
  			}	       
 			 			
 			long typeID()const{ return typeid_;}
 			bool hour(int h)const { return hours.test(h);}
 			void hour(int h, bool value){ hours.set(h, value);}
 			bool mustHaveType()const{ return hours.test(24);}
 			void mustHaveType(bool v){ hours.set(24, v);}
 			
 			friend std::ostream& operator<<(std::ostream& ost,
				  const StationInfo::Type& sd);
 	};
 	
 
 	typedef std::list<Type>                            TTypeList;
  	typedef std::list<Type>::iterator                 ITTypeList;
  	typedef std::list<Type>::const_iterator          CITTypeList;
  	typedef std::list<Type>::reverse_iterator        RITTypeList;
  	typedef std::list<Type>::const_reverse_iterator CRITTypeList;
 				
 
  typedef std::list<long>                   TLongList;
  typedef std::list<long>::iterator        ITLongList;
  typedef std::list<long>::const_iterator CITLongList;
  typedef std::list<long>::reverse_iterator        RITLongList;
  typedef std::list<long>::const_reverse_iterator CRITLongList;

  typedef std::list<std::string>                   TStringList;
  typedef std::list<std::string>::iterator        ITStringList;
  typedef std::list<std::string>::const_iterator CITStringList;

  typedef std::list<DelayInfo>                   TDelayList;
  typedef std::list<DelayInfo>::iterator        ITDelayList;
  typedef std::list<DelayInfo>::const_iterator CITDelayList;
  
 private:
  friend class StationInfoParse;
  friend class ConfMaker;

  StationInfo& operator=(const StationInfo &);
  int            height_;
  int            heightVisability_;
  int            heightTemperature_;
  int            heightPressure_;
  int            heightPrecip_;
  int            heightWind_;
  float          latitude_;
  float          longitude_;
  int            wmono_;
  std::string    name_;
  TLongList      stationid_;
  TTypeList      typepriority_;
  TStringList    precipitation_;
  TDelayList     delayList_;
  std::string    list_;
  bool           copyIsSet_;
  bool           copy_;
  std::string    copyto_;
  std::string    owner_;
  miutil::miTime delayUntil_;
  static std::string  debugdir_;
  milog::LogLevel loglevel_;  
  bool            cacheReloaded48_;
  

  StationInfo();

 public:
  StationInfo( int wmono );
  StationInfo(const StationInfo &);

  ~StationInfo();
  
  std::string delayConf; //Holds the configuration line for the delay from the configuration file.
                         //This information is hard to reconstruct from the delayInfo.

  std::string    list()const   { return list_; }
  void           list(const std::string &l){ list_=l;}
  bool           isCopySetInConfSection() const { return copyIsSet_; }
  bool           copy()const   { return copy_; }
  void           copy(bool c)  { copy_=c;}
  std::string    copyto()const { return copyto_; }
  void           copyto(const std::string &c2){ copyto_=c2;}
  std::string    owner()const  { return owner_;}
  void           owner(const std::string &o){ owner_=o;}
  milog::LogLevel loglevel()const { return loglevel_;}


  int       height()const{ return height_; }
  int       heightAdd( int ammount )const;
  void      height( int h, bool ifUnset = true );
  int       heightVisability() const;
  int       heightTemperature()const;
  int       heightPressure() const;
  int       heightPrecip()const;
  int       heightWind()const;
  float     latitude()const { return latitude_; }
  void      latitude( float lat, bool ifUnset=true );
  float     longitude()const { return longitude_; }
  void      longitude( float lon, bool ifUnset=true );
  int       wmono()const{ return wmono_;}
  std::string name()const{ return name_; }
  void      name( const std::string &n, bool ifUnset=true );

  TLongList stationID()const { return stationid_;}
  bool      hasStationId(long stid)const;

  static std::string debugdir() { return debugdir_;}

  TStringList precipitation()const { return precipitation_;}
  bool        hasPrecipitation()const{ return !precipitation_.empty();}

  /**
   * \brief A list of \em typeid to use data for when encoding SYNOP.
   * 
   * Typepriority plays two roles. Only data that is in this
   * list is used to create WMO message. The second use is
   * to select which Data that shall be used if there is more 
   * than one Data for one parameter. The Data is selected in the
   * order in the list. A Data with typeid preciding a another typeid
   * in the list is used before the other.
   *
   * \param hour Return the a list of typeid's that is valid for
   *             the hour \a hour. A negativ value means all typeids.
   * \return A list of \em typeid's.
   */ 
  TLongList typepriority(int hour=-1)const;


  /**
   * \brief return a list of typeid for this station that has
   *        an element in \a ctlist.
   */
  std::list<int> continuesTypeids(const std::list<int> &ctList)const;

  /**
   * \brief mustHaveTypes returns a list of \em typeid that there must be data 
   * for, before we can create a WMO message. 
   * 
   * We cant create a message if we doesnt have data from this types.
   * 
   * \param hour Return the a list of typeid's that is valid for
   *             the hour \a hour. A negativ value means all typeids.
   * \return A list of mustHaveTypes.
   */
  TLongList mustHaveTypes(int hour=-1)const;

  bool mustHaveType( int typeid_, int hour=-1 )const;


  /**
   * \brief Do we have typeID in the list of typeriority_.
   *
   * \param hour Return the a list of typeis's that is valid for
   *             the hour \a hour. A negativ value mins all typeids.
   * \return true if \em typeID is in the list of types we shall
   *         use data from when encoding SYNOP. false othewise.
   */
  bool hasTypeId(int typeID, int hour=-1)const;
  
  /**
   *  \brief Setter function to set the delay.
   */
  void delayUntil(const miutil::miTime &delay){ delayUntil_=delay; }

  /**
   * \brief Getter function to get the delay.
   */
  miutil::miTime delayUntil()const { return delayUntil_;}
  

  /**
   * \brief Checks if we shall delay for the hour if not
   * all data is received that shall be used to create the 
   * WMO message. 
   *
   * It return true if we shall delay, and the number of
   * minutes to delay is returned in the output parameter 'minute'.
   * If we shall not delay it return false.
   *
   * \param[in] hour to check for a delay.
   * \param minute[out] Number of minutes to delay.
   * \param force[out] True if we shall allways delay, even if all
   *        data that is required.
   * \param relativToFirst[out] Is set to true on return if the delay
   *                            shall be relativ to the first typeid received
   *                            data for for a given bufr time.
   * \return True if we shall delay and false otherwise.
   */
  bool delay(int hour, int &minute, bool &force, bool &relativToFirst)const;
  
  bool msgForTime(int hh)const;

  friend std::ostream& operator<<(std::ostream& ost,
				  const StationInfo& sd);

  std::string keyToString(const std::string &key);

  bool makeConfSection( std::string &confSection )const;
  /**
   * \brief Compare two StationInfo to se if they has the same 
   *  configuration data.
   *
   *  The configuration data that is comapared is the information
   *  that is specified in the configuration file.
   * 
   *  \param st The StationInfo to compare against this.
   *  \return true if they are equal and false otherwise.
   */
  bool equalTo(const StationInfo &st);
  bool cacheReloaded48()const{ return cacheReloaded48_;}
  void cacheReloaded48(bool reloaded){ cacheReloaded48_=reloaded;}
};

typedef boost::shared_ptr<StationInfo> StationInfoPtr;
typedef std::list<StationInfoPtr> StationList;

std::ostream& operator<<(std::ostream& ost,
			 const StationInfo& sd);

std::ostream& operator<<(std::ostream& ost,
			 const DelayInfo& sd);

std::ostream& operator<<(std::ostream& ost,
								  const StationInfo::Type& sd);


class StationInfoCompare
{

   StationList removedStations_;
   StationList newStations_;
   StationList changedStations_;

   StationInfoCompare( const StationList &removedStations,
                       const StationList &newStations,
                       const StationList &changedStations );
public:

   StationInfoCompare();
   StationInfoCompare( const StationInfoCompare &s );
   StationInfoCompare& operator=( const StationInfoCompare &rhs );

   static StationInfoPtr findStation( const StationList &stationList, StationInfoPtr station );
   static StationInfoCompare compare( const StationList &oldConf, const StationList &newConf );

   StationList removedStations()const { return removedStations_; }
   StationList newStations()const { return newStations_; }
   StationList changedStations()const { return changedStations_; }

   bool isConfChanged()const { return ! removedStations_.empty() || ! newStations_.empty() || ! changedStations_.empty(); }

};

#endif
