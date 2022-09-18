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
#include <math.h>
#include <iomanip>
#include <locale>
#include <float.h>
#include <limits.h>
#include <string>
#include <fstream>
#include <list>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <gtest/gtest.h>
#include <puTools/miTime.h>
#include <miutil/cmprspace.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include "bufr.h"
#include "kvBufrEncodeTestConf.h"
#include "StationInfoParse.h"
#include "ReadDataFile.h"
//#include "encodebufr.h"
#include "bufr/EncodeBufrBase.h"
#include "bufr/BUFRparam.h"
#include "bufr/EncodeBufrManager.h"
#include "bufr/BufrHelper.h"
#include "bufr/AreaDesignator.h"

using namespace std;
namespace b=boost;

namespace {
}


class BufrEncodeTest : public testing::Test
{

protected:
	Bufr bufrEncoder;
	BufrParamValidaterPtr validater;
	std::list<StationInfoPtr> stationList;

	StationInfoPtr findWmoNo( int wmono ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
			if( (*it)->wmono() == wmono )
				return *it;
		}

		return StationInfoPtr();
	}

   StationInfoPtr findWigosId( const std::string &wsi ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
			if( (*it)->wigosId() == wsi )
				return *it;
		}

		return StationInfoPtr();
	}


	StationInfoPtr findStationId( int stationid ) {
	      for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
	         if( (*it)->stationID() == stationid )
	            return *it;
	      }

	      return StationInfoPtr();
	   }

	StationInfoPtr findCallsign( const std::string &callsign ) {
	      for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
	         if( (*it)->callsign() == callsign )
	            return *it;
	      }

	      return StationInfoPtr();
	   }

	///Called before each test case.
	virtual void SetUp() {
		using namespace miutil::conf;
		ConfParser confParser;
		string bufr_tables( BUFRTBLDIR );

		 if( ! bufr_tables.empty() && *bufr_tables.rbegin() != '/' )
			 bufr_tables += '/';

		 //istringstream iconf(testconf);
		setenv("BUFR_TABLES", bufr_tables.c_str(), 1 );
		string conffile(string(TESTDATADIR) + "/kvBufrEncodeTest.conf" );
		string btabl(string(BUFRTBLDIR)+"/B0000000000000033000.TXT");
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

		validater = BufrParamValidater::loadTable( btabl );
      ASSERT_TRUE( validater.get() ) << "Cant load BUFR B table: " << btabl;
      EncodeBufrManager::paramValidater=validater;
	}

	///Called after each test case.
	virtual void TearDown() {

	}


};


namespace
{
	boost::posix_time::ptime getTime(const std::string & formatted)
	{
		return boost::posix_time::time_from_string(formatted);
	}
}


TEST_F(BufrEncodeTest, WIGOS_encode_with_wmono) 
{
using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   boost::posix_time::ptime dt;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   string wsi="0-20000-0-89504";
   stInfo = findWigosId( wsi );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wigos id '" << wsi << "'";

   loadBufrDataFromFile( "data-99990_501_316_20220917T060000.dat", stInfo, allData, validData );
   dt=getTime("2022-09-17 06:00:00");

   cerr << allData << endl;

   cerr << "allData #" << allData.size() << "\n";
   data=allData.subData( dt );
   
   EXPECT_TRUE( data.firstTime() == dt ) << "Expecting obstime: "<< dt << " got " <<data.firstTime();
   EXPECT_TRUE( data.size() != 0 ) << "Expcting at least one hour of data.";
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );
   auto bufrHelperPtr=bufrEncoder.encodeBufr(stInfo, data);
   EXPECT_TRUE(bufrHelperPtr != nullptr) << "Failed to encode BUFR.";
   


   bufrHelperPtr->setSequenceNumber(0);
   bufrHelperPtr->saveToFile(".", 0);

   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest(true); 
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900006);

   try {
      encoder.encode( templateList, bufrHelper );
      cout << bufrHelper.getLog() << endl;
      cout << " ------------------------ \n";
      bufrHelper.printTestValues(cout);

      //Check that wmo numer is set
      ASSERT_TRUE( bufrHelper.getTestValue("001001").getI()==89 &&  bufrHelper.getTestValue("001002").getI()==504) << "Expect the wmo numer to be set to 89504";

      //Check that the WIGOS id is set.

      ASSERT_TRUE(bufrHelper.getTestValue("001125").getI()==0 &&  bufrHelper.getTestValue("001126").getI() == 20000 
         && bufrHelper.getTestValue("001127").getI() == 0 && bufrHelper.getTestValue("001128").getS() == "89504" ) << "Expect WIGOS id '0-20000-0-89504'";
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }
}



TEST_F(BufrEncodeTest, WIGOS_encode_no_wmono) 
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   boost::posix_time::ptime dt;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   string wsi="0-578-0-92100";
   stInfo = findWigosId( wsi );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wigos id '" << wsi << "'";

   loadBufrDataFromFile( "data-92100_501_506_20220917T060000.dat", stInfo, allData, validData );
   dt=getTime("2022-09-17 06:00:00");

   cerr << allData << endl;

   cerr << "allData #" << allData.size() << "\n";
   data=allData.subData( dt );
   
   EXPECT_TRUE( data.firstTime() == dt ) << "Expecting obstime: "<< dt << " got " <<data.firstTime();
   EXPECT_TRUE( data.size() != 0 ) << "Expcting at least one hour of data.";
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );
   auto bufrHelperPtr=bufrEncoder.encodeBufr(stInfo, data);
   EXPECT_TRUE(bufrHelperPtr != nullptr) << "Failed to encode BUFR.";
   


   bufrHelperPtr->setSequenceNumber(0);
   bufrHelperPtr->saveToFile(".", 0);

   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest(true); 
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900006);

   try {
      encoder.encode( templateList, bufrHelper );
      cout << bufrHelper.getLog() << endl;
      cout << " ------------------------ \n";
      bufrHelper.printTestValues(cout);
      //Check that wmo numer is nor set
      ASSERT_TRUE(bufrHelper.getTestValue("001001").isMissing() &&  bufrHelper.getTestValue("001002").isMissing()) << "Expect the wmo numer to be missing";

      //Check that the WIGOS id is set.
      ASSERT_TRUE(bufrHelper.getTestValue("001125").getI()==0 &&  bufrHelper.getTestValue("001126").getI() == 578 
         && bufrHelper.getTestValue("001127").getI() == 0 && bufrHelper.getTestValue("001128").getS() == "92100" ) << "Expect WIGOS id '0-578-0-92100'";
      bufrHelper.printTestValues(cout);
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

//   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true, true ) );
}



TEST_F(BufrEncodeTest, WIGOS_ID) 
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   
   string wsi="0-20000-0-89504";
   stInfo = findWigosId( wsi );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wigos id '" << wsi << "'.";

   int identifierSeries, issuerOfIdentifier, issueNumber;
   string localIdentifier;
   stInfo->getWigosId(identifierSeries, issuerOfIdentifier, issueNumber,localIdentifier );

   cout << "identifierSeries: " << identifierSeries << " issuerOfIdentifier: " << issuerOfIdentifier << " issueNumber: " << issueNumber << " localIdentifier: '" << localIdentifier << "'\n";

   ASSERT_TRUE( identifierSeries==0 && issuerOfIdentifier==20000 && issueNumber==0 && localIdentifier=="89504" ) << "Failed to decode wigos id '" << wsi << "'";

}




TEST_F(BufrEncodeTest, UseCorrected) 
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   boost::posix_time::ptime dt;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1026;
   stInfo = findWmoNo( wmono   );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "90450_20211123T060000.dat", stInfo, allData, validData );
   dt=getTime("2021-11-23 06:00:00");

   cerr << allData << endl;

   cerr << "allData #" << allData.size() << "\n";
   data=allData.subData( dt );
   
   EXPECT_TRUE( data.firstTime() == dt ) << "Expecting obstime: "<< dt << " got " <<data.firstTime();
   EXPECT_TRUE( data.size() != 0 ) << "Expcting at least one hour of data.";
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );
   auto bufrHelperPtr=bufrEncoder.encodeBufr(stInfo, data);
   EXPECT_TRUE(bufrHelperPtr != nullptr) << "Failed to encode BUFR.";
   auto crc=bufrHelperPtr->computeCRC();
   cout << "crc '" << crc << "'" << endl;
   //FAIL();
   //EXPECT_EQ(2945486157, crc) << "Failed to generate crc.";

   bufrHelperPtr->setSequenceNumber(2);
   bufrHelperPtr->saveToFile(".", 0);

   BufrHelper bufrHelper( validater, stInfo, bufr );
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900000);

   try {
      encoder.encode( templateList, bufrHelper );
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true, true ) );
}

#if 0

TEST_F(BufrEncodeTest, BUOY_basic) 
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   boost::posix_time::ptime dt;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   string id="6301001";
   stInfo = findCallsign( id );

   ASSERT_TRUE( stInfo.get() ) << "No station information for callsign " << id;

   loadBufrDataFromFile( "76933_20211123T000000.dat", stInfo, allData, validData );
   dt=getTime("2021-11-23 06:00:00");

   cerr << allData << endl;

   cerr << "allData #" << allData.size() << "\n";
   data=allData.subData( dt );
   
   EXPECT_TRUE( data.firstTime() == dt ) << "Expecting obstime: "<< dt << " got " <<data.firstTime();
   EXPECT_TRUE( data.size() != 0 ) << "Expcting at least one hour of data.";
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );
   auto bufrHelperPtr=bufrEncoder.encodeBufr(stInfo, data);
   EXPECT_TRUE(bufrHelperPtr != nullptr) << "Failed to encode BUFR.";
   auto crc=bufrHelperPtr->computeCRC();
   cout << "crc '" << crc << "'" << endl;
   //FAIL();
   EXPECT_EQ(2945486157, crc) << "Failed to generate crc.";

   bufrHelperPtr->setSequenceNumber(2);
   bufrHelperPtr->saveToFile(".", 0);

   BufrHelper bufrHelper( validater, stInfo, bufr );
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900005);

   try {
      encoder.encode( templateList, bufrHelper );
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true, true ) );
}




TEST_F( BufrEncodeTest, GustFrom_FG_1_or_FG_010 )
{
   using namespace miutil;
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   boost::posix_time::ptime dt;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1495;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_18700_501_2019051404.dat", stInfo, allData, validData );
   dt=getTime("2019-05-14 06:00:00");
   data=allData.subData( dt );
   
   EXPECT_TRUE( data.firstTime() == dt ) << "Expecting obstime: "<< dt << " got " <<data.firstTime();
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );

   EXPECT_TRUE( data.size() != 0 );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< wmono;

   BufrHelper bufrHelper( validater, stInfo, bufr );
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900000);

   try {
      encoder.encode( templateList, bufrHelper );
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true ) );
}



/*
TEST_F( BufrEncodeTest, GustFrom_FG_1_or_FG_010 )
{
   using namespace miutil;
   int wmono=1495;
   DataElementList allData;
   DataElementList data;
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_18700_501_2019051404.dat", stInfo, allData, validData );
   dt=getTime("2019-05-14 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   cerr << "GustFrom_FG_1_or_FG_010: (" << bufrEncoder.getTestValue().getF("GUST") <<")\n";
   EXPECT_FLOAT_EQ( 1.2, bufr.FG_1 );
   ASSERT_FAIL(true);
   
}
*/


TEST_F( BufrEncodeTest, RR_from_RRRtr_AND_RR1 )
{
   using namespace miutil;
   int wmono=1492;
   DataElementList allData;
   DataElementList data;
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_18700_precip.dat", stInfo, allData, validData );
   dt=getTime("2010-10-21 06:00:00");
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

TEST_F( BufrEncodeTest, RR_FROM_RR1_negativ_RR1 )
{
   using namespace miutil;
   int wmono=1342;
   DataElementList allData;
   DataElementList data;
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_1342_342_20110314T09.dat", stInfo, allData, validData );
   dt=getTime("2011-03-14 09:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   EXPECT_FLOAT_EQ( -1, bufr.precipNational.hTr );
   EXPECT_TRUE( bufr.precipNational.RR >= 0 && bufr.precipNational.RR != FLT_MAX )
      << "RR: " << bufr.precipNational.RR;
}


TEST_F( BufrEncodeTest, TGN )
{
   using namespace miutil;
   int wmono=1027;
   DataElementList allData;
   DataElementList data;
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_90400_tgn.dat", stInfo, allData, validData );
   dt=getTime("2010-10-27 06:00:00");
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
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99927_temp.dat", stInfo, allData, validData );

   /******
    * Minimum and maximum temperature at 6 o'clock
    ******/
   dt=getTime("2010-10-27 06:00:00");
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
   dt=getTime("2010-10-26 18:00:00");
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
   dt=getTime("2010-10-27 05:00:00");
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
   dt=getTime("2010-10-28 06:00:00");
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
   boost::posix_time::ptime dt;
   StationInfoPtr stInfo;
   BufrData bufr;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;
   
   loadBufrDataFromFile( "data-18700-RA.dat", stInfo, allData, validData );
   dt=getTime("2010-06-22 06:00:00");
   
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

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << 1389;

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

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

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

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99735.dat", stInfo, allData, validData );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, allData, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( 11.6, bufr.FgMax.ff );
   EXPECT_FLOAT_EQ( 9.9, bufr.FxMax.ff );

   boost::posix_time::ptime dt=getTime("2010-10-21 07:00:00");
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
   boost::posix_time::ptime dt;
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1001;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_nddff.dat", stInfo, allData, validData );

   dt=getTime("2010-02-21 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 21.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-22 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-23 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 360.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-24 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 35.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-25 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 1.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-26 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.0, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 0.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-27 06:00:00");
   data = allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1389;
   EXPECT_FLOAT_EQ( 0.9, bufr.FF ) << "FF: Failed time: " << dt;
   EXPECT_FLOAT_EQ( 348.0, bufr.DD ) << "DD: Failed time: " << dt;

   dt=getTime("2010-02-28 06:00:00");
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


   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_4780-1.dat", stInfo, data, validData );

   EXPECT_TRUE( data.size() == 0 ) << "It is expected that the datalist is empty, but the size is: " << data.size();
}

TEST_F( BufrEncodeTest, encode_bufr )
{
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1492;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data-18700-1.dat", stInfo, data, validData );

   EXPECT_TRUE( data.size() != 0 );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< 1492;

   BufrHelper bufrHelper( validater, stInfo, bufr );
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900000);

   try {
      encoder.encode( templateList, bufrHelper );
   }
   catch ( EncodeException &ex ) {
      FAIL() << "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch ( const std::exception &ex ) {
       FAIL() << "EXCEPTION: " << ex.what();
   }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true ) );
}

TEST_F( BufrEncodeTest, encode_bufr2 )
{
   DataElementList allData;
   DataElementList data;
   StationInfoPtr  stInfo;
   BufrDataPtr bufr( new BufrData() );
   kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   int wmono=1003;
   boost::posix_time::ptime dt;
   stInfo = findWmoNo( wmono );


   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_99950.dat", stInfo, allData, validData );
   dt=getTime("2010-10-28 17:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( data.size() != 0 );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) );

   BufrHelper bufrHelper( validater, stInfo, bufr );
   EncodeBufrManager encoder;
   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900000);


   try {
      encoder.encode( templateList, bufrHelper );
   }
   catch ( EncodeException &ex ) {
      FAIL()<< "EXCEPTION (BufrEncodeException): " << ex.what();
   }
   catch( const std::exception &ex) {
         FAIL() << "EXCEPTION: Unknown: " << ex.what();
      }
   catch( ... ) {
      FAIL() << "EXCEPTION: Unknown.";
   }

   ASSERT_NO_THROW( bufrHelper.saveToFile( ".", true ) );
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

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

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
   boost::posix_time::ptime dt;
   //kvdatacheck::Validate validData( kvdatacheck::Validate::UseOnlyUseInfo );
   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
   int wmono=1385;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;

   loadBufrDataFromFile( "data_4780_cloud.dat", stInfo, allData, validData );

   dt=getTime("2010-10-28 06:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( 75, bufr.N );
   EXPECT_FLOAT_EQ( 300, bufr.HL );


   dt=getTime("2010-10-28 08:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) ) << "FAILED: Cant generate bufr for "<< 1006;
   EXPECT_FLOAT_EQ( FLT_MAX, bufr.N );
   EXPECT_FLOAT_EQ( 3610, bufr.HL );

   /*
   boost::posix_time::ptime dt=getTime("2010-10-28 06:00:00");
   DataElementList data=allData.subData( dt );
   EXPECT_TRUE( data.firstTime() == dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, bufr ) );
   EXPECT_FLOAT_EQ( 8.3, bufr.FxMax.ff );
   EXPECT_FLOAT_EQ( -60, bufr.FxMax.t );
   */


}


TEST_F( BufrEncodeTest, EncodeBufr307079 )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int wmono=1492;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;
   ASSERT_TRUE( loadBufrDataFromFile( "data_18700.dat", stInfo, allData, validData ) )
      << "Cant load data from filr: data_18700.dat";

   dt=getTime("2012-05-17 06:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< wmono;

   bufr->cloudExtra.add( BufrData::CloudDataExtra( 7, 3, 4, 1000) );
   bufr->EM=7;
   bufr->SA=23;
   bufr->TGN_12=8;
   bufr->TW=275;
   bufr->precip24.RR = -0.1;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );
   //b::assign::push_back( templateList )
   //   (301090)(302031)(302035)(302037)(302056)(302043);

   b::assign::push_back( templateList )(900000);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR301090" );

      fout.write( buf, len );
      fout.close();



   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   }
}

TEST_F( BufrEncodeTest, EncodeBufr307079_EmptyName )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int wmono=1492;
   stInfo = findWmoNo( wmono );

   stInfo->name("", false ); //Reset the name to an empty string
   ASSERT_TRUE( stInfo->name().empty() );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;
   ASSERT_TRUE( loadBufrDataFromFile( "data_18700.dat", stInfo, allData, validData ) )
      << "Cant load data from filr: data_18700.dat";

   dt=getTime("2012-05-17 06:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< wmono;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900000);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );

   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR301090_empty_name" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   }
}

TEST_F( BufrEncodeTest, EncodeBufrSVV )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );


   int stationid=17090;
   stInfo = findStationId( stationid );

   ASSERT_TRUE( stInfo.get() ) << "No station information for stationid " << stationid;
   ASSERT_TRUE( loadBufrDataFromFile( "svv_17090.dat", stInfo, allData, validData ) )
      << "Can't load data from file: svv_17090.dat";

   cerr << "allData: " << allData.size() << endl;
   dt=getTime("2012-05-30 06:00:00");
   data=allData.subData( dt );

   //cerr << "@@@@@@ dt: " << dt << " ft: " << data.firstTime()<<endl;
   EXPECT_TRUE( dt == data.firstTime() ) ;

   cerr << "data: " << data.size() << endl;
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< stationid;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900001);
   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR900001" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   }
}

TEST_F( BufrEncodeTest, MsgTimeTest )
{
   using namespace miutil;
   MsgTime t;
   MsgTime::Hours hours;
   int stationid=999991;
   boost::posix_time::ptime test;
   StationInfoPtr stInfo;
   stInfo = findStationId( stationid );

   ASSERT_TRUE( stInfo.get() ) << "No station information for stationid " << stationid;

   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 00:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 03:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 06:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 09:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 12:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 15:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 18:00:00" ) ));
   ASSERT_FALSE( stInfo->msgForTime( getTime("2012-06-07 21:00:00" ) ));
   ASSERT_TRUE( stInfo->msgForTime( getTime("2012-06-07 08:00:00" ) ));

   try {
      t.setMsgForTime( "0/6:0" );
      cout << t << endl;

      test = getTime("2012-06-07 00:00:00");
      EXPECT_TRUE( t.msgForTime( test ) );

      test = getTime("2012-06-07 00:01:00");
      EXPECT_FALSE( t.msgForTime( test ) );

      test = getTime("2012-06-07 04:00:00");
      EXPECT_FALSE( t.msgForTime( test ) );
   }
   catch( const std::exception &e ) {
      cout << "EXCEPTION: " << e.what() << endl;
   }

}




double
myRound( float val, double prec) {
   double y=val/prec;
   double q = round( y );
   return q*prec;
}

TEST_F( BufrEncodeTest, BufrValidaterTest )
{
   float rr24f=-0.1;

   BufrParamTypePtr pRR = validater->findParamDef(13023);
   ASSERT_TRUE( pRR.get() ) << "No BUFR param definition for paramid 0 13 023";

//   cerr << "rr24fr: " << setprecision(20) << myRound( rr24f, 0.1 ) << endl;
//   cerr << "rr24f : " << setprecision(20) << rr24f << endl;
//   cerr << "rr24  : " << setprecision(20) << rr24 << endl;

   ASSERT_TRUE( pRR->valid( rr24f ) );
}

TEST_F( BufrEncodeTest, EncodeBufr307079_18700 )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int wmono=1492;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;
   ASSERT_TRUE( loadBufrDataFromFile( "18700-20121023T06.dat", stInfo, allData, validData ) )
      << "Cant load data from file: 18700-20121023T06.dat";

   dt=getTime("2012-10-23 06:00:00");
   data=allData.subData( dt );
   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< wmono;



   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900000);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR301090" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   }
}

TEST_F( BufrEncodeTest, EncodeBufrBase_encodeRR)
{
    EXPECT_FLOAT_EQ( EncodeBufrBase::encodeRR( float(-1) ), float(0)  );
    EXPECT_FLOAT_EQ( EncodeBufrBase::encodeRR( float( 0 ) ), float(-0.1) );
    EXPECT_FLOAT_EQ( EncodeBufrBase::encodeRR( 0.1 ), 0.1 );
    EXPECT_FLOAT_EQ( EncodeBufrBase::encodeRR( 1.2 ), 1.2 );
    EXPECT_TRUE( EncodeBufrBase::encodeRR( FLT_MAX ) == FLT_MAX );
    EXPECT_TRUE( EncodeBufrBase::encodeRR( -0.5 ) == FLT_MAX );
    EXPECT_TRUE( EncodeBufrBase::encodeRR( -1.5 ) == FLT_MAX );
}



TEST_F( BufrEncodeTest, EncodeBufr90002)
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;



   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int stationid=73250;
   stInfo =  findStationId( stationid );

   ASSERT_TRUE( stInfo.get() ) << "No station information for id " << stationid;
   ASSERT_TRUE( loadBufrDataFromFile( "73250-302-20121024T06.dat", stInfo, allData, validData ) )
      << "Cant load data from file: 73250-302-20121024T06.dat";
   dt=getTime("2012-10-24 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< stationid;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900002);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR90002" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
      FAIL();
   }


   dt=getTime("2012-11-14 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< stationid;

   BufrHelper bufrHelper1( validater, stInfo, bufr );
   bufrHelper1.setTest( true );


   bufrHelper1.setObsTime( dt );
   bufrHelper1.setSequenceNumber( 0 );


     try {
         encoder.encode( templateList, bufrHelper1 );
         cerr << bufrHelper1.getLog() << endl;

        int len;
        const char *buf=bufrHelper1.getBufr( len );
        cerr << "#len: " << len << endl;
        ofstream fout( "BUFR90002_1" );

        fout.write( buf, len );
        fout.close();
     }
     catch ( const std::exception &ex ) {
        cerr << "<<<<< EXCEPTION LOG" << endl;
        cerr << bufrHelper.getLog() << endl;
        cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
        FAIL();
     }


}

TEST_F( BufrEncodeTest, EncodeBufr90003_SHIP)
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;
   string datafile("76991_KV_HARSTAD.dat");

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int stationid=76991;
   stInfo =  findCallsign( "LMXQ" );

   ASSERT_TRUE( stInfo.get() ) << "No station information for id " << stationid;
   ASSERT_TRUE( loadBufrDataFromFile( datafile, stInfo, allData, validData ) )
      << "Cant load data from file: '" << datafile << "'.";
   dt=getTime("2012-11-10 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< stationid;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900003);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR90003_LMXQ" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
      FAIL();
   }
}

TEST_F( BufrEncodeTest, EncodeBufr90003_Platforms)
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;
   string datafile("76920_EKOFISK.dat");
   string callsign("LF5U");

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   stInfo =  findCallsign( "LF5U" );

   ASSERT_TRUE( stInfo.get() ) << "No station information for callsign " << callsign;
   ASSERT_TRUE( loadBufrDataFromFile( datafile, stInfo, allData, validData ) )
      << "Cant load data from file: '" << datafile << "'.";
   dt=getTime("2012-11-10 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< callsign;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900003);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( "BUFR90003_LF5U" );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
      FAIL();
   }
}

TEST_F( BufrEncodeTest, EncodeBufr90003_FG)
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;
   string datafile("LF5U_20121115.dat");
   string callsign("LF5U");

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   stInfo =  findCallsign( callsign );

   ASSERT_TRUE( stInfo.get() ) << "No station information for callsign " << callsign;
   ASSERT_TRUE( loadBufrDataFromFile( datafile, stInfo, allData, validData ) )
      << "Cant load data from file: '" << datafile << "'.";
   dt=getTime("2012-11-15 06:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< callsign;

   EXPECT_FLOAT_EQ( bufr->FgMax.ff, 8.2304001);
   EXPECT_FLOAT_EQ( bufr->FgMax.dd, FLT_MAX );
   EXPECT_FLOAT_EQ( bufr->FgMax.t, -360 );
}




TEST_F( BufrEncodeTest, AreaDesignator)
{

	//Test northern hemisphere
	ASSERT_TRUE( computeAreaDesignator(10, 60) == "D" );
	ASSERT_TRUE( computeAreaDesignator(100, 60) == "C" );
	ASSERT_TRUE( computeAreaDesignator(-10, 60) == "A" );
	ASSERT_TRUE( computeAreaDesignator(-100, 60) == "B" );

	//Test southern hemisphere
	ASSERT_TRUE( computeAreaDesignator(10, -60) == "L" );
	ASSERT_TRUE( computeAreaDesignator(100, -60) == "K" );
	ASSERT_TRUE( computeAreaDesignator(-10, -60) == "I" );
	ASSERT_TRUE( computeAreaDesignator(-100, -60) == "J" );

	//Test tropical belt
	ASSERT_TRUE( computeAreaDesignator(10, 10) == "H" );
	ASSERT_TRUE( computeAreaDesignator(100, 10) == "G" );
	ASSERT_TRUE( computeAreaDesignator(-10, -10) == "E" );
	ASSERT_TRUE( computeAreaDesignator(-100, -10) == "F" );
	ASSERT_TRUE( computeAreaDesignator(0, 0) == "H" );
}

TEST_F( BufrEncodeTest, Esss )
{
    StationInfoPtr stInfo( findWmoNo( 1492 ) ); //Dummy
    DataElementList data;
    DataElement dataElement;
    BufrDataPtr bufr;
    boost::posix_time::ptime t = getTime("2012-12-06 06:00:00");

    dataElement.time( t );
    dataElement.SA=-1;
    dataElement.SD=-1;
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_TRUE( bufr->EE == FLT_MAX );
    ASSERT_FLOAT_EQ( 0, bufr->SA );

    dataElement.SA=-1;
    dataElement.SD=1;
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_TRUE( bufr->EE == FLT_MAX );
    ASSERT_FLOAT_EQ( -0.02, bufr->SA );

    dataElement.SA=0;
    dataElement.SD=1;
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_TRUE( bufr->EE == FLT_MAX );
    ASSERT_FLOAT_EQ( 0, bufr->SA );

    dataElement.SA=23;
    dataElement.SD=1;
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_TRUE( bufr->EE == FLT_MAX );
    ASSERT_FLOAT_EQ( 0.23, bufr->SA );
}

TEST_F( BufrEncodeTest, EE_SA )
{
    StationInfoPtr stInfo( findWmoNo( 1492 ) ); //Dummy
    DataElementList data;
    DataElement dataElement;
    BufrDataPtr bufr;
    boost::posix_time::ptime t = getTime("2014-04-30 06:00:00");

    dataElement.time( t );
    dataElement.SA=-1;
    dataElement.EE=1;
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_TRUE( bufr->EE == 1 );
    ASSERT_FLOAT_EQ( 0.00, bufr->SA ) << "Expect: Now snow.";
}

TEST_F( BufrEncodeTest, PressureHeight )
{
    StationInfoPtr stInfo( findWmoNo( 1494 ) ); //Dummy
    DataElementList data;
    DataElement dataElement;
    BufrDataPtr bufr;
    EncodeBufrManager encoder;
    boost::posix_time::ptime t = getTime("2014-04-30 06:00:00");

    dataElement.time( t );
    data.insert( t, dataElement, true );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );

    BufrHelper bufrHelper( validater, stInfo, bufr );
    bufrHelper.setTest( true );

    bufrHelper.setObsTime( t );
    bufrHelper.setSequenceNumber( 0 );

    try {
    	encoder.encode( bufrHelper );
           //cerr << bufrHelper.getLog() << endl;
           //cerr << bufrHelper.getLog() << endl;

    	int len;
    	const char *buf=bufrHelper.getBufr( len );

    	EXPECT_TRUE( buf );
    	cerr << "#len: " << len << endl;
    	ofstream fout( "BUFR_pressureHeight");

    	fout.write( buf, len );
    	fout.close();
    	cerr << bufrHelper.getLog() << endl;
    }
    catch ( const std::exception &ex ) {
    	cerr << "<<<<< EXCEPTION LOG" << endl;
    	cerr << bufrHelper.getLog() << endl;
    	cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
    }
}



TEST_F( BufrEncodeTest, EsssFromEE )
{
	int wmono=1026;
	string datafile("90450_20130510T0600.dat");
	DataElementList data;
    DataElement dataElement;
    BufrDataPtr bufr;
    boost::posix_time::ptime t = getTime("2013-05-10 06:00:00");
    DataElementList allData;
    StationInfoPtr stInfo;
    EncodeBufrManager encoder;

    kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );
    stInfo = findWmoNo( wmono );


    ASSERT_TRUE( stInfo != 0 ) << "No station information for wmono " << wmono;
    ASSERT_TRUE( loadBufrDataFromFile( datafile, stInfo, allData, validData ) )
         << "Can't load data from file: " << datafile;

    data = allData.subData( t );
    bufr = bufrEncoder.doBufr( stInfo, data );

    ASSERT_TRUE( bufr != 0 );
    ASSERT_FLOAT_EQ( bufr->EE, 11 );
    ASSERT_FLOAT_EQ( bufr->EM, 1 );

    BufrTemplateList templateList;
    BufrHelper bufrHelper( validater, stInfo, bufr );
    bufrHelper.setTest( true );

    bufrHelper.setObsTime( t );
    bufrHelper.setSequenceNumber( 0 );


    try {
       encoder.encode( bufrHelper );
       //cerr << bufrHelper.getLog() << endl;
       //cerr << bufrHelper.getLog() << endl;

       EXPECT_TRUE(!bufrHelper.emptyBufr());
    }
    catch ( const std::exception &ex ) {
       cerr << "<<<<< EXCEPTION LOG" << endl;
       cerr << bufrHelper.getLog() << endl;
       cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
    }


}



TEST_F( BufrEncodeTest, EncodeBufr9000_only_one_parameter )
{
   DataElementList allData;
   StationInfoPtr stInfo;
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );


   int wmono=1607;
   stInfo = findWmoNo( wmono );

   ASSERT_TRUE( stInfo.get() ) << "No station information for wmono " << wmono;
   ASSERT_TRUE( loadBufrDataFromFile( "15460-20121205T05.dat", stInfo, allData, validData ) )
      << "Can't load data from file: 15460-20121205T05.dat";

   cerr << "allData: " << allData.size() << endl;
   dt=getTime("2012-12-05 05:00:00");
   data=allData.subData( dt );

   //cerr << "@@@@@@ dt: " << dt << " ft: " << data.firstTime()<<endl;
   EXPECT_TRUE( dt == data.firstTime() ) ;

   cerr << "data: " << data.size() << endl;
   BufrDataPtr  bufr = bufrEncoder.doBufr( stInfo, data );
   EXPECT_TRUE( bufr.get() );


   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );


   try {
      encoder.encode( bufrHelper );
      //cerr << bufrHelper.getLog() << endl;

      EXPECT_TRUE(!bufrHelper.emptyBufr());
      EXPECT_EQ( 1, bufrHelper.nValues() );
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   }
}



TEST_F( BufrEncodeTest, EncodeBufr90003_SHIP_PWA)
{
   DataElementList allData;
   StationInfoPtr stInfo;
   BufrDataPtr bufr( new BufrData() );
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;
   string datafile("data_LF4B.dat");
   string callSign("LF4B");

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   stInfo =  findCallsign( callSign );

   ASSERT_TRUE( stInfo.get() ) << "No station information for id '" << callSign << "'";
   ASSERT_TRUE( loadBufrDataFromFile( datafile, stInfo, allData, validData ) )
      << "Cant load data from file: '" << datafile << "'.";
   dt=getTime("2013-04-28 05:00:00");
   data=allData.subData( dt );

   EXPECT_TRUE( bufrEncoder.doBufr( stInfo, data, *bufr ) ) << "FAILED: Cant generate bufr for "<< callSign;

   BufrTemplateList templateList;
   BufrHelper bufrHelper( validater, stInfo, bufr );
   bufrHelper.setTest( true );

   b::assign::push_back( templateList )(900003);


   bufrHelper.setObsTime( dt );
   bufrHelper.setSequenceNumber( 0 );

   EXPECT_FLOAT_EQ( bufr->Pwa, 6.0 );
   EXPECT_FLOAT_EQ( bufr->Pw, FLT_MAX );
   EXPECT_FLOAT_EQ( bufr->Hwa, 3.1 );
   EXPECT_FLOAT_EQ( bufr->Hw, FLT_MAX );


   try {
      encoder.encode( templateList, bufrHelper );
      cerr << bufrHelper.getLog() << endl;

      int len;
      const char *buf=bufrHelper.getBufr( len );
      cerr << "#len: " << len << endl;
      ofstream fout( string(string("BUFR90003_")+callSign).c_str() );

      fout.write( buf, len );
      fout.close();
   }
   catch ( const std::exception &ex ) {
      cerr << "<<<<< EXCEPTION LOG" << endl;
      cerr << bufrHelper.getLog() << endl;
      cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
      FAIL();
   }
}

TEST_F( BufrEncodeTest, EncodeBufr307079_EmptyName1 )
{
   DataElementList allData;
   StationInfoPtr stInfo_1192;
   StationInfoPtr stInfo_1204;
   DataElementList data;
   boost::posix_time::ptime dt;
   EncodeBufrManager encoder;

   kvdatacheck::Validate validData( kvdatacheck::Validate::NoCheck );

   int wmono=1192;
   stInfo_1192 = findWmoNo( wmono );

   dt=getTime("2013-01-07 06:00:00");

   ASSERT_TRUE( stInfo_1192.get() ) << "No station information for wmono " << wmono;
   wmono=1204;
   stInfo_1204 = findWmoNo( wmono );
   ASSERT_TRUE( stInfo_1204.get() ) << "No station information for wmono " << wmono;
//   ASSERT_TRUE( loadBufrDataFromFile( "data_18700.dat", stInfo, allData, validData ) )
//      << "Cant load data from filr: data_18700.dat";

   DataElement de;

   de.TA = 10;
   data[dt] = de;

   BufrTemplateList templateList;
   b::assign::push_back( templateList )(900000);

   {
	   BufrDataPtr bufr( new BufrData() );
	   Bufr bEncoder;
	   bEncoder.setTest( true );

	   EXPECT_TRUE( bEncoder.doBufr( stInfo_1192, data, *bufr ) )
	   	   << "FAILED: Cant generate bufr for "<< 1192;
	   BufrHelper bufrHelper( validater, stInfo_1192, bufr );
	   bufrHelper.setTest( true );
	   bufrHelper.setObsTime( dt );
	   bufrHelper.setSequenceNumber( 0 );

	   try {
		   encoder.encode( templateList, bufrHelper );
		   int len;
		   const char *buf=bufrHelper.getBufr( len );
		   cerr << "#len: " << len << endl;
		   ofstream fout( "BUFR301090_empty_name_1" );

		   fout.write( buf, len );
		   fout.close();
	   }
	   catch ( const std::exception &ex ) {
		   cerr << "<<<<< EXCEPTION LOG" << endl;
		   cerr << bufrHelper.getLog() << endl;
		   cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
	   }
   }

   {
   	   BufrDataPtr bufr( new BufrData() );
   	   Bufr bEncoder;
   	   bEncoder.setTest( true );

   	   EXPECT_TRUE( bEncoder.doBufr( stInfo_1204, data, *bufr ) )
   	   	   << "FAILED: Cant generate bufr for "<< 1204;
   	   EXPECT_EQ( "", stInfo_1204->name());
   	   BufrHelper bufrHelper( validater, stInfo_1204, bufr );
   	   bufrHelper.setTest( true );
   	   bufrHelper.setObsTime( dt );
   	   bufrHelper.setSequenceNumber( 0 );

   	   try {
   		   encoder.encode( templateList, bufrHelper );
   		   int len;
   		   const char *buf=bufrHelper.getBufr( len );
   		   cerr << "#len: " << len << endl;
   		   ofstream fout( "BUFR301090_empty_name_2" );

   		   fout.write( buf, len );
   		   fout.close();
   	   }
   	   catch ( const std::exception &ex ) {
   		   cerr << "<<<<< EXCEPTION LOG" << endl;
   		   cerr << bufrHelper.getLog() << endl;
   		   cerr << ">>>>> EXCEPTION: " << ex.what() << endl;
   	   }
      }

}

TEST_F( BufrEncodeTest, MsgTimeNoMustHaveTypesTest )
{
   using namespace miutil;
   MsgTime t;
   MsgTime::Hours hours;
   int minToDelay;
   bool forceDelay;
   bool relativeToFirst;
   int stationid=76921;
   boost::posix_time::ptime test;
   StationInfoPtr stInfo;
   stInfo = findCallsign( "LF5X" );

   ASSERT_TRUE( stInfo.get() ) << "No station information for stationid " << stationid;

//   stInfo->printDelayInfo( cerr ) << endl;

   minToDelay = stInfo->delay( getTime("2014-06-07 00:00:00" ), forceDelay, relativeToFirst );

//   cerr << "minToDelay: " << minToDelay << " forceDelay: " << (forceDelay?"true":"false")
//		<< " relativeToFirst: "<< (relativeToFirst?"true":"false") << endl;

   ASSERT_TRUE( stInfo->msgForTime( getTime("2014-06-07 00:00:00" ) ));
   ASSERT_TRUE( minToDelay == 17 );
   ASSERT_FALSE( forceDelay );
   ASSERT_FALSE( relativeToFirst );
}
#endif

int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
