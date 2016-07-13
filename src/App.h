/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: App.h,v 1.13.2.9 2007/09/27 09:02:22 paule Exp $                                                       

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
/*
 * $Header: /var/lib/cvs/kvalobs/src/kvsynopd/App.h,v 1.13.2.9 2007/09/27 09:02:22 paule Exp $
 */

#ifndef __kvbufrd_app_h__
#define __kvbufrd_app_h__

#include <list>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <kvcpp/corba/CorbaKvApp.h>
#include <kvdb/dbdrivermgr.h>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include "kvbufrd.hh"
#include "kvbufrdImpl.h"
#include "StationInfo.h"
#include "StationInfoParse.h"
#include "Waiting.h"
#include "tblBufr.h"
#include "obsevent.h"
#include "kvDbGateProxyThread.h"


struct Opt {
   std::string conffile;
   std::string progname;
   std::string pidfile;
   miutil::miTime fromTime;
};

std::string getProgNameFromArgv0( const std::string &cmdname );
void decodeArgs( int argn, char **argv, Opt &opt );
void usage( const std::string &progname, int exitCode );

extern Opt options;


namespace kvbufrd{
  class StationInfoList;
}

class GetData;

class App : public kvservice::corba::CorbaKvApp
{
  
private:
   dnmi::db::DriverManager dbMgr;
   std::string             dbDriver;
   std::string             dbConnect;
   std::string             dbDriverId;
   StationList             stationList;
   miutil::miTime          startTime_;
   WaitingList             waitingList;
   std::string             confFile;
   std::string             mypathInCorbaNS;
   std::list<int>          continuesTypeID_;
   std::list<ObsEvent*>    obsEventWaitingOnCacheReload;
   std::list<GetData*>     getDataThreads;
   bool                    hasStationWaitingOnCacheReload;
   bool                    acceptAllTimes_;
   kvbufrd::bufr_var       bufrRef;
   milog::LogLevel         defaultLogLevel;

   mutable boost::mutex mutex;
   mutable boost::mutex mutexDbDriverManager;

public:
   boost::shared_ptr<kvalobs::KvDbGateProxyThread> dbThread;
   //dnmi::thread::CommandQue dbQue;

   App( int argn, char **argv, 
        const std::string &confFile_, miutil::conf::ConfSection *conf);
   ~App();

   void readWaitingElementsFromDb();

   bool acceptAllTimes()const { return acceptAllTimes_;}

   /**
    * \brief Setup and initialize the interface to kvbufrd.
    * 
    * \return true if the subsystem has been setup and false otherwise.
    */ 
   bool initKvBufrInterface( dnmi::thread::CommandQue &newObsQue );
  
   /**
    * \brief cretae a globale logger with id \a id.
    *
    * \param is An id to identify the logger.
    * \return true on success and false otherwise.
    */
   bool createGlobalLogger(const std::string &id, milog::LogLevel ll=milog::NOTSET );


  
  /**
   * \brief Read the StationInfo from the configuration file
   * represented by \c conf.
   *
   * \param conf A parsed configuration file elements.
   * \return true if the StationInfo data was succsessfuly read from
   *         conf and false otherwise.
   */
  bool readStationInfo( miconf::ConfSection *conf);

  /**
   * \brief Read the StationInfo from the configuration file.
   * 
   * The \c stationList is \b NOT updated.
   *
   * \param[out] stList The StationInfo's is returned is this list.
   * \return true on success false otherwise.
   */
  bool readStationInfo(std::list<StationInfoPtr> &stList)const;

  StationList getStationList()const;


  /**
   * \brief Creates a new connection to the database. 
   *
   * The caller must call releaseDbConnection after use.
   *
   * \return A database connection.
   */
  dnmi::db::Connection *createDbConnection();

  /**
   * \brief release a connection to the database.
   *
   * The connction to be released must be obtained by a call to 
   * getNewDbConnection().
   *
   * \param con The connection to release.
   */
  void                 releaseDbConnection(dnmi::db::Connection *con);

  /**
   * \brief The path in the CORBA nameserver we shall register our
   *        services.
   *
   *  The path is given with corba.kvpath in the configuration file.
   *  If it is not set, the default is the same as corba.path.
   */
  std::string  mypathInCorbaNameserver()const{ return mypathInCorbaNS; }

  /**
   * \brief Find all station information based on the stationid.
   *
   * \param stationid The stationid to find the station information too.
   * \return A StationInfoLis.
   */
  StationInfoList findStationInfo(long stationid);


  /**
   * \brief Find all station information that has WMO number.
   *
   * \param wmono The WMO number to find the station information too.
   * \return A StationInfoList.
   */

  StationInfoList findStationInfoWmono(int wmono);
  
  bool           listStations(kvbufrd::StationInfoList &list);

  
  /**
   * \brief The time the application was started.
   * 
   * \return The start time.
   */
  miutil::miTime startTime()const { return startTime_;}
  miutil::miTime checkpoint();
  void           createCheckpoint(const miutil::miTime &checkpoint);



  
  /**
   * \brief Set the list of typeids where the data is continues with 
   * respect on obstime.
   */
  
  void continuesTypeID(const std::list<int> &continuesTimes);


  
  
  /**
   * \brief Typeids where the data is expected to hava a continues timeserie.
   * \return a list of typeids.
   */

  std::list<int> continuesTypeID();

  
  /**
   * \brief Check if \a typeID is a continues typeid.
   *
   * \return true if typeID is a continues typeid and false otherwise.
   */
  
  bool isContinuesType(int typeID);

  /**
   * \brief Is the station set up with only typeid's that may not be continues 
   *   with respect on obstime.
   */
  bool onlyNoContinuesTypeID(StationInfoPtr st);


  /**
   *\brief  addWaiting, put the \c Waiting record,\a w, in the list of delayed
   * stations. 
   * 
   * If it allready is a record for \a w, ie. for \a w wmono and 
   * obstime, the record is replaced with this \a w, if and only if 
   * \a replace is true. The record is also  added to the table \c waiting 
   * in the database. This table is searched  at startup of \e kvsynopd.
   *
   * The waiting list is sorted on delay. ie. the longest delay in the
   * future is on the back and the delay nearest in time is on the  front.
   * In this way we can deceide if a delay has expired with a look
   * at the front, only.
   *
   * \param w The record to add to the \c waitingList.
   * \param replace Replace an already registred waiting record with this.
   * \return A WatingPtr if there allready is an Waiting element for
   *         this obstime and wmono. 0 otherwise.
   */
  WaitingPtr addWaiting(WaitingPtr w, bool replace );


  WaitingPtr getWaiting( const miutil::miTime &obstime,
                         StationInfoPtr info );

  /**
   * \brief getExpired return a list of \c Waiting elements that has expired.
   *
   * The elements is removed from the \c waitingList, but not from the 
   * database. Use \b removeWaiting to remove the element from the 
   * database.
   *
   * \sa App::removeWaiting
   */
  WaitingList    getExpired();

  /**
   * \brief Replace the StationInfo for the \c wmono represented
   *        by \a newInfoPtr.
   *
   * It is an error if the station list han no StationInfo for the wmonumber 
   * represented by \a newInfoPtr->wmono().
   *
   * \param newInfoPtr The new station information for the station 
   *                   represented by newInfoPtr->wmono()
   * \return The old StationInfo for the station on success and 0 otherwise.
   *         
   */
  StationInfoPtr replaceStationInfo(StationInfoPtr newInfoPtr);


  /**
   * \brief Add a new StationInfo for to the list of stationinformation.
   *
   * It is an error if the \a newInfoPtr exist.
   *
   * \param newInfoPtr The new station information to be added to the list of 
   *                   station information.
   * \return true on success and false otherwice.
   */
  bool addStationInfo(StationInfoPtr newInfoPtr);
  

  /**
   * Replace the current configuration with the new one.
   * @param newConf The new configuration to replace the old.
   */
  void replaceStationConf(const StationList &newConf );


  /**
   * \brief getDelayList, fill a \a DelayList with data from 
   * the \c waitingList.
   *
   * This method is used by the CORBA client interface, \c kvsynopdImpl.
   *
   * \param nowTime The time when the \c waitingList was sampled.
   * \return A pointer to a \a DelayList on sucess, 0 on fail.
   */
  kvbufrd::DelayList* getDelayList(miutil::miTime &nowTime);

  /**
   * \brief removeWaiting, remove the waiting element \a w from the database.
   *
   * \param w The Waiting element to remove from the \c waitingList.
   * \param con A open connection to the database.
   */
  void           removeWaiting(WaitingPtr w );

  /**
   * \brief removeWaiting, remove a waiting element from the database
   * and the waittingList, if it exist.
   *
   * The waiting element is identified with \a wmono and \a obstime.
   *
   * \param wmono  The wmo number to the \a waiting element we want
   *               to remove.
   * \param id the stationid.
   * \param obstime The obstime to the \a waiting element we want
   *               to remove.
   * \param con A open connection to the database.
   */
  void           removeWaiting( StationInfoPtr info,
                                const miutil::miTime &obstime );


  /**
   * saveBufrData, save data for a generated bufr to the database.
   *
   * \param tblBufr The data to be saved.
   * \return true if the data was saved and false on error.
   */
  bool saveBufrData(const TblBufr &tblBufr );


  /**
   * getSavedBufrData. Search the database for a bufr message for
   * this station, for the given obstime if it exist.
   *
   * \param info The station to search for.
   * \param obstime The obstime to search for.
   * \param tblBufr This paramter holds the synop data.
   * \return true if we there was now database error, and false otherwise.
   */
  bool getSavedBufrData( StationInfoPtr info,
                         const miutil::miTime &obstime,
                         std::list<TblBufr> &tblBufr );
  

  /**
   * \brief Start a background thread to get data from kvalobs.
   *
   *  Getdata from kvalobs for the \a wmono from time \a t.
   *  If \a wmono is <0 get data for all defined wmo stations.
   *
   * \param t reload the cache with data from this time.
   * \param wmono The WMO number we want to reload the cache for.
   *              wmono<0, reload the cache for all defined stations.
   * \param que   post Synop event on this que.
   */
  bool           getDataFrom( const miutil::miTime &t,
                              int                  wmono,
			                     dnmi::thread::CommandQue &que);


  /**
   * \brief Join all joinable GetData threads.
   *
   * If \a waitToAllIsJoined is true the function does not
   * return until all threads has exited.
   *
   * \note
   * It is the delay controll thread that cleans up GetDataThreads.
   *
   * \param Set waitToAllIsJoined to true if we shall wait to all threads 
   * has exited. Set it to false to return if no threads is joinable.
   * \param logid log to this logger.
   * \return true if one or more threads was joined, and false if no threads 
   *         are joined.
   */
  bool joinGetDataThreads(bool waitToAllIsJoined, const std::string &logid="");

  /**
   * \brief Mark the station as relaoded with kvalobs data.
   */
  void cacheReloaded( StationInfoPtr info );


  /**
   * \brief Mark the a station to be relaoded with kvalobs data.
   *
   * \param wmono The WMO number to the station to be reloaded.
   *              If wmono = -1 mark all station for reload.
   * \param id stationid.
   * \return A list of all stations that is marked for reload.
   */
  StationList reloadCache(int wmono, int id=0 );


  /**
   * \brief return a list of current stations marked for reload.
   * 
   * \return a list of station waiting on cache reload and number
   *         of \a ObsEvent waiting.
   */
  kvbufrd::ReloadList* listCacheReload();


  /**
   *\brief Add an obsevent to the list of events waiting on cache reload
   *       if the station is not reloaded.
   *
   * If the station is reloaded send the event to the event que \a que.
   *
   * \param event the event to be added.
   * \param que The event que to send the event if the station is reloaded.
   */
  void addObsEvent(ObsEvent *event, dnmi::thread::CommandQue &que);

  /**
   * \brief Check the list of event waiting on cache reload against the 
   * stationInfo list to se if the cache reload to se if the station is
   * reloaded. If so post the event on the event que \a que.
   *
   * \param que The event que to post the event on.
   */
  void checkObsEventWaitingOnCacheReload(dnmi::thread::CommandQue &que,
					 const std::string &logid="");



};

#endif


