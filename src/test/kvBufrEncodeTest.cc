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

TEST_F( BufrEncodeTest, RR_from_RRRtr_AND_RR1 )
{
   using namespace miutil;
   int wmono=1492;
   DataElementList allData;
   DataElementList data;
   miTime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_18700_precip.dat", stInfo, allData, validData );
   dt=miTime("2010-10-21 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   EXPECT_FLOAT_EQ( -24, bufr.precip24.hTr );
   EXPECT_FLOAT_EQ( 0, bufr.precip24.RR );
   EXPECT_FLOAT_EQ( -12, bufr.precipRegional.hTr );
   EXPECT_FLOAT_EQ( 0, bufr.precipRegional.RR );
   EXPECT_FLOAT_EQ( -1, bufr.precipNational.hTr );
   EXPECT_FLOAT_EQ(  0, bufr.precipRegional.RR );
}

TEST_F( BufrEncodeTest, TGN )
{
   using namespace miutil;
   int wmono=1027;
   DataElementList allData;
   DataElementList data;
   miTime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_90400_tgn.dat", stInfo, allData, validData );
   dt=miTime("2010-10-27 06:00:00");
   data=allData.subData( dt );

   //Minimum and maximum temeratire at 6 o'clock

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   EXPECT_FLOAT_EQ( 272.35, bufr.TGN_12 );
}


TEST_F( BufrEncodeTest, MaxAndMinTemperature )
{
   using namespace miutil;
   int wmono=1002;
   DataElementList allData;
   DataElementList data;
   miTime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99927_temp.dat", stInfo, allData, validData );

   /******
    * Minimum and maximum temperature at 6 o'clock
    ******/
   dt=miTime("2010-10-27 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   //Maximum temperature.
   EXPECT_FLOAT_EQ( 269.05, bufr.TAX_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );

   //Minimum temperature.
   EXPECT_FLOAT_EQ( 266.75, bufr.TAN_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );

   /*******
    * Minimum and maximum temperature at 18 o'clock
    *******/
   dt=miTime("2010-10-26 18:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   //Maximum temperature.
   EXPECT_FLOAT_EQ( 267.25, bufr.TAX_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );

   //Minimum temperature.
   EXPECT_FLOAT_EQ( 265.85, bufr.TAN_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );

   /*******
    * Hourly min/max except 06 and 18 o'clock.
    *******/
   dt=miTime("2010-10-27 05:00:00");
   data=allData.subData( dt );

   //Minimum and maximum temperature at 6 o'clock

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   //Maximum temperature.
   EXPECT_FLOAT_EQ( 267.55, bufr.TAX_N );
   EXPECT_FLOAT_EQ( -1, bufr.tTAX_N  );

   //Minimum temperature.
   EXPECT_FLOAT_EQ( 267.15, bufr.TAN_N );
   EXPECT_FLOAT_EQ( -1, bufr.tTAX_N  );


   /*******
    * Min/max temperature at 06 and 18 o'clock, but
    * the values cant be computed.
    *******/
   dt=miTime("2010-10-28 06:00:00");
   data=allData.subData( dt );

   //Minimum and maximum temperature at 6 o'clock
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   //Maximum temperature.
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.TAX_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );

   //Minimum temperature.
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.TAN_N );
   EXPECT_FLOAT_EQ( -12, bufr.tTAX_N  );
}



TEST_F( BufrEncodeTest, RR_from_RA )
{
   using namespace miutil;
   int wmono=1493;
   DataElementList allData;
   DataElementList data;
   miTime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data-18700-RA.dat", stInfo, allData, validData );
   dt=miTime("2010-06-22 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< wmono;
   EXPECT_FLOAT_EQ( -12, bufr.precipRegional.hTr );
   EXPECT_FLOAT_EQ( 2.0, bufr.precipRegional.RR );

   EXPECT_FLOAT_EQ( 5.0, bufr.precip24.RR );
}



TEST_F( BufrEncodeTest, RR24_for_RRRtr )
{
   DataElementList data;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( 1389 );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << 1389;

   loadBufrDataFromFile( "data_7010-1.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.3, bufr.precip24.RR ) << "EXPECTED: precip24.RR: 0.3" << " Got: " << bufr.precip24.RR;

   //No precipitation.
   loadBufrDataFromFile( "data_7010-2.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.precip24.RR ) << "EXPECTED: precip24.RR: 0.0"  << " Got: " << bufr.precip24.RR;

   //Trace of precipitation
   loadBufrDataFromFile( "data_7010-3.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( -0.1, bufr.precip24.RR ) << "EXPECTED (1): precip24.RR: -0.1"  << " Got: " << bufr.precip24.RR;

   loadBufrDataFromFile( "data_7010-4.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( -0.1, bufr.precip24.RR ) << "EXPECTED (2): precip24.RR: -0.1"  << " Got: " << bufr.precip24.RR;


   loadBufrDataFromFile( "data_7010-5.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.1, bufr.precip24.RR ) << "EXPECTED: precip24.RR: 0.1"  << " Got: " << bufr.precip24.RR;
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
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-2.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-3.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr  ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-4.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-5.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-6.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-7.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-8.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;

   loadBufrDataFromFile( "data_TzFxFx-9.dat", stInfo, data, validData );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
}

TEST_F( BufrEncodeTest, encode_FgFx )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1006;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99735.dat", stInfo, allData, validData );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, allData, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( 11.6, bufr.FgMax.ff );
   EXPECT_FLOAT_EQ( 9.9, bufr.FxMax.ff );

   miutil::miTime dt=miutil::miTime("2010-10-21 07:00:00");
   DataElementList data=allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   EXPECT_FLOAT_EQ( 8.3, bufr.FxMax.ff );
   EXPECT_FLOAT_EQ( -60, bufr.FxMax.t );
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
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 21.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-22 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-23 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-24 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 35.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-25 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-26 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 0.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-27 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 0.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=miTime("2010-02-28 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 348.0, bufr.DD ) << "DD: Failed time: " << dt;
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

   EXPECT_TRUE( data.size() != 0 );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1492;

   BufrEncoder encoder( stInfo, true );
   try {
      encoder.encodeBufr( bufr, 0 );
   }
   catch ( BufrEncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( encoder.saveToFile( ".", true ) );
}

TEST_F( BufrEncodeTest, encode_bufr2 )
{
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1003;
   miutil::miTime dt;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99950.dat", stInfo, allData, validData );
   dt=miutil::miTime("2010-10-28 17:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.size() != 0 );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );

   BufrEncoder encoder( stInfo, true );

   try {
      encoder.encodeBufr( bufr, 0 );
   }
   catch ( BufrEncodeException &ex ) {
      FAIL()<< "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( encoder.saveToFile( ".", true ) );
}


TEST_F( BufrEncodeTest, writeTo )
{
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1492;
   stInfo = findWmoNo( wmono );
   ostringstream ost;
   string sData;

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data-18700-1.dat", stInfo, data, validData );

   EXPECT_TRUE( data.size() != 0 ) << "It is expected that the datalist is empty, but the size is: " << data.size();

   data.writeTo( ost );

   //cerr << " *****" << endl <<"[" << ost.str() << "]" << endl << "*****" << endl;
}

TEST_F( BufrEncodeTest, CloudsAndVV )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrData bufr;
   DataElementList data;
   miutil::miTime dt;
   //kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1385;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_4780_cloud.dat", stInfo, allData, validData );

   dt=miutil::miTime("2010-10-28 06:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( 75, bufr.N );
   EXPECT_FLOAT_EQ( 300, bufr.HL );


   dt=miutil::miTime("2010-10-28 08:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.N );
   EXPECT_FLOAT_EQ( 3610, bufr.HL );

   /*
   miutil::miTime dt=miutil::miTime("2010-10-28 06:00:00");
   DataElementList data=allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   EXPECT_FLOAT_EQ( 8.3, bufr.FxMax.ff );
   EXPECT_FLOAT_EQ( -60, bufr.FxMax.t );
   */


}

int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
