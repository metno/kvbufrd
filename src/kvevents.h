/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvevents.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvalobs_kvevents_h__
#define __kvalobs_kvevents_h__

#include <sstream>
#include "CommandQueue.h"
#include "CommandPriorityQueue.h"
#include "boost/date_time/posix_time/ptime.hpp"
#include <kvalobs/miutil/timeconvert.h>
#include <kvalobs/milog/milog.h>
#include "StationInfo.h"
#include "Waiting.h"
#include "KvObsData.h"
#include <stdexcept>

//namespace kvalobs {

class KvEventBase : virtual public threadutil::CommandBase {
 public:
  KvEventBase() {
  };

  virtual ~KvEventBase() {
  };

  /**
   * from CommandBase. 
   * Do nothing!
   */
  bool executeImpl() override {
    return true;
  };
};




class DataEvent : public KvEventBase {
  kvalobs::KvObsDataMapPtr data_;
  std::string    inCommingMessage_;
  std::string    producer_;

 public:
   DataEvent(kvalobs::KvObsDataMapPtr data)
   : data_(data) {
   }

   void inCommingMessage( const std::string &m ) {
      inCommingMessage_=m;
   }

   std::string inCommingMessage()const {
      return inCommingMessage_;
   }

   std::string producer() const {
      return producer_;
   }

   void producer(const std::string &prod ) {
      producer_=prod;
   }

   kvalobs::KvObsDataMapPtr data() const {
      return data_;
   }
};



class ObsEvent : 
   virtual public threadutil::Priority2CommandBase, 
   virtual public threadutil::PriorityCommandBase
{
   static boost::posix_time::ptime now_;
   boost::posix_time::ptime delayInQue_;
   bool                     isDelayedInQue_;
   boost::posix_time::ptime obstime_;
   StationInfoPtr        stInfo;
   WaitingPtr            waiting_;
   boost::posix_time::ptime addedToQueue_;
   boost::posix_time::time_duration timeInQueue_;

   ///Used to send a message back in the callback \a ref
   std::ostringstream    msg_;

   ///Used to send a bufr back in the callback \a ref
   std::string           bufr_;

   ///Used to send status indicator back in the callback \a ref
   bool                  isOk_;

   ///Used to indicate that a bufr posibly need to be regenerated
   ///due to changed data.
   bool                  regenerate_;



   struct IdType{
      int sid_;
      int tid_;

      IdType(int sid, int tid):sid_(sid), tid_(tid){}
      IdType(const IdType &it):sid_(it.sid_), tid_(it.tid_){}

      IdType& operator=(const IdType &rhs){
         if(this!=&rhs){
            sid_=rhs.sid_;
            tid_=rhs.tid_;
         }
         return *this;
      }
   };


   std::list<IdType> typeidReceived;

public:

   /**
    * \brief Constructor to be used to signal a regenerate of a SYNOP.
    */
   ObsEvent( const boost::posix_time::ptime &obstime,
             StationInfoPtr stInfo_,
             bool regenerate=false )
   :delayInQue_(boost::posix_time::microsec_clock::universal_time()),
    isDelayedInQue_(false),
    obstime_(obstime),
    stInfo(stInfo_),
    isOk_(false),
    regenerate_(regenerate)
   {}


   ObsEvent(WaitingPtr w)
      : delayInQue_(boost::posix_time::microsec_clock::universal_time()),
        isDelayedInQue_(false),
        obstime_(w->obstime()),
        stInfo(w->info()),
        waiting_(w),
        isOk_(false),
        regenerate_(false)
   {
      auto it=w->note().find("regen");
      regenerate_=it!=std::string::npos;
   }

   virtual ~ObsEvent(){}

   //setNow is only for testing.
   void setNow( const boost::posix_time::ptime &n) {
      now_=n;
   }

   boost::posix_time::ptime getNow(bool use_microsecond_clock=true)const {
      namespace pt = boost::posix_time;
      if(now_.is_special() ) {
         if(use_microsecond_clock) {
            return pt::microsec_clock::universal_time();
         }
         return pt::second_clock::universal_time();
      }
      return now_;
   }
  /**
   * from CommandBase. 
   * Do nothing!
   */
   bool executeImpl() override {
      return true;
   };

   void onPost() override {
      addedToQueue_=getNow();
   }

   void onGet() override {
      auto now=getNow();
      timeInQueue_=now - addedToQueue_;
   }

   boost::posix_time::time_duration timeInQue()const {
      return timeInQueue_;
   }

   //The queue is sorted with the latest obstime first
   virtual Action add(const Priority2CommandBase &e_)override{
      auto e=dynamic_cast<const ObsEvent*>(&e_);
      if(!e) {
         std::cerr << "ObsEvent::add: failed assert. Cant cast 'Priority2CommandBase' to 'ObsEvent'\n";
         LOGWARN("ObsEvent::add: Failed assert. Cant cast 'Priority2CommandBase' to 'ObsEvent'");
         abort();
      }

      if( e->obstime_ > obstime_) {
         return CONTINUE;
      } else {
         return INSERT;
      }
   }; 

   virtual bool get(){
      boost::posix_time::ptime now=getNow();
      if(delayInQue_>now) {
         return false;
      } else {
         return true;
      }
   }; 

   virtual bool lessThan(const PriorityCommandBase *rhs)const {
      auto p = dynamic_cast<const ObsEvent*>(rhs);
      if( !p ) {
         throw std::runtime_error("--ObsEvent: lessThan: argument is not an ObsEvent--");
      }
      return obstime_< p->obstime_;
   }
   
   boost::posix_time::ptime obstime()const{ return obstime_;}
   StationInfoPtr     stationInfo()const{ return stInfo;}
   WaitingPtr             waiting()const{ return waiting_;}
   
   void                  delayInQue(int milliseconds) {
      namespace pt=boost::posix_time;
      isDelayedInQue_=milliseconds>0;
      
      delayInQue_=pt::microsec_clock::universal_time()+pt::milliseconds(milliseconds);
   }

   bool regenerate()const { return regenerate_;}
   std::string note()const {
      if(waiting_)
         return waiting_->note();
      return "";
   }

   ///The following tree functions is used to comunicate
   ///data back to a client trough the callback \a ref.
   ///The callback is only set when we get a explicit request
   ///to generate a bufr.

   std::ostringstream& msg() { return msg_;}
   void              bufr(const std::string &s){ bufr_=s;}
   std::string       bufr()const { return bufr_;}
   void              isOk(bool f){ isOk_=f;}
   bool              isOk()const { return isOk_;}

   void              addTypeidReceived(long stationid, int typeid_){
      for(std::list<IdType>::iterator it=typeidReceived.begin();
            it!=typeidReceived.end();
            it++){
         if(it->sid_==stationid && it->tid_==typeid_)
            return;
      }

      typeidReceived.push_back(IdType(stationid,typeid_));
   }

   void clearTypeidReceived(){
      typeidReceived.clear();
   }

   int nTypeidReceived()const{
      return typeidReceived.size();
   }

   bool hasReceivedTypeid(int sid, int tid, bool doLogTypeidInfo)const{
      std::ostringstream ost;

      if(typeidReceived.empty()){

         if(doLogTypeidInfo){
            LOGWARN("ObsEvent: TypeidReceived empty. Accept all data!");
         }

         return true;
      }

      if(doLogTypeidInfo){
         ost << "ObsEvent: hasTypeidReceived (" << sid << "/" << tid << "):";
         for(std::list<IdType>::const_iterator it=typeidReceived.begin();
               it!=typeidReceived.end();
               it++)
            ost << " (" << it->sid_ << "/" << it->tid_ << ")";
      }

      for(std::list<IdType>::const_iterator it=typeidReceived.begin();
            it!=typeidReceived.end();
            it++){
         if(it->sid_==sid && it->tid_==tid){
            if(doLogTypeidInfo){
               ost << " Return: TRUE!";
               LOGDEBUG3(ost.str());
            }

            return true;
         }
      }

      if(doLogTypeidInfo){
         ost << " Return FALSE!";
         LOGDEBUG3(ost.str());
      }

      return false;
   }

   bool waitingOnContinuesData()const{
      if(waiting_ && waiting_->waitingOnContinuesData())
         return true;

      return false;
   }
   void debugInfo(std::ostream& info) const  override{   
      namespace pt=boost::posix_time;
      auto now = getNow().time_of_day();
      auto d = delayInQue_ - addedToQueue_;
      std::string delayed;
      if( isDelayedInQue_) {
         delayed="(D) ";
      }
      info << delayed << "ObsEvent: " << stInfo->toIdentString() << "/" << pt::to_kvalobs_string(obstime_) 
         << " addedToQue: " << addedToQueue_.time_of_day()
         << " delayInQue: " << delayInQue_.time_of_day() << " now: " << now << " d: " << d.total_milliseconds() <<"ms"
         << " regen: " << (regenerate_?"T":"F") << " waiting: " << (!waiting_?"F":"T") << "\n"; 
   } 
};




//}

#endif
