/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: bufr.h,v 1.12.2.5 2007/09/27 09:02:18 paule Exp $

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

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <test/ReadDataFile.h>
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <Data.h>

using namespace std;

bool
readDataFile( const std::string &filename, DataEntryList &data, const boost::posix_time::ptime &fromtime )
{
	string file=string(TESTDATADIR) + "/" + filename;
	ifstream fin;
	string line;
	boost::posix_time::ptime obstime;

	data.clear();

	fin.open( file.c_str() );

	if( ! fin.is_open() )
		return false;

	while( getline( fin, line ) ) {

		string::size_type i = line.find( "#@#"); //Comment

		if( i != string::npos )
			line.erase( i );

		boost::trim(line);

		if( line.empty() )
			continue;


		std::vector<std::string> dataValues;
		boost::split(dataValues, line, boost::is_any_of("|"));

		if( dataValues.size() != 12 ) {
		   cerr << "readDataFile: to few elements ("<< dataValues.size() << ") expecting 12\n";
		   continue;
		}


		obstime = boost::posix_time::time_from_string( dataValues[1] );

		if( ! fromtime.is_not_a_date_time() && obstime > fromtime )
			continue;

		Data d( atoi( dataValues[0].c_str() ), obstime,
				dataValues[2], atoi( dataValues[3].c_str() ) ,
				atoi( dataValues[5].c_str() ), atoi( dataValues[6].c_str() ),
				atoi( dataValues[7].c_str() ), dataValues[9], dataValues[10] );

		//cerr << "readDataFile: insert( " << d << ")\n";
 		data.insert( d );
	}

//	for( DataEntryList::CITDataEntryList itd=data.begin(); itd!=data.end(); ++itd ) {
//		std::list<int> types = itd->getTypes();
//
//		for( std::list<int>::iterator tit=types.begin(); tit!=types.end(); ++tit ) {
//			DataListEntry::TDataList dl = itd->getTypeId( *tit );
//
//			for( DataListEntry::CITDataList dit=dl.begin(); dit!=dl.end(); ++dit )
//				cerr << *dit << endl;
//		}
// 	}

	return true;
}

bool
loadBufrDataFromFile( const std::string &filename,
					   StationInfoPtr      info,
					   DataElementList       &sd,
					   kvdatacheck::Validate &validate ,
					   const boost::posix_time::ptime &fromtime )
{
	DataEntryList rawdata;

	sd.clear();

	if( !info ) {
		cerr << "loadBufrDataFromFile: StationInfoPtr == 0 " << endl;
		return false;
	}

	if( ! readDataFile( filename, rawdata, fromtime ) ) {
		cerr << "Failed to read datafile <" << filename << ">." << endl;
		return false;
	}

	loadBufrData( rawdata, sd, info, validate );
//	cerr << "---- START ---\n";
//	for( CIDataElementList it=sd.begin(); it != sd.end(); ++it ) {
//	   cerr << it->time() << " RR_24: " << it->RR_24 << " EM: " << it->EM
//	        <<" SA: " << it->SA << " SS_24: " << it->SS_24 << endl;
//	}
//	cerr << "---- END ---\n";

  //cerr << sd << endl;
	return true;
}

