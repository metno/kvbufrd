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
#include <atomic>
#include <memory>
#include "boost/filesystem.hpp"
#include <mutex>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvcpp/KvApp.h"
#include "kvdb/dbdrivermgr.h"
#include "milog/milog.h"
#include "kvdb/ConnectionPool.h"
#include "StationInfo.h"
#include "StationInfoParse.h"
#include "Waiting.h"
#include "tblBufr.h"
#include "kvevents.h"
#include "kvDbGateProxyThread.h"
#include "DbQuery.h"

struct Opt {
  std::string conffile;
  std::string progname;
  std::string pidfile;
  std::string obslogfile;
  milog::LogLevel loglevel;
  bool diasableDataReceiver;
  boost::posix_time::ptime fromTime;
};

std::string getProgNameFromArgv0(const std::string &cmdname);
void decodeArgs(int argn, char **argv, Opt &opt);
void usage(const std::string &progname, int exitCode);

//extern Opt options;

namespace kvbufrd {
class StationInfoList;
}

class GetData;

class App : public kvalobs::sql::DbQuery {

 public:
  typedef std::mutex Mutex;
  typedef std::lock_guard<Mutex> Lock;
  static volatile std::atomic_bool AppShutdown;
 private:

  dnmi::db::ConnectionPool kvDbPool;
  std::string dbDriver;
  std::string dbConnect;
  std::string dbDriverId;
  std::string kvDbDriver;
  std::string kvDbConnect;
  std::string kvDbDriverId;
  std::string kafkaDomain;
  std::string kafkaBrokers;
  std::string kafkaGroupId;
  StationList stationList;
  boost::posix_time::ptime startTime_;
  WaitingList waitingList;
  std::string confFile;
  std::string obslogFile;
  std::string mypathInCorbaNS;
  std::list<int> continuesTypeID_;
  std::list<ObsEvent*> obsEventWaitingOnCacheReload;
  std::list<GetData*> getDataThreads;
  bool hasStationWaitingOnCacheReload;
  bool acceptAllTimes_;
  milog::LogLevel defaultLogLevel;


  mutable Mutex mutex;
  mutable Mutex mutexGetDataThreads; //Mutex to protect getDataThreads

  void readDatabaseConf(miutil::conf::ConfSection *conf);
  void setObslogfile(Opt &opt, miutil::conf::ConfSection *conf);

 public:

  static App *kvApp;
  Opt options;
  milog::LogLevel priorityQueLogLevel;

  std::shared_ptr<kvalobs::KvDbGateProxyThread> dbThread;

  App(int argn, char **argv, Opt &opt /* const std::string &confFile_*/, miutil::conf::ConfSection *conf);
  ~App();

  void readWaitingElementsFromDb();

  bool acceptAllTimes() const {
    return acceptAllTimes_;
  }

  std::string getCacheDbFile()const;

  /**
   * \brief cretae a globale logger with id \a id.
   *
   * \param is An id to identify the logger.
   * \return true on success and false otherwise.
   */
  bool createGlobalLogger(const std::string &id, milog::LogLevel ll = milog::NOTSET);

  /**
   * \brief Read the StationInfo from the configuration file
   * represented by \c conf.
   *
   * \param conf A parsed configuration file elements.
   * \return true if the StationInfo data was succsessfuly read from
   *         conf and false otherwise.
   */
  bool readStationInfo(miconf::ConfSection *conf);

  /**
   * \brief Read the StationInfo from the configuration file.
   * 
   * The \c stationList is \b NOT updated.
   *
   * \param[out] stList The StationInfo's is returned is this list.
   * \return true on success false otherwise.
   */
  bool readStationInfo(std::list<StationInfoPtr> &stList) const;

  StationList getStationList() const;


  /**
   * \brief Creates a new connection to the database.
   *
   * The caller must call releaseDbConnection after use.
   *
   * \return A database connection.
   */
  dnmi::db::Connection *createKvDbConnection();

  /**
   * \brief release a connection to the database.
   *
   * The connction to be released must be obtained by a call to
   * getNewDbConnection().
   *
   * \param con The connection to release.
   */
  void releaseKvDbConnection(dnmi::db::Connection *con);


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
  void releaseDbConnection(dnmi::db::Connection *con);

  /**
   * \brief Find all station information based on the stationid.
   *
   * \param stationid The stationid to find the station information too.
   * \return A StationInfoLis.
   */
  StationInfoList findStationInfo(long stationid, long typeId=0, const boost::posix_time::ptime &obtime=boost::posix_time::ptime());

  /**
   * \brief Find all station information that has WMO number.
   *
   * \param wmono The WMO number to find the station information too.
   * \return A StationInfoList.
   */

  StationInfoList findStationInfoWmono(int wmono);

  StationInfoList findStationInfo(int wmono, long id, const std::string &callsign, int code )const;

  /**
   * \brief The time the application was started.
   * 
   * \return The start time.
   */
  boost::posix_time::ptime startTime() const {
    return startTime_;
  }

  boost::posix_time::ptime checkpoint();
  void createCheckpoint(const boost::posix_time::ptime &checkpoint);

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
   * in the database. This table is searched  at startup of \e kvbufrd.
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
  WaitingPtr addWaiting(WaitingPtr w, bool replace);

  WaitingPtr getWaiting(const boost::posix_time::ptime &obstime, StationInfoPtr info);

  /**
   * \brief getExpired return a list of \c Waiting elements that has expired.
   *
   * The elements is removed from the \c waitingList, but not from the 
   * database. Use \b removeWaiting to remove the element from the 
   * database.
   *
   * \sa App::removeWaiting
   */
  WaitingList getExpired();

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
  void replaceStationConf(const StationList &newConf);

  /**
   * \brief removeWaiting, remove the waiting element \a w from the database.
   *
   * \param w The Waiting element to remove from the \c waitingList.
   * \param con A open connection to the database.
   */
  void removeWaiting(WaitingPtr w);

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
  void removeWaiting(StationInfoPtr info, const boost::posix_time::ptime &obstime);

  /**
   * saveBufrData, save data for a generated bufr to the database.
   *
   * \param tblBufr The data to be saved.
   * \return true if the data was saved and false on error.
   */
  bool saveBufrData(const TblBufr &tblBufr);

  /**
   * getSavedBufrData. Search the database for a bufr message for
   * this station, for the given obstime if it exist.
   *
   * \param info The station to search for.
   * \param obstime The obstime to search for.
   * \param tblBufr This paramter holds the synop data.
   * \return true if we there was now database error, and false otherwise.
   */
  bool getSavedBufrData(StationInfoPtr info, const boost::posix_time::ptime &obstime, std::list<TblBufr> &tblBufr);

  /**
   * \brief Start a background thread to get data from kvalobs.
   *
   *  Getdata from kvalobs for the \a wmono from time \a t.
   *  If \a wmono is <0 get data for all defined wmo stations.
   *
   *  If \a hours is <0 it is only generated one synop event, and this
   *  event is for the time \a t.
   *
   * \param t reload the cache with data from this time.
   * \param wmono The WMO number we want to reload the cache for.
   *              wmono<0, reload the cache for all defined stations.
   * \param hours If hours<0 get \a hours with data back in time from
   *              \a t. If hours>=0 get data from \a t until current time.
   * \param que   post ObsEvent on this que.
   */
  bool getDataFrom(const boost::posix_time::ptime &t, int wmono, int hours, std::shared_ptr<threadutil::CommandQueueBase> que);

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
  bool joinGetDataThreads(bool waitToAllIsJoined, const std::string &logid = "");

  /**
   * \brief Mark the station as relaoded with kvalobs data.
   */
  void cacheReloaded(StationInfoPtr info);
  void cacheReload(StationInfoPtr station);

  /**
   * \brief Mark the a station to be relaoded with kvalobs data.
   *
   * \param wmono The WMO number to the station to be reloaded.
   *              If wmono = -1 mark all station for reload.
   * \param id stationid.
   * \return A list of all stations that is marked for reload.
   */
  StationList reloadCache(int wmono, int id = 0);


  /**
   *\brief Add an obsevent to the list of events waiting on cache reload
   *       if the station is not reloaded.
   *
   * If the station is reloaded send the event to the event que \a que.
   *
   * \param event the event to be added.
   * \param que The event que to send the event if the station is reloaded.
   */
  void addObsEvent(ObsEvent *event, threadutil::CommandQueueBase &que);

  /**
   * \brief Check the list of event waiting on cache reload against the 
   * stationInfo list to se if the cache reload to se if the station is
   * reloaded. If so post the event on the event que \a que.
   *
   * \param que The event que to post the event on.
   *
   * \return true if there still is station waiting on reloading and false otherwise.
   */
  bool checkObsEventWaitingOnCacheReload(threadutil::CommandQueue &que, const std::string &logid = "");

  bool shutdown();

  void run( std::shared_ptr<threadutil::CommandQueue> newDataQue);

};

#endif

