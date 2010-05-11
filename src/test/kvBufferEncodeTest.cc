/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: buffer.h,v 1.12.2.5 2007/09/27 09:02:18 paule Exp $

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

#include <float.h>
#include <limits.h>
#include <string>
#include <puTools/miTime.h>
#include <miutil/cmprspace.h>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include "buffer.h"
#include "kvBufferEncodeTestConf.h"
#include "StationInfoParse.h"
#include <sstream>
#include "ReadDataFile.h"
#include <gtest/gtest.h>

using namespace std;

namespace {
}


class BufferEncodeTest : public testing::Test
{

protected:
	Buffer bufferEncoder;
	std::list<StationInfoPtr> stationList;

	StationInfoPtr findWmoNo( int wmono ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
			if( (*it)->wmono() == wmono )
				return *it;
		}

		return StationInfoPtr();
	}

	///Called before each test case.
	virtual void SetUp() {
		using namespace miutil::conf;
		ConfParser confParser;
		istringstream iconf(testconf);

		//Turn off almost all logging.
		milog::Logger::logger().logLevel( milog::ERROR );

		bufferEncoder.setTest( true );
		//cerr << "[" << endl << testconf << endl << "]" << endl;

		ConfSection *conf = confParser.parse( iconf );

		ASSERT_TRUE( conf ) << "Cant parse the configuration settings.";

		StationInfoParse stationParser;

		ASSERT_TRUE( stationParser.parse( conf, stationList ) ) << "Cant parse the station information.";
	}

	///Called after each test case.
	virtual void TearDown() {

	}


};



TEST_F( BufferEncodeTest, RR24_for_RRRtr )
{
	BufferDataList data;
	StationInfoPtr stInfo;
	string buffer;
	kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
	stInfo = findWmoNo( 1389 );

	ASSERT_TRUE( stInfo ) << "No station information for wmono " << 1389;

	loadBufferDataFromFile( "data_7010-1.dat", stInfo, data, validData );
	EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
	miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70003 555 41///=") << "Generated buffer 1: " << buffer;

    loadBufferDataFromFile( "data_7010-2.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 70000 555 40///=")<< "Generated buffer 2: " << buffer;

    loadBufferDataFromFile( "data_7010-3.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 79999 555 40///=")<< "Generated buffer 3: " << buffer;

    loadBufferDataFromFile( "data_7010-4.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 23061 01389 16/// ///// 1//// 2//// 69902 333 79999 555 40///=")<< "Generated buffer 4: " << buffer;

    loadBufferDataFromFile( "data_7010-5.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70001 555 41///=")<< "Generated buffer 5: " << buffer;
}


TEST_F( BufferEncodeTest, encode_TzFxFx )
{
	BufferDataList data;
	StationInfoPtr stInfo;
	string buffer;
	kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
	int wmono=1001;
	stInfo = findWmoNo( wmono );


	ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

	loadBufferDataFromFile( "data_TzFxFx-1.dat", stInfo, data, validData );
	EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
	miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/003 4////=") << "Generated buffer 1: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-2.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/003 4////=") << "Generated buffer 2: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-3.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/103 4////=") << "Generated buffer 3: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-4.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/304 4////=") << "Generated buffer 4: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-5.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/405 4////=") << "Generated buffer 5: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-6.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/404 4////=") << "Generated buffer 6: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-7.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/405 4////=") << "Generated buffer 7: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-8.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/008 4////=") << "Generated buffer 8: " << buffer;

    loadBufferDataFromFile( "data_TzFxFx-9.dat", stInfo, data, validData );
    EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
    miutil::cmprspace( buffer, true );
    EXPECT_EQ( buffer, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/005 4////=") << "Generated buffer 9: " << buffer;
}

TEST_F( BufferEncodeTest, encode_nddff )
{
   using namespace miutil;
   BufferDataList allData;
   BufferDataList data;
   StationInfoPtr stInfo;
   string buffer;
   miTime dt;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1001;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufferDataFromFile( "data_nddff.dat", stInfo, allData, validData );

   dt=miTime("2010-02-21 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 21061 01001 16/// ///21 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-22 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 22061 01001 16/// /3601 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-23 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 23061 01001 16/// /3601 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-24 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 24061 01001 16/// /0401 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-25 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 25061 01001 16/// ///01 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-26 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 26061 01001 16/// /0000 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-27 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 27061 01001 16/// /0000 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;

   dt=miTime("2010-02-28 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufferEncoder.doBuffer( stInfo, data, buffer, false ) ) << "FAILED: Cant generate buffer for "<< 1389;
   miutil::cmprspace( buffer, true );
   EXPECT_EQ( buffer, "AAXX 28061 01001 16/// /35// 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated buffer 1: " << buffer;
}




int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
