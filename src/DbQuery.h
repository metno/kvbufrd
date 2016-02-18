/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#ifndef SRC_DBQUERY_H_
#define SRC_DBQUERY_H_

#include <functional>
#include <list>
#include <stdexcept>
#include <boost/date_time/posix_time/ptime.hpp>
#include "decodeutility/kvalobsdata.h"
#include "kvalobs/kvData.h"
#include "kvdb/kvdb.h"
#include "KvObsData.h"

namespace kvalobs {
namespace sql {
class DbQuery  {
 public:
  explicit DbQuery(std::function<std::shared_ptr<dnmi::db::Connection> ()> createConnection);
  virtual ~DbQuery();

  /**
   * Get the data for stationid and typeid between from and to, not including to.
   * If typeid is 0 get the data for all typeid for this station.
  * @throw std::logic_error
  */
  void getKvData( std::list<kvalobs::kvData> *data, long stationid, long typeID, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime());
  void getKvData( kvalobs::serialize::KvalobsData *data,std::list<long> stationids, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime());
  void getKvData( kvalobs::KvObsData *data,std::list<long> stationids, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool onlyKvData=true);


  bool getKvData( std::function<void(long stationid, long typeID, const boost::posix_time::ptime &obstime, const kvalobs::ObsDataElement &data)> func,
                  const std::list<long> &stationids, const std::list<long> &types, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool onlyKvData=false);
  void getKvData( kvalobs::KvObsDataMap *data, std::list<long> stationids, std::list<long> typeIDs, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool onlyKvData=false);

  private:
    typedef std::shared_ptr<dnmi::db::Connection> ConPtr;
    ConPtr getConnection();
    void getKvData_( dnmi::db::Connection *con, std::list<kvalobs::kvData> *data, long stationid, long type, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool sorted=true);
    void getKvTextData_( dnmi::db::Connection *con, std::list<kvalobs::kvTextData> *data, long stationid, long type, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool sorted=true);
    void getKvData_( dnmi::db::Connection *con, std::list<kvalobs::kvData> *data, const std::list<long> &stationids, long type, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool sorted=true);
    void getKvTextData_( dnmi::db::Connection *con, std::list<kvalobs::kvTextData> *data, const std::list<long> &stationids, long type, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime(), bool sorted=true);
    void callback(std::function<void(long stationid, long typeID, const boost::posix_time::ptime &obstime, const kvalobs::ObsDataElement &data)> func, const kvalobs::ObsDataElement &data );
    std::function<std::shared_ptr<dnmi::db::Connection> ()> createConnection_;
};



}  // namespace sql
}  // namespace kvalobs




#endif /* SRC_DBQUERY_H_ */
