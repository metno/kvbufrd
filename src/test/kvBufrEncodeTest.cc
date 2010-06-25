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

#include <float.h>
#include <limits.h>
#include <string>
#include <fstream>
#include <puTools/miTime.h>
#include <miutil/cmprspace.h>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include "bufr.h"
#include "kvBufrEncodeTestConf.h"
#include "StationInfoParse.h"
#include <sstream>
#include "ReadDataFile.h"
#include <gtest/gtest.h>
#include <encodebufr.h>

using namespace std;

namespace {
}


class BufrEncodeTest : public testing::Test
{

protected:
	Bufr bufrEncoder;
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
		//istringstream iconf(testconf);
		std::string conffile(string(TESTDATADIR) + "/kvBufrEncodeTest.conf" );
		ifstream iconf( conffile.c_str() );
		//Turn off almost all logging.
		milog::Logger::logger().logLevel( milog::ERROR );

		ASSERT_TRUE( iconf.is_open() ) << "Cant open conf file <" << conffile << ">.";

		bufrEncoder.setTest( true );
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



TEST_F( BufrEncodeTest, RR24_for_RRRtr )
{
   DataElementList data;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( 1389 );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << 1389;

   loadBufrDataFromFile( "data_7010-1.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.3, bufr.precip24.RR );
   //EXPECT_EQ( bufr, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70003 555 41///=") << "Generated bufr 1: " << bufr;

   //No precipitation.
   loadBufrDataFromFile( "data_7010-2.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.precip24.RR );
   //EXPECT_EQ( bufr, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 70000 555 40///=")<< "Generated bufr 2: " << bufr;

   //Trace of precipitation
   loadBufrDataFromFile( "data_7010-3.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( -0.1, bufr.precip24.RR );
   //EXPECT_EQ( bufr, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 79999 555 40///=")<< "Generated bufr 3: " << bufr;

   loadBufrDataFromFile( "data_7010-4.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( -0.1, bufr.precip24.RR );
   //EXPECT_EQ( bufr, "AAXX 23061 01389 16/// ///// 1//// 2//// 69902 333 79999 555 40///=")<< "Generated bufr 4: " << bufr;


   loadBufrDataFromFile( "data_7010-5.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.1, bufr.precip24.RR );
   //EXPECT_EQ( bufr, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70001 555 41///=")<< "Generated bufr 5: " << bufr;
}


TEST_F( BufrEncodeTest, encode_TzFxFx )
{
	DataElementList data;
	StationInfoPtr stInfo;
	BufrData bufr;
	kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
	int wmono=1001;
	stInfo = findWmoNo( wmono );


	ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

	loadBufrDataFromFile( "data_TzFxFx-1.dat", stInfo, data, validData );
	EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
	//EXPECT_EQ( bufr, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/003 4////=") << "Generated bufr 1: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-2.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/003 4////=") << "Generated bufr 2: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-3.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/103 4////=") << "Generated bufr 3: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-4.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22151 01001 46/// ///// 1//// 2//// 555 0/304 4////=") << "Generated bufr 4: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-5.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/405 4////=") << "Generated bufr 5: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-6.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/404 4////=") << "Generated bufr 6: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-7.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/405 4////=") << "Generated bufr 7: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-8.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/008 4////=") << "Generated bufr 8: " << bufr;

    loadBufrDataFromFile( "data_TzFxFx-9.dat", stInfo, data, validData );
    EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
    //EXPECT_EQ( bufr, "AAXX 22121 01001 16/// ///// 1//// 2//// 6//// 555 0/005 4////=") << "Generated bufr 9: " << bufr;
}

TEST_F( BufrEncodeTest, encode_nddff )
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr stInfo;
   BufrData bufr;
   miTime dt;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1001;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_nddff.dat", stInfo, allData, validData );

   dt=miTime("2010-02-21 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 21.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 21061 01001 16/// ///21 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-22 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 22061 01001 16/// /3601 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-23 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 23061 01001 16/// /3601 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-24 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 35.0, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 24061 01001 16/// /0401 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-25 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 25061 01001 16/// ///01 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-26 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 0.0, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 26061 01001 16/// /0000 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-27 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 0.0, bufr.DD ) << "DD: Failed time: " << dt;
   //EXPECT_EQ( bufr, "AAXX 27061 01001 16/// /0000 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;

   dt=miTime("2010-02-28 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 348.0, bufr.DD ) << "DD: Failed time: " << dt;

   //EXPECT_EQ( bufr, "AAXX 28061 01001 16/// /35// 1//// 2//// 6//// 333 7//// 555 4////=") << "Generated bufr 1: " << bufr;
}


TEST_F( BufrEncodeTest, encode_noData )
{
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1384;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_4780-1.dat", stInfo, data, validData );

   EXPECT_TRUE( data.size() == 0 ) << "It is expected that the datalist is empty, but the size is: " << data.size();
}

TEST_F( BufrEncodeTest, encode_bufr )
{
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1492;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data-18700-1.dat", stInfo, data, validData );

   EXPECT_TRUE( data.size() != 0 ) << "It is expected that the datalist is empty, but the size is: " << data.size();
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr, false ) ) << "FAILED: Cant generate bufr for "<< 1492;

   BufrEncoder encoder( stInfo );
   try {
      encoder.encodeBufr( bufr );
   }
   catch ( BufrEncodeException &ex ) {
      cerr << "EXCEPTION: " << ex.what() << endl;
   }
   catch( ... ) {
      cerr << "EXCEPTION: Unknown."<< endl;
   }
   //ASSERT_NO_THROW( encoder.encodeBufr( bufr ) );
   ASSERT_NO_THROW( encoder.saveToFile( ".", true ) );

}


int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
