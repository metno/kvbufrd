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

#include <algorithm>
#include <sstream>
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "DbQuery.h"

namespace pt = boost::posix_time;

using std::string;
using std::move;
using std::make_tuple;
using std::tuple;
using std::get;
using std::list;
using std::unique;
using std::shared_ptr;
using kvalobs::kvData;
using kvalobs::kvTextData;

namespace {
template<class T>
list<T> unique(list<T> l) {
  l.sort();
  typename list<T>::iterator end = unique(l.begin(), l.end());
  return list<T>(l.begin(), end);
}


std::string isotime(const pt::ptime &dt) {
  if (dt.is_infinity()) {
    return "'+infinity'";
  } else if (dt.is_neg_infinity()) {
    return "'-infinity'";
  } else if (!dt.is_special()) {
    std::ostringstream s;
    boost::gregorian::date d = dt.date();
    s << "'"<< d.year() << '-' << std::setfill('0') << std::setw(2) << std::right << d.month().as_number() << '-' << std::setfill('0') << std::setw(2) << std::right
      << d.day() << "T" << pt::to_simple_string(dt.time_of_day()) << "'";
    return s.str();
  } else {
    throw std::logic_error("The input must be a valid time.");
  }
}

template<class C>
class QueryResult {
 public:
  typedef C Data;
  enum CmpEqual {
    Stationid,
    StationidObstime,
    StationidTypeid,
    StationidTypeidObstime
  };
  QueryResult(std::shared_ptr<dnmi::db::Result> result, CmpEqual cmp = StationidTypeidObstime)
      : result_(result),
        cmp_(cmp) {
  }

  bool hasNext() {
    return result_ && (result_->hasNext() || !data_.empty());
  }

  bool isEqual(const Data &d1, const Data &d2) {
    if (cmp_ == Stationid)
      return d1.stationID() == d2.stationID();
    else if (cmp_ == StationidObstime)
      return d1.stationID() == d2.stationID() && d1.obstime() == d2.obstime();
    else if (cmp_ == StationidTypeid)
      return d1.stationID() == d2.stationID() && d1.typeID() == d2.typeID();
    else  // StationidTypeidObstime
      return d1.stationID() == d2.stationID() && d1.typeID() == d2.typeID() && d1.obstime() == d2.obstime();
  }

  // returns one list for each stationid, obstime, typeid, require that the query is sorted by stationid, obstime and typeid.
  std::list<Data> next() {
    while (result_->hasNext()) {
      Data current(result_->next());
      if (data_.empty() || isEqual(current, data_.front()))
        data_.push_back(current);
      else {
        std::list<Data> ret(1, current);
        std::swap(ret, data_);
        return ret;
      }
    }
    std::list<Data> ret;
    std::swap(ret, data_);
    return ret;
  }

  std::list<Data> all() {
    while (result_->hasNext())
      data_.push_back(Data(result_->next()));
    std::list<Data> ret;
    std::swap(ret, data_);
    return ret;
  }

 private:

  std::list<Data> data_;
  std::shared_ptr<dnmi::db::Result> result_;
  CmpEqual cmp_;
};

string dataOrTextDataQuery(const std::string &tbl_, const list<long> &stationids_, const list<long> &typeIDs_, const boost::posix_time::ptime &from_,
                           const boost::posix_time::ptime &to_) {
  std::ostringstream q;
  list<long> types;

  // Removes all zeros, as this, by convention, means all typeids.
  std::for_each(typeIDs_.begin(), typeIDs_.end(), [&](long t){ if(t!=0) types.push_back(t);});
  types = unique(types);
  list<long> stationids = unique(stationids_);

  if( stationids.empty() )
    throw std::range_error("Must have at least one statioid in thequery.");

  q << "select * from " << tbl_ << " where";

  if( stationids.size()==1) {
    q << " stationid=" << stationids.front();
  }else if( stationids.size() > 1 ) {
    auto it = stationids.begin();
    q << " stationid in (" << *it;
    for (++it; it != stationids.end(); ++it)
      q << "," << *it;
    q << ")";
  }


  if( types.size() == 1 ) {
    q <<" and  typeid=" << types.front();
  } else if( types.size() > 1 ){
    auto it = types.begin();
    q << " and typeid in (" << *it;
    for (++it; it != types.end(); ++it)
        q << "," << *it;
    q << ")";
  }

  if (from_.is_special())
    throw std::range_error("From time must be a valid time.");

  if (to_.is_special())
    q << " and obstime>=" << isotime(from_);
  else if (to_ == from_)
    q << " and obstime=" << isotime(from_);
  else
    q << " and obstime>=" << isotime(from_) << " and obstime<" << isotime(to_);

  LOGDEBUG("DataQuery: " << q.str());
  return q.str();
}

string dataQuery(const list<long> &stationid, long typeID, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to =
                     boost::posix_time::ptime()) {
  list<long> tids;

  if (typeID != 0)
    tids.push_back(typeID);
  return dataOrTextDataQuery("data", stationid, tids, from, to);
}

string textDataQuery(const std::list<long> &stationid, long typeID, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to =
                         boost::posix_time::ptime()) {
  list<long> tids;

  if (typeID != 0)
    tids.push_back(typeID);

  return dataOrTextDataQuery("text_data", stationid, tids, from, to);
}

string dataQuery(const list<long> &stationid, const list<long> &typeIDs, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to =
                     boost::posix_time::ptime()) {
  return dataOrTextDataQuery("data", stationid, typeIDs, from, to);
}

string textDataQuery(const list<long> &stationid, const list<long> &typeIDs, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to =
                         boost::posix_time::ptime()) {
  return dataOrTextDataQuery("text_data", stationid, typeIDs, from, to);
}

std::string dataQuery(long stationid, long typeID, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to = boost::posix_time::ptime()) {
  return dataQuery( { stationid }, typeID, from, to);
}

std::string textDataQuery(long stationid, long typeID, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to = boost::posix_time::ptime()) {
  return textDataQuery( { stationid }, typeID, from, to);
}

template<class T>
void query(dnmi::db::Connection *con, std::list<T> *data, const std::string &query) {
  if (!data)
    return;

  data->clear();

  if (!con)
    return;

  do {
    try {
      std::shared_ptr<dnmi::db::Result> res(con->execQuery(query));

      if (!res)
        return;

      QueryResult<T> getRes(res);

      if (getRes.hasNext())
        *data = getRes.all();
      return;
    } catch (const dnmi::db::SQLNotConnected &ex) {
      if (!con->tryReconnect())
        throw;
    } catch (const std::exception &ex) {
      throw std::logic_error(ex.what());
    }
  } while (true);
}

template<class T>
QueryResult<T> query(dnmi::db::Connection *con, const std::string &query) {
  if (!con)
    return QueryResult<T>(nullptr);

  do {
    try {
      std::shared_ptr<dnmi::db::Result> res(con->execQuery(query));

      if (!res)
        return QueryResult<T>(nullptr);
      else
        return QueryResult<T>(res);
    } catch (const dnmi::db::SQLNotConnected &ex) {
      if (!con->tryReconnect())
        throw;
    } catch (const std::exception &ex) {
      throw std::logic_error(ex.what());
    }
  } while (true);
}

int compare(const list<kvTextData> &txtData, const list<kvData> &data) {
  if (txtData.empty() && data.empty())
    return 0;

  if (txtData.empty())
    return 1;

  if (data.empty())
    return -1;

  if (txtData.front().stationID() < data.front().stationID())
    return -1;
  else if (txtData.front().stationID() > data.front().stationID())
    return 1;
  else {
    if (txtData.front().obstime() < data.front().obstime())
      return -1;
    else if (txtData.front().obstime() > data.front().obstime())
      return 1;
    else {
      if (txtData.front().typeID() < data.front().typeID())
        return -1;
      else if (txtData.front().typeID() > data.front().typeID())
        return 1;
      else
        return 0;
    }
  }
}


}  // namespace

namespace kvalobs {
namespace sql {
DbQuery::DbQuery(std::function<std::shared_ptr<dnmi::db::Connection> ()> createConnection)
    : createConnection_(createConnection) {
}

DbQuery::~DbQuery() {
}

void DbQuery::getKvData(std::list<kvalobs::kvData> *data, long stationid, long typeID, const boost::posix_time::ptime &from,
                        const boost::posix_time::ptime &to) {
  ConPtr con = getConnection();
  data->clear();
  getKvData_(con.get(), data, stationid, typeID, from, to, true);
}

void DbQuery::getKvData(kvalobs::serialize::KvalobsData *data, std::list<long> stationids, const boost::posix_time::ptime &from,
                        const boost::posix_time::ptime &to) {
  ConPtr con = getConnection();
  data->clear();
  for (auto id : stationids) {
    std::list<kvalobs::kvData> ret;
    getKvData_(con.get(), &ret, id, 0, from, to, false);
    data->insert(ret.begin(), ret.end());
  }
}

void DbQuery::getKvData(kvalobs::KvObsData *data, std::list<long> stationids, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
                        bool onlyKvData) {
  ConPtr con = getConnection();
  data->clear();

  //Remove duplicates
  stationids = unique(stationids);

  for (auto id : stationids) {
    list<kvalobs::kvData> retKvData;
    list<kvalobs::kvTextData> retkvTextData;
    getKvData_(con.get(), &retKvData, id, 0, from, to, false);

    if (!onlyKvData)
      getKvTextData_(con.get(), &retkvTextData, id, 0, from, to, false);

    if (!retKvData.empty())
      data->emplace(id, move(make_tuple(move(retKvData), move(retkvTextData))));
  }
}

bool DbQuery::getKvData(std::function<void(long stationid, long typeID, const boost::posix_time::ptime &obstime, const kvalobs::ObsDataElement &data)> func,
                        const std::list<long> &stationids, const std::list<long> &types, const boost::posix_time::ptime &from, const boost::posix_time::ptime &to, bool onlyKvData) {
  try {
    ConPtr conData = getConnection();
    ConPtr conTxtData = !onlyKvData ? getConnection() : ConPtr(nullptr);


    auto dataRes = query<kvData>(conData.get(), dataQuery(stationids, types, from, to) + " order by stationid, obstime, typeid");
    auto txtDataRes = query<kvTextData>(conTxtData.get(), textDataQuery(stationids, 0, from, to) + " order by stationid, obstime, typeid");

    list<kvalobs::kvData> retKvData;
    list<kvalobs::kvTextData> retKvTextData;

    while (dataRes.hasNext() || !retKvData.empty()) {
      if (retKvData.empty())
        retKvData = dataRes.next();

      if (retKvTextData.empty() && txtDataRes.hasNext())
        retKvTextData = txtDataRes.next();

      int cmp = compare(retKvTextData, retKvData);

      if (cmp == 0) {
        callback(func, make_tuple(retKvData, retKvTextData));
        retKvData.clear();
        retKvTextData.clear();
      } else if (cmp < 0) {
        callback(func, make_tuple(list<kvData>(), retKvTextData));
        retKvTextData.clear();
      } else if (cmp > 0) {
        callback(func, make_tuple(retKvData, list<kvTextData>()));
        retKvData.clear();
      }
    }

    if (!retKvTextData.empty())
      callback(func, make_tuple(list<kvData>(), retKvTextData));

    while (txtDataRes.hasNext()) {
      retKvTextData = txtDataRes.next();
      callback(func, make_tuple(list<kvData>(), retKvTextData));
    }
    return true;
  } catch (const std::exception &ex) {
    std::ostringstream err;
    err << "DbQuery::getKvData: stations:";
    for (auto id : stationids)
      err << " " << id;

    err << ", from " << from << " - ";

    if (to.is_special())
      err << "'+infinity'";
    else
      err << to;

    err << ", onlyKvData: " << (onlyKvData ? "true" : "false") << ".\nReason: " << ex.what();
    LOGERROR(err.str());
    return false;
  }
}

void DbQuery::getKvData(kvalobs::KvObsDataMap *data, std::list<long> stationids, std::list<long> typeIDs, const boost::posix_time::ptime &from,
                        const boost::posix_time::ptime &to, bool onlyKvData) {
  ConPtr conData = getConnection();

  data->clear();

  //Remove duplicates
  stationids = unique(stationids);
  typeIDs = unique(typeIDs);

  auto dataRes = query<kvData>(conData.get(), dataQuery(stationids, typeIDs, from, to) + " order by stationid, obstime, typeid");

  while (dataRes.hasNext()) {
    list<kvData> res = dataRes.next();
    if (!res.empty())
      data->add(res.front().stationID(), res.front().typeID(), res.front().obstime(), std::move(res));
  }

  if (onlyKvData)
    return;

  auto txtDataRes = query<kvTextData>(conData.get(), textDataQuery(stationids, typeIDs, from, to) + " order by stationid, obstime, typeid");
  while (txtDataRes.hasNext()) {
    list<kvTextData> res = txtDataRes.next();
    if (!res.empty())
      data->add(res.front().stationID(), res.front().typeID(), res.front().obstime(), std::move(res));
  }
}

void DbQuery::getKvData_(dnmi::db::Connection *con, std::list<kvalobs::kvData> *data, long stationid, long typeId, const boost::posix_time::ptime &from,
                         const boost::posix_time::ptime &to, bool sorted) {
  getKvData_(con, data, { stationid }, typeId, from, to, sorted);
}

void DbQuery::getKvTextData_(dnmi::db::Connection *con, std::list<kvalobs::kvTextData> *data, long stationid, long typeId, const boost::posix_time::ptime &from,
                             const boost::posix_time::ptime &to, bool sorted) {
  getKvTextData_(con, data, { stationid }, typeId, from, to, sorted);
}

void DbQuery::getKvData_(dnmi::db::Connection *con, std::list<kvalobs::kvData> *data, const std::list<long> &stationids, long typeID,
                         const boost::posix_time::ptime &from, const boost::posix_time::ptime &to, bool sorted) {
  data->clear();
  std::ostringstream q;
  q << dataQuery(stationids, typeID, from, to);

  if (sorted)
    q << " order by stationid,  obstime, typeid";

  query(con, data, q.str());

}
void DbQuery::getKvTextData_(dnmi::db::Connection *con, std::list<kvalobs::kvTextData> *data, const std::list<long> &stationids, long typeID,
                             const boost::posix_time::ptime &from, const boost::posix_time::ptime &to, bool sorted) {
  data->clear();
  std::ostringstream q;
  q << textDataQuery(stationids, typeID, from, to);

  if (sorted)
    q << " order by obstime, typeid";

  query(con, data, q.str());
}

DbQuery::ConPtr DbQuery::getConnection() {
  ConPtr con = createConnection_();
  //LOGDEBUG("DbQuery: getConnection()");
  if (!con)
    throw std::logic_error("Could NOT create a connection to the database.");

  return con;
}

void DbQuery::callback(std::function<void(long stationid, long typeID, const boost::posix_time::ptime &obstime, const kvalobs::ObsDataElement &data)> func,
                       const kvalobs::ObsDataElement &data) {
  if (!std::get<0>(data).empty()) {
    const list<kvData> &d = std::get<0>(data);
    func(d.front().stationID(), d.front().typeID(), d.front().obstime(), data);
  } else if (!std::get<1>(data).empty()) {
    const list<kvTextData> &d = std::get<1>(data);
    func(d.front().stationID(), d.front().typeID(), d.front().obstime(), data);
  }
}

}  // namespace sql
}  // namespace kvalobs

