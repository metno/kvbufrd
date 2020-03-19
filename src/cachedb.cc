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

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include "cachedb.h"
#include "utils.h"
#include "kvalobs/fileutil/fileutil.h"
#include "kvalobs/kvdb/dbdrivermgr.h"
#include "kvalobs/kvdb/kvdb.h"
#include "kvalobs/milog/milog.h"



namespace {
const std::string cacheDbDriverId("sqlite3");
const std::string sqliteDriver("sqlite3driver");
//std::string cachedb(std::string(CACHEDIR) + "/kvbufrd.sqlite");
std::mutex mu;
bool driverLoaded = false;
//CREATE TABLE data (stationid integer, obstime timestamp, tbtime timestamp   DEFAULT CURRENT_TIMESTAMP, original text, paramid integer, typeid integer, sensor integer, level integer, controlinfo text, useinfo text,  UNIQUE(stationid, obstime, paramid, typeid, level, sensor))
const char* cacheSchema[] = {
  "CREATE TABLE data (stationid integer, obstime timestamp, tbtime timestamp "
  "  DEFAULT CURRENT_TIMESTAMP, original text, paramid integer, typeid integer, "
  "  sensor integer, level integer, controlinfo text, useinfo text, "
  " UNIQUE(stationid, obstime, paramid, typeid, level, sensor)"
  ");",
  "CREATE TABLE bufr(wmono integer, id integer, callsign text, code text, "
  "  obstime timestamp, createtime timestamp, crc integer, ccx integer, data "
  "  text, bufrBase64 text, tbtime timestamp DEFAULT CURRENT_TIMESTAMP, "
  " UNIQUE(wmono, id, callsign, code, obstime)"
  ");",
  "CREATE TABLE waiting( wmono integer, id integer, callsign text, code text, "
  "  obstime timestamp, delaytime timestamp, "
  " UNIQUE(wmono, id, callsign, code,  obstime) "
  ");",
  "CREATE TABLE keyval(key text, val text, UNIQUE(key));",
  "CREATE INDEX data_stationid_obstime_index on data (stationid,obstime);",
  "CREATE INDEX data_tbtime_index on data (tbtime);",
  "CREATE INDEX bufr_obstime_index on bufr (obstime);",
  "CREATE INDEX bufr_tbtime_index on data (tbtime);",
  "PRAGMA user_version = 1;",
  0
};

const char *truncateTbls[] = {
  "data",
  "bufr",
  "waiting",
  0
};
}

namespace pt = boost::posix_time;
namespace fs = boost::filesystem;
namespace db = dnmi::db;
using db::DriverManager::connect;
using db::DriverManager::getErr;
using db::DriverManager::listDrivers;
using db::DriverManager::loadDriver;
using db::DriverManager::releaseConnection;
using namespace std;

void
loadCacheDriver()
{
  std::lock_guard<std::mutex> lock(mu);
  if (driverLoaded)
    return;

  for (auto& driverId : listDrivers()) {
    if (driverId == cacheDbDriverId) {
      driverLoaded = true;
      return;
    }
  }

  std::string driverId;

  if (!loadDriver(sqliteDriver, driverId)) {
    auto err = getErr();
    LOGFATAL("Failed to load cacheDB driver <" << sqliteDriver
                                               << ">. Error: " << err);
    std::cerr << "Failed to load cacheDB driver <" << sqliteDriver
              << ">. Error: " << err;
    exit(2);
  }

  if (driverId != cacheDbDriverId) {
    std::cerr << "NOMATCH: driverIds: '" << driverId << "'\n";
  }
}



void
truncateCacheDB(const std::string &cachedb)
{
  db::Connection* con = connect(cacheDbDriverId, cachedb);

  if (!con) {
    IDLOGFATAL("cachedb", "Failed to create connection to cacheDB '" << cachedb
                                          << "'. Error: " << getErr());
    LOGFATAL("Failed to create connection to cacheDB '" << cachedb
                                          << "'. Error: " << getErr());
   cerr << "Failed to create connection to cacheDB '" << cachedb << "'. Error: " << getErr() << endl << endl;
   exit(2);
  }


  string tbl;
  try {
    for (int i = 0; truncateTbls[i]; ++i) {
      std::ostringstream q;
      tbl = truncateTbls[i];
      q << "DELETE " << tbl << ";";
      con->exec(q.str());
      IDLOGINFO("cachedb", "Truncated table '" <<tbl << "' cacheDB '" << cachedb << "'");

      try {
        con->exec("VACUUM;");
      }
      catch (const exception& ex ) {
        IDLOGFATAL("cachedb", "VACUUM failed cacheDB '" << cachedb << "'. "  << ex.what());
        LOGFATAL("VACUUM failed cacheDB '" << cachedb << "'. "   << ex.what());
        cerr << "VACUUM failed cacheDB '" << cachedb << "'. " << ex.what() << endl << endl;
        exit(2);
      }
    }
    releaseConnection(con);
  } catch (const exception& ex) {
    IDLOGFATAL("cachedb", "Failed to trucate tables. Failed table '" << tbl << "' cacheDB schema '" << cachedb
      << "'. Error: " << ex.what());
    LOGFATAL("Failed to trucate tables. Failed table '" << tbl << "' cacheDB schema '" << cachedb
      << "'. Error: " << ex.what());
    cerr << "Failed to trucate tables. Failed table '" << tbl << "' cacheDB schema '" << cachedb
      << "'. Error: " << ex.what();
    releaseConnection(con);
    unlink(cachedb.c_str());
    exit(2);
  }
}



void
createCacheDB(const std::string &cachedb)
{
  db::Connection* con = connect(cacheDbDriverId, cachedb);

  if (!con) {
    LOGFATAL("Failed to create cacheDB '" << cachedb
                                          << "'. Error: " << getErr());
   cerr << "Failed to create cacheDB '" << cachedb << "'. Error: " << getErr() << endl << endl;
   exit(2);
  }

  
  try {
    for (int i = 0; cacheSchema[i]; ++i) {
      con->exec(cacheSchema[i]);
    }
    releaseConnection(con);
    IDLOGINFO("cachedb", "Created cacheDB '" << cachedb << "'");
    LOGINFO("Created cacheDB '" << cachedb << "'");
    cerr << "Created cacheDB '" << cachedb << "'" << endl;
  } catch (const exception& ex) {
    IDLOGFATAL("cachedb", "Failed to create cacheDB schema '" << cachedb
      << "'. Error: " << ex.what());
    LOGFATAL("Failed to create cacheDB schema '" << cachedb
      << "'. Error: " << ex.what());
    releaseConnection(con);
    unlink(cachedb.c_str());
    exit(2);
  }
}

// Compute the nearest time back in time to the main synop times 0, 6, 12, 18.
pt::ptime
computeToOld()
{
  pt::ptime now(pt::second_clock::universal_time());
  pt::time_duration clk(pt::time_duration(now.time_of_day().hours(), 0, 0));
  now = pt::ptime(now.date(), clk);
  int h = now.time_of_day().hours();

  return now - pt::hours(h % 6);
}

bool
cacheIsToOld(const std::string& cachedb, pt::ptime* pToOld)
{
  pt::ptime toOld(computeToOld());
  pt::ptime ft(util::file::modificationTime(cachedb));

  if (ft.is_special()) {
    throw std::logic_error(string("The file '") + cachedb +
                           "' do not exist (strange).");
  }

  if (pToOld) {
    *pToOld = toOld;
  }

  if (ft < toOld) {
    return true;
  }

  return false;
}

/**
 * returns Start BUFR production from. 
 */
boost::posix_time::ptime checkAndUpdateCache();

void
checkAndInitCacheDB(const std::string &cachedb_)
{
  ostringstream ost;
  fs::path cachedb(cachedb_);

  loadCacheDriver();

  try {
    auto cachedir = cachedb.parent_path();
     
    if ( cachedir.empty() || ! cachedir.is_absolute() ) {
      LOGFATAL("Config <database.cache.connect> '" << cachedb_ << "'must bee an absolute path.");
      cerr << "Config <database.cache.connect> '" << cachedb_ << "'must bee an absolute path." << endl << endl;
      exit(2);
    }

    ost << "Failed to check existence of directory '" << cachedir <<"'.";
    if ( ! fs::exists(cachedir) ) {
      ost.str("");
      ost << "Failed to create directory '" << cachedir <<"' for the cache database.";
      if (!fs::create_directories(cachedir) ) {
        LOGFATAL( ost.str());
        cerr << ost.str() << endl << endl;
        exit(2);
      }
    } else {
      ost.str("");
      ost << "Failed to check if '" << cachedir <<"' is a directory.";
      if ( ! fs::is_directory(cachedir) ) {
        LOGFATAL("The directory path '" << cachedir << "' exist, but is not a directory.");
        cerr << "The directory path '" << cachedir << "' exist, but is not a directory." << endl << endl;
        exit(2);
      }
    }
    ost.str("");
    ost << "Failed to check if cachedb file '" << cachedb << "' exist or is a regular file.";
    if( fs::exists(cachedb) ) {
      if( !fs::is_regular_file(cachedb) ) {
        LOGFATAL("The cachedb directory path '" << cachedb <<"' exist, but is not a regular file.");
        cerr << "The cachedb directory path '" << cachedb <<"' exist, but is not a regular file."<< endl << endl;
        exit(2);
      }

      pt::ptime mt = pt::from_time_t( fs::last_write_time(cachedb) );
      pt::ptime now=pt::second_clock::universal_time()-pt::hours(48);
      if ( mt > now ) {
        return;
      }
      LOGINFO("The cachedb file '" << cachedb <<"' exist, but is not touched in 48 hours. Deleting and start with a fresh cache.")
      cerr << "The cachedb file '" << cachedb <<"' exist, but is not touched in 48 hours. Deleting and start with a fresh cache."<< endl << endl;
      ost.str("");
      ost << "Failed to delete cachedb '" << cachedb << "'.";
      truncateCacheDB(cachedb.string());
      return;
    } 
    ost.str("");
    //The cachedb does not exist, create it.

    createCacheDB(cachedb.string());
  } 
  catch (const fs::filesystem_error &ex ) {
    LOGFATAL(ost.str() << " Reason: "  << ex.what());
    exit(2);
  }
  catch (const std::exception& ex) {
    LOGFATAL("Failed to init cacheDB '" << cachedb << "'. Error: " << ex.what()
                                        << ".");
    cerr << "FATAL: Failed to init cacheDB '" << cachedb
         << "'. Error: " << ex.what() << ".\n";
    exit(2);
  }
}
