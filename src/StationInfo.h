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
#include <list>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "miconfparser/miconfparser.h"
#include "milog/milog.h"


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
*                          standard WMO times, ie. 0,3,6,9,12,15,18 og 21
*                 HH:mm  - HH specify that mm is for all hours.
*                 FS:mm  - Force a delay for all standard WMO times.
*                 FH:mm  - Force a delay for all hours.
*                 fS:mm  - Delay max mm minutes after the first data is
*                          received for all standard WMO times.
*                 fH:mm  - Delay max mm minutes after the first data is
*                          received for all hours.
*                 -SS    - Dont generate bufr for standard WMO times.
*                 -tt    - Dont generate bufr for the hour tt, where
* 	                        tt is in the range [0,23].
*
*                 +HH:mm - Generate bufr for all hours with an obstime
*                          where minutes equals mm.
*                 +SS:mm - Generate bufr for all standard WMO times at obstime
*                          where minutes equals mm.
*                 +hh:mm - Only generate bufr where obstime has an hour == hh and
*                          minute == mm.
*
*                Alternative way to specify times when -/+ is used in front.
*                Use a list of times for each of hh and mm.
*                Ex.
*                   0,3:0 create a BUFFR message at 0 and 3 at full hour.
*                   0,3:0,10 create a BUFFR message at 0 and 3 at full hour and
*                            10 past full hour.
*
*                We can also use start/step specification.
*                Ex.
*                   0/3:0 possibly create a BUFR at 0,3,6,9,12,15,18 and 21 at full
*                         hour. 0/3 is the same as +SS.
*                   3/3:0/10 possibly create a BUFR at 3,6,9,12,15,18 and 21 at full
*                         hour every 10 minutes.
*
*                Use * to mean all.
*                Ex.
*                 *:* possibly create a BUFR message at all hours at all minutes.
*                 *:0 possibly create a BUFR message at all hours at full hour.
*                     This is the default.
*
*                Ex. With use of both a + or - spec and a delay.
*                   delay=("+HH:00", "HH:17"), the first spec "+HH:00" specifies
*                   that we shall only use data on a hole hour, the second spec, "HH:17",
*                   specifies that we shall delay the BUFR generation to 17 over each hour
*                   if we have no data for all types we expect data for.
*
*                NOTE It can be only one elements with +/- in front
*                in the list of delays. This elements do NOT specify delays
*                but when a BUFR message is possibly generated. No BUFR message
*                is generated if there is no data at that time.
*
*                 ex: delay=("6:10", "SS:03")
*                     This means that we shall delay with 3 minutes for all
*                     bufrtimes except for the 6 termin that we shall delay
*                     with 10 minutes.
*
*                     delay=("6:10", "SS:03", "HH:01")
*                     This means that we shall delay all termins with 1
*                     minute, except for bufrtimes (except the 6 termin
*                     that shall be delayed with 10 minutes) that shall delay
*                     with 3 minutes.
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
*   typepriority [list]: A list of typeid's that is used to create a
*                        BUFR message.
* </pre>
*/
class MsgTime {
public:
   typedef std::bitset<60> Minutes;
   typedef std::bitset<25> Hours;
private:
   Hours hours_; //The last bit, 24, signals min for each hour or not.
   Minutes *minAtHour_;
   void reallocateAndSet( int hour, int min );

   ///@return -1 if npo index is found
   int getIndexForHour( int h )const;
   int count()const;
   bool minForEachHour()const;
   void decodeHourSpec( const std::string &hh );
   void decodeMinSpec( const std::string &hh );
   void setAllHours();
   void setHour( int h, bool flag );

public:
   MsgTime();
   MsgTime( bool minutesForEachHour );
   MsgTime( const MsgTime &mt );
   ~MsgTime();


   Hours getHours()const { return hours_; }
   MsgTime& operator=( const MsgTime &rhs );
   bool operator==(const MsgTime &rhs)const;

   bool msgForTime( const boost::posix_time::ptime &t )const;
   Minutes minAtHour( int h )const;
   void setMsgForTime( int hour, int min, bool flag );

   /**
    * @throw std::logical_error,
    *        std::out_of_range
    *        std::bad_alloc,
    *        boost::bad_lexical_cast
    */
   void setMsgForTime( const std::string &timespec );

   void invertHours();

   friend std::ostream& operator<<( std::ostream &o, const MsgTime &mt );

};

std::ostream&
operator<<( std::ostream &o, const MsgTime &mt );


class DelayInfo
{


   //hour have several value:
   // [0,23] the termin (hour) to delay.
   //  -1 specify only bufrtimes ie. 0, 3, 6, 9, 12, 15, 18, 21
   //  -2 specify all hours

   char hour_;
   char delay_; //minutes
   bool force_;
   MsgTime *msgtimes_;  //Create bufr for this times.


public:
   enum {STIME=-1, HTIME=-2, FSTIME=-3, FHTIME=-4,
         SKIP=100,HSKIP=101,SSKIP=102, UNDEF=-127};

   DelayInfo(int hour=UNDEF)
   :hour_(hour), delay_(0), force_(false), msgtimes_(0){
      if(hour>=SKIP){
         //std::cerr << "DelayInfo::CTOR: SKIP!" << std::endl;
         msgtimes_ = new MsgTime();
      }
   }
   DelayInfo(char hour, char delay, bool force)
   : hour_(hour), delay_(delay), force_(force), msgtimes_(0){}
   DelayInfo(const DelayInfo &d)
   :hour_(d.hour_),delay_(d.delay_),force_(d.force_), msgtimes_(0){
      if(d.msgtimes_)
         msgtimes_ = new MsgTime( *d.msgtimes_ );
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
            if( msgtimes_ )
               delete msgtimes_;

            msgtimes_ = new MsgTime( *rhs.msgtimes_ );
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

      if( hour_==di.hour_   &&
          delay_==di.delay_ &&
          force_==di.force_ ){

         if(msgtimes_ || di.msgtimes_){
            if( msgtimes_ && di.msgtimes_ )
               return *msgtimes_ == *di.msgtimes_;
            else
               return false;
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

   void setMsgTime( const MsgTime &msgTime ) {
      if( msgtimes_ ) {
         *msgtimes_ = msgTime;
      } else {
         msgtimes_ = new MsgTime( msgTime );
      }
   }

   //Shall we generate bufr for this hour
   bool msgForThisTime( const boost::posix_time::ptime  &t )const{
      if( t.is_special() )
         return false;

      if(!msgtimes_)
         return true;

      return msgtimes_->msgForTime( t );
   }



   void msgForThisTime( const int hour, int min,  bool flag){
      //Must be one of the SKIP spec.
      if( ! msgtimes_ )
         msgtimes_ = new MsgTime();

      msgtimes_->setMsgForTime( hour, min, flag );
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
   * must have type, ie we cant create a SYNOP without it.
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

   typedef std::list<int>                    IntList;
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
   IntList        code_;
   int            height_;
   int            heightVisability_;
   int            heightTemperature_;
   float          heightPressure_;
   int            heightPrecip_;
   int            heightWind_;
   int            heightWindAboveSea_;
   float          latitude_;
   float          longitude_;
   int            wmono_;
   std::string    name_;
   int            stationID_;
   std::string    callsign_;
   TLongList      definedStationid_;
   TTypeList      typepriority_;
   TStringList    precipitation_;
   TDelayList     delayList_;
   std::string    list_;
   bool           copyIsSet_;
   bool           copy_;
   std::string    copyto_;
   std::string    owner_;
   boost::posix_time::ptime delayUntil_;
   static std::string  debugdir_;
   milog::LogLevel loglevel_;
   bool            cacheReloaded48_;
   bool            ignore;


   StationInfo();

public:
   StationInfo( int wmono );
   StationInfo(const StationInfo &);

   ~StationInfo();

   bool ignoreThisStation() const { return ignore; }
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
   //IntList        code()const { return code_; }
   int        code()const; 
   std::string    codeToString()const;


   int       height()const{ return height_; }
   int       heightAdd( int ammount )const;
   void      height( int h, bool ifUnset = true );
   int       heightVisability() const;
   void      heightVisability( int h, bool ifUnset = true);
   int       heightTemperature()const;
   void      heightTemperature( int h, bool ifUnset = true );
   float     heightPressure() const;
   void      heightPressure( float h, bool ifUnset=true);
   int       heightPrecip()const;
   void      heightPrecip( int h, bool ifUnset=true );
   int       heightWind()const;
   void      heightWind( int h, bool ifUnset=true );
   int       heightWindAboveSea()const;
   void      heightWindAboveSea( int h, bool ifUnset=true  );
   float     latitude()const { return latitude_; }
   void      latitude( float lat, bool ifUnset=true );
   float     longitude()const { return longitude_; }
   void      longitude( float lon, bool ifUnset=true );
   int       wmono()const{ return wmono_;}

   /**
    * return callsign as a number if possible.
    * return MAX_INT, if it is not a number.
    */
   int       callsignAsInt()const;
   std::string callsign()const { return callsign_;}
   std::string name()const{ return name_; }
   void      name( const std::string &n, bool ifUnset=true );

   TLongList definedStationID()const { return definedStationid_;}
   bool      hasDefinedStationId(long stid)const;

   /**
    * Is this station definition defined for stationid and typeid at hour.
    * If hour<0 ignore the hour, \see hasTypeId.
    */
   bool      hasDefinedStationIdAndTypeId(long stid, long tid, int hour=-1)const;

   /**
   * The stationid is set if the station definition comes from
   * a id_nnnnn section in the configuration file
   * @return INT_MIN if not set.
   */
   int       stationID()const;

   /**
    * creates an string that help us to identify the stations
    * configuration section in the configuration file.
    *
    * It returns 'wmo_nnnn' when the wmono is set,
    * 'id_nnnn' when the stationid is set and
    * 'id_callsign' if the callsign is set.
    *
    *
    * @return a identification string.
    */
   std::string toIdentString()const;
   static std::string debugdir() { return debugdir_;}

   TStringList precipitation()const { return precipitation_;}
   bool        hasPrecipitation()const{ return !precipitation_.empty();}

   /**
   * \brief A list of \em typeid to use data for when encoding SYNOP.
   * 
   * Typepriority plays two roles. Only data that is in this
   * list is used to create BUFR message. The second use is
   * to select which Data that shall be used if there is more 
   * than one Data for one parameter. The Data is selected in the
   * order in the list. Data with typeid preciding another typeid
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
   *             the hour \a hour. A negative value means all typeids.
   * \return true if \em typeID is in the list of types we shall
   *         use data from when encoding SYNOP. false othewise.
   */
   bool hasTypeId(int typeID, int hour=-1)const;

   /**
   *  \brief Setter function to set the delay.
   */
   void delayUntil(const boost::posix_time::ptime &delay){ delayUntil_=delay; }

   /**
   * \brief Getter function to get the delay.
   */
   boost::posix_time::ptime delayUntil()const { return delayUntil_;}


   /**
   * \brief Checks if we shall delay for this hour if not
   * all data is received for this station.
   *
   * It return true if we shall delay, and the number of
   * minutes to delay is returned in the output parameter 'minute'.
   * If we shall not delay it return false.
   *
   * \param[in] Check if we need to delay for this time.
   * \param minute[out] Number of minutes to delay.
   * \param force[out] True if we shall allways delay, even if all
   *        data that is required.
   * \param relativToFirst[out] Is set to true on return if the delay
   *                            shall be relativ to the first typeid received
   *                            data for for a given bufr time.
   * \return Return the minutes to delay, if 0 do not delay.
   */
   int delay( const boost::posix_time::ptime &t, bool &force, bool &relativToFirst)const;

   bool msgForTime( const boost::posix_time::ptime &t )const;

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

   bool operator<(const StationInfo &rhs )const;

   /**
    * The equal operator only test if two StationInfo instances
    * represent the same configuration based on the identifications
    * ie. stationID or wmono. It do not check that all configuration data
    * is the same, use equalTo to check for this.
    *
    * @param rhs The other StationInfo.
    * @return true if equal and false otherwise.
    */

   bool operator==(const StationInfo &rhs) const;
   /**
    * The not equal operator only test if two StationInfo instances
    * represent different StationInfo based on the identifications
    * ie. stationID or* wmono. It do not take the configuration
    * data into account. Use equalTo to check for this.
    *
    * @param rhs The other StationInfo.
    * @return true if not equal and false otherwise.
    */

   bool operator!=(const StationInfo &rhs) const {
      return !( *this == rhs );
   }

   /**
    * Returns a string on the form [stationid1,stationid2,..,stationidN/typeid1,typeid2,..,typeidN] 
    */
   std::string getStationsAndTypes()const;

   std::ostream& printDelayInfo( std::ostream& ost );
};

typedef boost::shared_ptr<StationInfo> StationInfoPtr;
typedef std::list<StationInfoPtr> StationInfoList;

class StationList :
   public std::list<StationInfoPtr>
{
public:
      StationList();
      ~StationList();

      /**
       * Find all stations that has defined stationid, typeid and hour in obstime.
       *  - If typeId is 0 ignore typeid and obstime.
       *  - If obstime is undefined ignore hour.
       */
      StationInfoList findStation( long stationid, long typeId=0, const boost::posix_time::ptime &obstime=boost::posix_time::ptime() )const;
      StationInfoList findStationByWmono( int wmono )const;
      StationInfoList findStationById( int stationid )const;
      StationInfoList findStationByCallsign( const std::string &callsign )const;
};

typedef StationList::iterator        IStationList;
typedef StationList::const_iterator CIStationList;


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
