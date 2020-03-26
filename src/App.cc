/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: App.cc,v 1.19.2.14 2007/09/27 09:02:22 paule Exp $

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
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include <ctype.h>
#include <list>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include "boost/version.hpp"
#include "boost/filesystem.hpp"
#include "milog/milog.h"
#include "fileutil/pidfileutil.h"
#include "kvalobs/kvPath.h"
#include "miutil/timeconvert.h"
#include "kvDbGateProxy.h"
#include "tblWaiting.h"
#include "Data.h"
#include "App.h"
#include "StationInfoParse.h"
#include "tblKeyVal.h"
#include "getDataReceiver.h"
#include "GetDataThread.h"
#include "parseMilogLogLevel.h"
#include "KvDataConsumer.h"
#include "parseMilogLogLevel.h"
#include "cachedb.h"

using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace milog;
using namespace miutil::conf;
using namespace kvservice;
using dnmi::db::ConnectionPool;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
namespace b = boost;

namespace {

void createDirectory(const fs::path &path) {
  try {
    if (!fs::exists(path)) {
      fs::create_directories(path);
    } else if (!fs::is_directory(path)) {
      LOGFATAL("The path <" << path << "> exist, but is NOT a directory.");
      cerr << "The path <" << path << "> exist, but is NOT a directory.";
      exit(1);
    }
  } catch (const fs::filesystem_error &ex) {
    LOGFATAL("The path <" << path << "> does not exist and cant be created." << "Reason: " << ex.what());
    cerr << "The path <" << path << "> does not exist and cant be created." << "Reason: " << ex.what() << endl;
    exit(1);
  }

}

class MyConnectionFactory : public kvalobs::ConnectionFactory {

  App *app;
  int count_;
  std::mutex m_;

 public:
  MyConnectionFactory(App *app_)
      : app(app_), count_(0) {
  }

  virtual dnmi::db::Connection* newConnection() {
    std::lock_guard<std::mutex> l(m_);
    count_++;
    return app->createDbConnection();
  }

  virtual void releaseConnection(dnmi::db::Connection *con) {
    std::lock_guard<std::mutex>  l(m_);
    count_--;
    app->releaseDbConnection(con);
  }

  virtual int openCount()override {
    std::lock_guard<std::mutex>  l(m_);
    return count_;
  }

};

string getValue(const miconf::ConfSection *conf, const std::string &key) {
  string val = conf->getValue(key).valAsString("");
  if (val.empty())
    throw std::runtime_error("No '" + key + "' configured.");
  return val;
}

std::string getKafkaDomain(const miconf::ConfSection *conf) {
  return getValue(conf, "kafka.domain");
}

std::string getKafkaBrokers(const miconf::ConfSection *conf) {
  return getValue(conf, "kafka.brokers");
}

void sig_term(int signal) {
  App::AppShutdown = true;
}

void setSigHandlers() {
  sigset_t oldmask;
  struct sigaction act, oldact;

  act.sa_handler = sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGTERM, &act, &oldact) < 0) {
    LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
    exit(1);
  }

  act.sa_handler = sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGINT, &act, &oldact) < 0) {
    LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
    exit(1);
  }
}

}  // namespace

App* App::kvApp = nullptr;

volatile std::atomic_bool App::AppShutdown(false);

bool App::createGlobalLogger(const std::string &id, milog::LogLevel ll) {
  try {

    if (ll == milog::NOTSET)
      ll = defaultLogLevel;

    if( LogManager::hasLogger(id) )
       return true;

    FLogStream *logs = new FLogStream(4, 524288);  //512k
    std::ostringstream ost;
    fs::path logpath = fs::path(kvPath("logdir")) / options.progname / (id + ".log");

    if (logs->open(logpath.string())) {
      logs->loglevel(ll);
      if (!LogManager::createLogger(id, logs)) {
        delete logs;
        return false;
      }

      return true;
    } else {
      LOGERROR("Cant open the logfile <" << logpath << ">!");
      delete logs;
      return false;
    }
  } catch (...) {
    LOGERROR("Cant create a logstream for LOGID " << id);
    return false;
  }
}

void App::readDatabaseConf(miutil::conf::ConfSection *conf){
  dbDriver = conf->getValue("database.cache.driver").valAsString("");
  dbConnect = conf->getValue("database.cache.connect").valAsString("");

  if (dbDriver.empty() || dbConnect.empty()) {
    LOGFATAL("No <database.cache.driver> or <database.cache.connect> in the configurationfile!");
    exit(1);
  }

  auto cachedbdir = fs::absolute(fs::path(dbConnect).parent_path());
  if ( setenv("SQLITE_TMPDIR", fs::absolute(cachedbdir).c_str(), 0) == -1 ) {
     LOGFATAL("Failed to set SQLITE_TMPDIR '" << cachedbdir);
     exit(1);
  } else {
    char *e = getenv("SQLITE_TMPDIR");

    if ( !e ) {
      LOGFATAL("No enviroment variable SQLITE_TMPDIR");
      exit(1);
    }
    LOGINFO("SQLITE_TMPDIR: '" << e << "'.");
    IDLOGINFO("main","SQLITE_TMPDIR: '" << e << "'.");
    cerr << "SQLITE_TMPDIR: '" << e << "'." << endl << endl;
  }
  
  kvDbDriver = conf->getValue("database.kvalobs.driver").valAsString("");
  kvDbConnect = conf->getValue("database.kvalobs.connect").valAsString("");

  if (kvDbDriver.empty() || kvDbConnect.empty()) {
    LOGFATAL("No <database.kvalobs.driver> or <database.kvalobs.connect> in the configurationfile!");
    exit(1);
  }

  LOGINFO("Loading driver for database engine <" << dbDriver << ">!\n");

  if (!dnmi::db::DriverManager::loadDriver(dbDriver, dbDriverId)) {
    LOGFATAL("Can't load driver <" << dbDriver << endl << dnmi::db::DriverManager::getErr() << endl << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }

  LOGINFO("Loading driver for database engine <" << kvDbDriver << ">!\n");

  if (!dnmi::db::DriverManager::loadDriver(kvDbDriver, kvDbDriverId)) {
    LOGFATAL("Can't load driver <" << dbDriver << endl << dnmi::db::DriverManager::getErr() << endl << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }
}

App::App(int argn, char **argv, const std::string &confFile_, miutil::conf::ConfSection *conf)
    : kvDbPool([this]() {return createKvDbConnection();}, [this](dnmi::db::Connection *con) {releaseKvDbConnection(con);} ),
      DbQuery([this]() {return kvDbPool.get();}),
      startTime_(pt::second_clock::universal_time()),
      confFile(confFile_),
      hasStationWaitingOnCacheReload(false),
      acceptAllTimes_(false),
      defaultLogLevel(milog::INFO) {
  ValElementList valElem;
  string val;
  string bufr_tables(DATADIR);
  string logdir = kvPath("logdir");
  bool bufr_tables_names( false);

  kvApp = this;
  LogContext context("ApplicationInit");
  kafkaDomain = getKafkaDomain(conf);
  kafkaBrokers = getKafkaBrokers(conf);

  createDirectory(fs::path(kvPath("logdir")) / options.progname);

  valElem = conf->getValue("loglevel");

  if (!valElem.empty()) {
    std::string slevel = valElem[0].valAsString();
    milog::LogLevel ll = parseMilogLogLevel(slevel);

    if (ll != milog::NOTSET)
      defaultLogLevel = ll;
  }

  milog::LogManager *manager = milog::LogManager::instance();

  if (manager) {
    manager->loglevel(defaultLogLevel);
    milog::Logger &logger = milog::Logger::logger();
    logger.logLevel(defaultLogLevel);
  }

  createGlobalLogger("GetData");
  createGlobalLogger("DelayCtl");
  createGlobalLogger("main");
  createGlobalLogger("uinfo0");
  createGlobalLogger("kafka");
  createGlobalLogger("DataReceiver");
  milog::createGlobalLogger(logdir, options.progname, "kafka_received", milog::DEBUG, 10*1024, 10, new milog::StdLayout1());
  milog::createGlobalLogger(logdir, options.progname, "datareceiver_saved", milog::DEBUG, 10*1024, 10, new milog::StdLayout1());
  milog::createGlobalLogger(logdir, options.progname, "observation", milog::DEBUG, 1024, 1, new milog::StdLayout1());
  milog::createGlobalLogger(logdir, options.progname, "cachedb", milog::DEBUG, 500, 2, new milog::StdLayout1());
 
  readDatabaseConf(conf);

  //If a station is set up with this types delay them if
  //they has not at least 4 hours with data.
  continuesTypeID_.push_back(311);
  continuesTypeID_.push_back(310);
  continuesTypeID_.push_back(3);
  continuesTypeID_.push_back(330);

  valElem = conf->getValue("bufr_tables");

  if (!valElem.empty())
    bufr_tables = valElem[0].valAsString();

  if (!bufr_tables.empty()) {
    if (bufr_tables[0] != '/') {
      LOGERROR("bufr_tables must be an absolute path '" << bufr_tables << "'.");
    }

    if (*bufr_tables.rbegin() != '/')
      bufr_tables += '/';
  }

  valElem = conf->getValue("bufr_tables_names");

  if (!valElem.empty()) {
    string t = valElem[0].valAsString();

    if (!t.empty() && (t[0] == 't' || t[0] == 'T'))
      bufr_tables_names = true;
  }

  IDLOGINFO("main", "BUFR_TABLES=" << bufr_tables);

  setenv("BUFR_TABLES", bufr_tables.c_str(), 1);

  if (getenv("BUFR_TABLES")) {
    LOGINFO("BUFR_TABLES='" << getenv( "BUFR_TABLES") << "'.");
  } else {
    LOGINFO("Failed to set BUFR_TABLES.");
  }

  if (bufr_tables_names) {
    IDLOGINFO("main", "BUFR_TABLE_NAMES=true");
    setenv("PRINT_TABLE_NAMES", "true", 1);
  } else {
    IDLOGINFO("main", "BUFR_TABLE_NAMES=false");
    setenv("PRINT_TABLE_NAMES", "false", 1);
  }

  valElem = conf->getValue("accept_all_obstimes");

  if (!valElem.empty()) {
    string t = valElem[0].valAsString();

    if (!t.empty() && (t[0] == 't' || t[0] == 'T'))
      acceptAllTimes_ = true;
  }

  if (acceptAllTimes_) {
    IDLOGINFO("main", "Accepting all obstimes.");
  } else {
    IDLOGINFO("main", "Rejecting obstimes that is too old or to early.");
  }



  if (!readStationInfo(conf)) {
    LOGFATAL("Exit! No configuration!");
    exit(1);
  }

  //Create cacheDB if it does not exist.
  checkAndInitCacheDB(getCacheDbFile());

  dbThread = boost::shared_ptr<KvDbGateProxyThread>(new KvDbGateProxyThread(boost::shared_ptr<MyConnectionFactory>(new MyConnectionFactory(this))));
  dbThread->start();

  readWaitingElementsFromDb();

  //We dont need conf any more.
  delete conf;
}

App::~App() {
}


std::string App::getCacheDbFile()const {
  return dbConnect;
}

void App::readWaitingElementsFromDb() {
  list<TblWaiting> data;
  list<TblWaiting>::iterator it;

  kvDbGateProxy gate(dbThread->dbQue);
  gate.busytimeout(300);

  if (gate.select(data, " order by delaytime")) {
    for (it = data.begin(); it != data.end(); it++) {
      StationInfoList info = findStationInfoWmono(it->wmono());

      if (info.empty()) {
        gate.remove(*it);
        continue;
      }

      for (StationInfoList::iterator itInfo = info.begin(); itInfo != info.end(); ++itInfo) {
        try {
          waitingList.push_back(WaitingPtr(new Waiting(it->delaytime(), it->obstime(), *itInfo)));
        } catch (...) {
          LOGFATAL("NOMEM: while reading 'waiting' from database!");
          exit(1);
        }
      }
    }
  } else {
    LOGERROR("ERROR (Init): While reading 'waiting' from database!" << endl << gate.getErrorStr());
  }
}

void App::continuesTypeID(const std::list<int> &continuesTimes) {
  Lock lock(mutex);
  continuesTypeID_ = continuesTimes;
}

std::list<int> App::continuesTypeID() {
  Lock lock(mutex);

  return continuesTypeID_;
}

/**
 * Return true if the station has only typeid that is NOT in the list
 * continuesTypeID.
 */
bool App::onlyNoContinuesTypeID(StationInfoPtr st) {
  Lock lock(mutex);

  StationInfo::TLongList tp = st->typepriority();
  StationInfo::ITLongList itp;

  for (list<int>::iterator it = continuesTypeID_.begin(); it != continuesTypeID_.end(); it++) {

    for (itp = tp.begin(); itp != tp.end(); itp++) {
      if (*it == *itp)
        return false;
    }
  }

  return true;
}

bool App::isContinuesType(int typeID) {
  Lock lock(mutex);

  for (list<int>::iterator it = continuesTypeID_.begin(); it != continuesTypeID_.end(); it++) {

    if (*it == typeID) {
      return true;
    }
  }

  return false;
}

bool App::readStationInfo(miconf::ConfSection *conf) {
  StationInfoParse theParser;
  StationList tmpList;

  if (!theParser.parse(conf, tmpList)) {
    LOGFATAL("Cant parse the SYNOP configuration.");
    return false;
  }

  Lock lock(mutex);
  stationList = tmpList;

  return true;
}

bool App::readStationInfo(std::list<StationInfoPtr> &stList) const {
  StationInfoParse theParser;

  LOGDEBUG2("Reading conf from file!" << endl << "<"<<confFile<<">" <<endl);

  miutil::conf::ConfSection *conf = miutil::conf::ConfParser::parse(confFile);

  if (!conf)
    return false;

  stList.clear();

  bool ret = true;

  if (!theParser.parse(conf, stList)) {
    LOGWARN("Cant parse the BUFFER configuration!" << endl << "File: <" << confFile << ">" << endl);
    ret = false;
  }

  delete conf;

  return ret;
}

StationList App::getStationList() const {
  Lock lock(mutex);

  return stationList;
}

dnmi::db::Connection*
App::createKvDbConnection() {
  dnmi::db::Connection *con;

  con = dnmi::db::DriverManager::connect(kvDbDriverId, kvDbConnect);

  if (!con) {
    LOGERROR("Can't create a database connection  (" << kvDbDriverId << ")" << endl << "Connect string: <" << kvDbConnect << ">!");
    return 0;
  }

  LOGDEBUG("New database connection (" << kvDbDriverId << ") created!");
  return con;
}

void App::releaseKvDbConnection(dnmi::db::Connection *con) {
  LOGDEBUG("Database connection released (" << kvDbDriverId << ").");
  dnmi::db::DriverManager::releaseConnection(con);
}



dnmi::db::Connection*
App::createDbConnection() {
  dnmi::db::Connection *con;

  con = dnmi::db::DriverManager::connect(dbDriverId, dbConnect);

  if (!con) {
    LOGERROR("Can't create a database connection  (" << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!");
    return 0;
  }

  LOGDEBUG3("New database connection (" << dbDriverId << ") created!");
  return con;
}

void App::releaseDbConnection(dnmi::db::Connection *con) {
  LOGDEBUG3("Database connection released.");
  dnmi::db::DriverManager::releaseConnection(con);
}

StationInfoList App::findStationInfo(long stationid, long typeId, const boost::posix_time::ptime &obstime) {
  Lock lock(mutex);

  return stationList.findStation(stationid, typeId, obstime);
}

StationInfoList App::findStationInfoWmono(int wmono) {
  Lock lock(mutex);
  return stationList.findStationByWmono(wmono);
}

WaitingPtr App::addWaiting(WaitingPtr w, bool replace) {
  Lock lock(mutex);
  IWaitingList it;
  for (it = waitingList.begin(); it != waitingList.end(); it++) {
    if (*w->info() == *(*it)->info() && w->obstime() == (*it)->obstime()) {

      if (replace) {
        if (w->delay() != (*it)->delay()) {
          LOGINFO("Replace delay element for: " << w->info()->toIdentString() << " obstime: " << w->obstime() << " delay to: " << w->delay());
          *it = w;
          if (!w->addToDb()) {
            LOGERROR(
                "DBERROR while replaceing delay element for: " << w->info()->toIdentString() << " obstime: " << w->obstime() << " delay to: " << w->delay());
          }
        }
      }
      return *it;
    }
  }

  LOGINFO("Add delay element for: " << w->info()->toIdentString() << " obstime: " << w->obstime() << " delay to: " << w->delay());

  if (!w->addToDb()) {
    LOGERROR("DBERROR while adding delay element for: " << w->info()->toIdentString() << " obstime: " << w->obstime() << " delay to: " << w->delay());
  }
  //We have no record for this station and obstime in the waitingList.

  if (waitingList.empty()) {
    waitingList.push_back(w);
    return WaitingPtr();
  }

  for (it = waitingList.begin(); it != waitingList.end(); it++) {
    if (w->delay() <= (*it)->delay()) {
      break;
    }
  }

  waitingList.insert(it, w);

  return WaitingPtr();
}

WaitingPtr App::getWaiting(const pt::ptime &obstime, StationInfoPtr info) {
  Lock lock(mutex);
  IWaitingList it;

  for (it = waitingList.begin(); it != waitingList.end(); it++) {
    if (*(*it)->info() == *info && (*it)->obstime() == obstime) {
      (*it)->removeFrom();
      WaitingPtr w = *it;
      waitingList.erase(it);

      return w;
    }
  }

  return WaitingPtr();
}

WaitingList App::getExpired() {
  WaitingList wl;
  IWaitingList it;
  IWaitingList itTmp;
  pt::ptime now;
  ostringstream ost;
  bool msg = false;

  Lock lock(mutex);

  milog::LogContext context("Delay");

  now = pt::second_clock::universal_time();
  it = waitingList.begin();

  while (it != waitingList.end() && (*it)->delay() <= now) {
    LOGDEBUG("getExpired: loop");
    if (!msg) {
      ost << "Expired delay for stations at time: " << now << endl;
      msg = true;
    }
    ost << "-- " << (*it)->info()->toIdentString() << " obstime: " << pt::to_kvalobs_string((*it)->obstime()) << " delay to: " << pt::to_kvalobs_string((*it)->delay()) << endl;
    wl.push_back(*it);
    waitingList.erase(it);
    it = waitingList.begin();
  }

  if (msg) {
    LOGINFO(ost.str());
  }

  if (!wl.empty()) {
    for (it = wl.begin(); it != wl.end(); it++) {
      (*it)->removeFrom();
    }
  }

  return wl;
}


void App::removeWaiting(StationInfoPtr info, const pt::ptime &obstime) {
  IWaitingList it;

  Lock lock(mutex);

  for (it = waitingList.begin(); it != waitingList.end(); it++) {
    if (*info == *(*it)->info() && obstime == (*it)->obstime()) {

      if (!(*it)->removeFrom()) {
        LOGWARN("Cant remove waiting element from database:  " << (*it)->info()->toIdentString() << " obstime: " << obstime);

      }

      LOGINFO("Removed waiting element: " << (*it)->info()->toIdentString() << " obstime: " << obstime << endl << " with delay: " << (*it)->delay());

      waitingList.erase(it);

      return;
    }
  }
}

void App::removeWaiting(WaitingPtr w) {
  LOGINFO("remove: " << w->info()->toIdentString() << " obstime: " << pt::to_kvalobs_string(w->obstime()) << " delay to: " << pt::to_kvalobs_string(w->delay()) << " from database!");

  w->removeFrom();
}

pt::ptime App::checkpoint() {
  list<TblKeyVal> data;
  list<TblKeyVal>::iterator it;

  kvDbGateProxy gate(dbThread->dbQue);

  gate.busytimeout(300);

  if (!gate.select(data, "WHERE key=\'checkpoint\'")) {
    LOGERROR("DBERROR: cant obtain checkpoint!");
    return pt::ptime();
  }

  if (data.empty()) {
    LOGINFO("No checkpont!");
    return pt::ptime();
  }

  return pt::time_from_string(data.front().val());
}

void App::createCheckpoint(const pt::ptime &cpoint) {
  if (cpoint.is_special()) {
    LOGERROR("Checkpoint: undef checkpont time!");
    return;
  }

  kvDbGateProxy gate(dbThread->dbQue);
  gate.busytimeout(300);

  if (cpoint.is_special()) {
    LOGERROR("Checkpoint: undef checkpont time!");
    return;
  }

  if (!gate.insert(TblKeyVal("checkpoint", pt::to_kvalobs_string(cpoint)), true)) {
    LOGERROR("Failed to create checkpoint! (" <<pt::to_kvalobs_string(cpoint) <<") \nReason: " << gate.getErrorStr());
  } else {
    LOGINFO("Checkpoint created at: " << pt::to_kvalobs_string(cpoint));
  }
}

StationInfoPtr App::replaceStationInfo(StationInfoPtr newInfoPtr) {
  Lock lock(mutex);

  IStationList it = stationList.begin();

  for (; it != stationList.end(); it++) {
    if (*(*it) == *newInfoPtr) {

      //Set the cacheReload of the new StationInfo to the same as
      //reload.
      newInfoPtr->cacheReloaded48((*it)->cacheReloaded48());
      StationInfoPtr info = *it;
      *it = newInfoPtr;
      return info;
    }
  }

  return StationInfoPtr();
}

bool App::addStationInfo(StationInfoPtr newInfoPtr) {

  Lock lock(mutex);
  IStationList it = stationList.begin();

  for (; it != stationList.end(); it++) {
    if (*(*it) == *newInfoPtr) {
      return false;
    }
  }

  stationList.push_back(newInfoPtr);

  return true;
}

/**
 * Replace the current configuration with the new one.
 * @param newConf The new configuration to replace the old.
 */
void App::replaceStationConf(const StationList &newConf) {
  Lock lock(mutex);

  stationList = newConf;
}

bool App::getSavedBufrData(StationInfoPtr info, const pt::ptime &obstime, std::list<TblBufr> &tblBufr) {
  kvDbGateProxy gate(dbThread->dbQue);
  ostringstream ost;

  gate.busytimeout(300);

  ost << " WHERE wmono=" << info->wmono() << " AND id=" << info->stationID() << " AND callsign='" << info->callsign() << "'" << " AND obstime='" << pt::to_kvalobs_string(obstime)
      << "'";

  if (!gate.select(tblBufr, ost.str())) {
    LOGERROR("DBERROR: getSavedBufrData: " << gate.getErrorStr());
    return false;
  }

  return true;
}

bool App::saveBufrData(const TblBufr &tblBufr) {
  kvDbGateProxy gate(dbThread->dbQue);

  gate.busytimeout(300);

  if (!gate.insert(tblBufr, true)) {
    LOGERROR("DBERROR: saveBufrData: " << gate.getErrorStr());
    return false;
  }

  return true;
}

bool App::getDataFrom(const pt::ptime &t, int wmono, int hours, std::shared_ptr<dnmi::thread::CommandQue> que) {
  LogContext lContext("getDataFrom");

  LOGINFO("Get data from server, start time: " << t);

  GetData *getData;

  try {
    getData = new GetData(*this, t, wmono, hours, que);
  } catch (...) {
    LOGERROR("NO MEM!");
    return false;
  }

  try {
    //Create and start a background thread to receive the
    //data from kvalobs.
    getData->setThread(new boost::thread(*getData));
  } catch (...) {
    LOGERROR("NO MEM!");
    delete getData;
    return false;
  }

  Lock lock(mutex);

  getDataThreads.push_back(getData);

  return true;
}

bool App::joinGetDataThreads(bool waitToAllIsJoined, const std::string &logid) {
  Lock lock(mutex);
  std::list<GetData*>::iterator it = getDataThreads.begin();
  bool joined = false;

  IDLOGDEBUG(logid, "# " << getDataThreads.size() << " getDataThreads!");

  for (; it != getDataThreads.end(); it++) {
    if (waitToAllIsJoined) {
      (*it)->join();
      delete *it;
      it = getDataThreads.erase(it);
      joined = true;
    } else {
      if ((*it)->joinable()) {
        (*it)->join();
        delete *it;
        it = getDataThreads.erase(it);
        joined = true;
      }
    }
  }

  return joined;
}

void App::cacheReloaded(StationInfoPtr info) {
  Lock lock(mutex);
  info->cacheReloaded48(true);
}

void App::cacheReload(StationInfoPtr station){
  Lock lock(mutex);
  station->cacheReloaded48(false);
}

StationList App::reloadCache(int wmono, int id) {
  Lock lock(mutex);
  StationList myStationList;

  IStationList it = stationList.begin();

  if (wmono < 0) {
    for (; it != stationList.end(); it++) {
      (*it)->cacheReloaded48(false);
      myStationList.push_back(*it);
    }
  } else {
    for (; it != stationList.end(); it++) {
      if ((*it)->wmono() == wmono && (*it)->stationID() == id) {
        (*it)->cacheReloaded48(false);
        myStationList.push_back(*it);
        break;
      }
    }
  }

  if (!myStationList.empty()) {
    hasStationWaitingOnCacheReload = true;
  }

  return myStationList;
}


void App::addObsEvent(ObsEvent *event, dnmi::thread::CommandQue &que) {
  Lock lock(mutex);

  if (!hasStationWaitingOnCacheReload) {
    try {
      que.postAndBrodcast(event);
    } catch (...) {
      delete event;
    }

    return;
  }

  for (IStationList it = stationList.begin(); it != stationList.end(); it++) {
    if (*event->stationInfo() == **it) {
      if ((*it)->cacheReloaded48()) {
        //The cache for the station is reloaded post the event to
        //the event que and return.
        try {
          que.postAndBrodcast(event);
        } catch (...) {
          delete event;
        }
        return;
      } else {
        //Break out of the loop and add the event to
        //the obsEventWaitingOnCacheReload.
        break;
      }
    }
  }

  for (std::list<ObsEvent*>::iterator it = obsEventWaitingOnCacheReload.begin(); it != obsEventWaitingOnCacheReload.end(); it++) {
    if ((*it)->obstime() == event->obstime() && *(*it)->stationInfo() == *event->stationInfo()) {
      delete *it;
      *it = event;
      return;
    }
  }

  obsEventWaitingOnCacheReload.push_back(event);
}

bool App::checkObsEventWaitingOnCacheReload(dnmi::thread::CommandQue &que, const std::string &logid) {
  Lock lock(mutex);

  IDLOGDEBUG(logid, "CheckObsEventWaitingOnCacheReload called!");

  if (!hasStationWaitingOnCacheReload) {
    IDLOGDEBUG(logid, "No Station waiting on reload!!");
    return false;
  }

  for (std::list<ObsEvent*>::iterator it = obsEventWaitingOnCacheReload.begin(); it != obsEventWaitingOnCacheReload.end(); it++) {

    for (IStationList sit = stationList.begin(); sit != stationList.end(); sit++) {
      if (*(*it)->stationInfo() == **sit) {
        if ((*sit)->cacheReloaded48()) {
          IDLOGINFO(logid, "The cache is reloaded for station: " << (*sit)->toIdentString() << " obstime: " << (*it)->obstime());

          ObsEvent *event = *it;
          it = obsEventWaitingOnCacheReload.erase(it);
          //The cache for the station is reloaded post the event to
          //the event que.
          try {
            que.postAndBrodcast(event);
          } catch (...) {
            LOGERROR("Cant post event to the eventque for the station: " << (*sit)->toIdentString());
            IDLOGERROR(logid, "Cant post event to the eventque for the station: " << (*sit)->toIdentString());

            delete event;
          }
        }

        //Break out of the loop.
        break;
      }
    }
  }

  //If the list obsEventWaitingOnCacheReload is empty
  //check the stationList to see if all stations is reloaded. If so
  //set hasStationWaitingOnCacheReload to false.
  if (obsEventWaitingOnCacheReload.empty()) {
    bool allReloaded = true;
    ostringstream ost;

    for (IStationList it = stationList.begin(); it != stationList.end(); it++) {
      if (!(*it)->cacheReloaded48()) {
        ost << (*it)->toIdentString() << " ";
        allReloaded = false;
      }
    }

    if (!allReloaded) {
      IDLOGDEBUG(logid, "This stations is not reloaded with new data:" << endl << ost.str());
    } else {
      IDLOGINFO(logid, "All stations reloaded with data from kvalobs.");
      hasStationWaitingOnCacheReload = false;
    }
  }

  return hasStationWaitingOnCacheReload;
}

bool App::shutdown() {
  return AppShutdown;
}

void App::run(std::shared_ptr<dnmi::thread::CommandQue> newDataQue) {
  int idleCheck=0;
  std::unique_ptr<KvDataConsumer> consumer;

  if( !options.diasableDataReceiver )
    consumer.reset(new KvDataConsumer(kafkaDomain, kafkaBrokers, newDataQue));

  while (!shutdown()){
    if( consumer )
      consumer->runOnce(1000);
    else
      std::this_thread::sleep_for(std::chrono::seconds(1));

    idleCheck = (idleCheck+1)%60;
    if( idleCheck==0)
      kvDbPool.releaseIdleConnections();
  }

  if( consumer )
    consumer->stopAll();
}

void decodeArgs(int argn, char **argv, Opt &opt) {
  struct option long_options[] =
    {
        { "config-file", 1, 0, 'c' },
        { "pidfile", 1, 0, 'p' },
        { "fromtime", 1, 0, 'f' },
        { "help", 0, 0, 'h' },
        { "disable-data-receiver", 0, 0, 'd'},
        { "loglevel", 1, 0, 'l'},
        { 0, 0, 0, 0 }
    };

  opt.progname = getProgNameFromArgv0(argv[0]);
  opt.diasableDataReceiver=false;

  int c;
  int index;

  while (true) {
    c = getopt_long(argn, argv, "hp:cd:l:", long_options, &index);

    if (c == -1)
      break;

    switch (c) {
      case 'h':
        usage(opt.progname, 1);
        break;
      case 'd':
        opt.diasableDataReceiver=true;
        break;
      case 'p':
        opt.pidfile = optarg;
        break;
      case 'c':
        opt.conffile = optarg;
        break;
      case 'l':
        opt.loglevel = parseMilogLogLevel(optarg);
        break;
      case 'f': {
        string tmp(optarg);
        pt::ptime fromTime;

        if (tmp.find_first_of("-:") == string::npos) {
          int n = atoi(optarg);
          if (n > 0) {
            fromTime = pt::second_clock::universal_time();
            fromTime -= pt::hours(n);
          }
        } else {
          fromTime = pt::time_from_string(optarg);
        }

        if (fromTime.is_special()) {
          cerr << "Invalid from time '" << optarg << "'. " << "Format YYYY-MM-DD hh:mm:ss or hours" << endl;
          usage(opt.progname, 1);
        } else {
          opt.fromTime = fromTime;
        }
      }
        break;
      case '?':
        cout << "Unknown option : <" << (char) optopt << ">!" << endl;
        cout << opt.progname << " -h for help.\n\n";
        usage(opt.progname, 1);
        break;
      case ':':
        cout << optopt << " missing argument!" << endl;
        usage(opt.progname, 1);
        break;
      default:
        cout << "?? option caharcter: <" << (char) optopt << "> unknown!" << endl;
        usage(opt.progname, 1);
        break;
    }
  }

  if (opt.conffile.empty()) {
    opt.conffile = kvPath("sysconfdir") + "/" + opt.progname + ".conf";
  } else if (*opt.conffile.begin() != '/') {
    opt.conffile = kvPath("sysconfdir") + "/" + opt.conffile;
  }

  if (opt.pidfile.empty()) {
    opt.pidfile = dnmi::file::createPidFileName(kvPath("rundir"), opt.progname);
  } else if (*opt.pidfile.begin() != '/') {
    opt.pidfile = kvPath("rundir") + "/" + opt.pidfile;
  }

}

void usage(const std::string &progname, int exitCode) {
  cout << "\n " << progname << " is a program that creates BUFR message from kvalobs."
       << "\n\nUSAGE " << "\n"
       << progname << " [[--config-file|-c] conffile] [[--pidfile|-p] pidfile]"
       << "\n\t   [[--fromtime|-f] 'YYYY-MM-DD hh:mm:ss' or hours back"
       << "\n\t   [--disable-data-receiver|-d]\n"
       << "\n\t [--disable-data-receiver|-d] disable receiving of data from kvalobs."
       << "\n\t      This option is mostly used for debugging."
       << "\n\t [--config-file|-c] configfile Use the configfile. If the name is not"
       << "\n\t      an absolute path the file is looked up relative to " << kvPath("sysconfdir")
       << "\n\t      Default value is set to " << kvPath("sysconfdir") << "/" << progname << ".conf"
       << "\n\t [--pidfile|-p] pidfile Use this as the pid file. "
       << "\n\t      Default value " << kvPath("rundir") << "/" << progname << "-node.pid"
       << "\n\t      Where node is determined by the hostname." << "\n\n";
  exit(exitCode);
}

string getProgNameFromArgv0(const std::string &cmd) {
  fs::path myname(cmd);

#if BOOST_VERSION < 104500
  return myname.leaf();
#else
  return myname.filename().string();
#endif
}

