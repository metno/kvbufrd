/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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
#ifndef __kvbufr_Data_h__
#define __kvbufr_Data_h__

#include <set>
#include <iostream>
#include <string>
#include "boost/date_time/posix_time/ptime.hpp"
#include "kvalobs/kvData.h"
#include "kvalobs/kvDataFlag.h"
#include "kvalobs/kvDbBase.h"
#include "kvDbGateProxyThread.h"


class Data : public kvalobs::kvDbBase {
private:
	int              stationid_;
	boost::posix_time::ptime obstime_;
	boost::posix_time::ptime tbtime_;
	std::string      original_;
	std::string      corrected_;
	int              paramid_;
	int              typeid_;
	int              sensor_;
	int              level_;
	std::string      controlinfo_;
	std::string      useinfo_;
	bool             useCorrected_;

	void createSortIndex();

public:
	Data() {clean();}
	Data(const kvalobs::kvData &data){ set(data);}
	Data(const dnmi::db::DRow &r){set(r);}
	Data(int                      pos,
			const boost::posix_time::ptime &obt,
			const std::string       &org,
			const std::string       &cor,
			int                      par,
			int                      typ,
			int                      sen,
			int                      lvl,
			const std::string &controlinfo="",
			const std::string &useinfo="" )
	{ set(pos, obt, org, cor, par, typ, sen, lvl, controlinfo, useinfo);}

	bool set(int                      pos,
			const boost::posix_time::ptime &obt,
			const std::string       &org,
			const std::string       &cor,
			int                      par,
			int                      typ,
			int                      sen,
			int                      lvl,
			const std::string &controlinfo="",
			const std::string &useinfo="");

	bool set(const dnmi::db::DRow&);
	bool set(const kvalobs::kvData &data);

	void clean();

	const char* tableName() const {return "data";}

	std::string toSend()   const;
	std::string toUpdate() const;
	std::string uniqueKey() const;

	int              stationID()   const { return stationid_;  }
	boost::posix_time::ptime obstime()     const { return obstime_;    }
	boost::posix_time::ptime tbtime()      const { return tbtime_;    }

		
	bool             useCorrected()const { return useCorrected_; }
	void             useCorrected( bool v) { useCorrected_=v;}
	std::string      original()    const;
	std::string      corrected()   const { return corrected_;  }
	int              paramID()     const { return paramid_;    }
	int              typeID()      const { return typeid_;     }
	int              sensor()      const { return sensor_;     }
	int              level()       const { return level_;      }
	kvalobs::kvControlInfo controlinfo()const;
	kvalobs::kvUseInfo useinfo()const;

	friend std::ostream& operator<<( std::ostream& ost,
									 const Data& data );
};


class DataInsertCommand: public kvalobs::KvDbGateDoExecCommand {
	public:
		DataInsertCommand(const std::list<Data> &dl, const std::string &logid="");
		std::string getError()const;

		virtual std::string name()const override {
    	return "DataInsertCommand";
   	}


		virtual bool doExec( dnmi::db::Connection *con) override;
	private:
		std::list<Data> dl_;
		std::string logid_;
		std::ostringstream err_;
};


std::list<Data> kvDataToData( const std::list<kvalobs::kvData> &data);

struct DataKey {
   int              stationid_;
   int              typeid_;
   int              paramid_;
   int              sensor_;
   int              level_;
   boost::posix_time::ptime obstime_;

   DataKey( const kvalobs::kvData &data );
   bool operator<(const DataKey &dk )const;
};

struct DataKeySet
   : public std::set<DataKey>
{
      ///@return true if the key is added and false if the key
      ///already is in the list.
      bool add( const DataKey &key );
};

std::ostream& operator<<( std::ostream& ost,
						  const Data& data );

std::ostream& operator<<( std::ostream& ost,
						  const std::list<Data>& dl );

#endif
