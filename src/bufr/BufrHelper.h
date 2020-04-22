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

#ifndef __BUFRHELPER_H__
#define __BUFRHELPER_H__

#include <stdexcept>
#include <string>
#include <tuple>
#include "boost/date_time/posix_time/ptime.hpp"
#include "bufrdefs.h"
#include "bufrexceptions.h"
#include "BUFRparam.h"
#include "../StationInfo.h"
#include "../BufrData.h"

typedef std::list<int> BufrTemplateList;

class Values;
class EncodeBufrBase;

class TestHelper {
   public:
   struct Value {
      typedef enum{ F, I, S, U} VT;
      VT vt;
      float fv;
      int iv;
      std::string sv;

      Value():vt(U){}
      Value(float v): vt(F), fv(v){} 
      Value(int v): vt(I), iv(v){} 
      Value(const std::string &v): vt(S), sv(v){} 

      float getF()const { return vt==F?fv:FLT_MAX;}
      int   getI()const { return vt==I?iv:INT_MAX;}
      std::string   getS()const { return vt==S?sv:"";}
   };
   
   
   public:
   void setF(float v, const std::string &id) { tests[id]=Value(v);}
   void setI(int v, const std::string &id) { tests[id]=Value(v);}
   void setS(const std::string &v, const std::string &id) { tests[id]=Value(v);}
   Value get(const std::string &id) { return tests[id];}
   private:
   std::map<std::string, Value> tests;
};

class BufrHelper {
   BufrHelper( );
   BufrHelper( const BufrHelper & );
   BufrHelper& operator=(const BufrHelper &);

   BufrParamValidaterPtr paramValidater;
   StationInfoPtr stationInfo;
   BufrDataPtr data;

   TestHelper testHelper;

   /* bufren variables */
   int kelem;
   int kvals;
   int kdlen;
   int ksec0[3], ksec1[40], ksec2[64], ksec3[4], ksec4[2];
   int ikdata;  //Index into kdata
   int kdata[KDLEN];      /* integer array containing data needed for data
                           * descriptor expansion (delayed replication factors) */
   int  iValue;   //Index into values.
   Values *values;        /* expanded data values */
   int  iCvals;
   char cvals[KVALS][80]; /* String values - index to which are extracted
                           * from values array */
   size_t cvalsLen[KVALS];
   int ktdlen;            /* number of data descriptors in section 3 */
   int ktdlst[KELEM];     /* array containing data descriptors in section 3 */

   int kbuflen;
   int *kbuff; /* integer array containing bufr message */
   boost::posix_time::ptime obstime;
   int ccx;
   bool test;
   std::string gtsHeader;
   bool validBufr_;
   std::ostringstream errorMessage;

   friend class EncodeBufrBase;

public:
   std::string encoderName;
   EncodeBufrBase *bufrBase;
   
   //std::string gtsHeader;

   BufrHelper( BufrParamValidaterPtr paramValidater,
               StationInfoPtr stationInfo_,
               BufrDataPtr bufrData_ );

   BufrHelper( StationInfoPtr stationInfo_,
               BufrDataPtr bufrData_ );               
   ~BufrHelper();

   bool emptyBufr() const;
   int  nValues() const;

   StationInfoPtr getStationInfo()const;
   BufrDataPtr getData()const;
   BufrParamValidaterPtr getParamValidater()const;
   int getCCX()const { return ccx; }
   boost::posix_time::ptime getObstime()const{ return obstime;}
   void setGTSHeader( const std::string &TT,
                      const std::string &AA );
   void setTest( bool f ) { test=f; }

   //Section 1
   void setDataAndInternationalSubDataCategory( int dataCategory, int subCategory );
   void setSequenceNumber( int ccx );
   void setObsTime( const boost::posix_time::ptime &obstime );
   void setOriginatingCentreAndSubCentre( int originatingCenter,
                                          int subCenter=0 );
   /**
    * setLocalDataSubCategory.
    * Used by local data-processing senter.
    */
   void setLocalDataSubCategory( int category );

   //Section 3
   void addDescriptor( int descriptorId );
   void addDescriptorList( const BufrTemplateList &templateList );

   //Section 4
   void addDelayedReplicationFactor( int paramid, int value, const std::string &name="" );
   void addValue( int bufrParamId, float value, const std::string &name="", bool countAsData=true, const std::string &testId="" );
   void addValue( int bufrParamId, int value, const std::string &name="", bool countAsData=true, const std::string &testId="" );
   void addValue( int bufrParamId, const std::string &value, const std::string &name="", bool countAsData=true, const std::string &testId="" );

   //throw EncodeException
   void encodeBufr();

   //throw EncodeException
   const char* getBufr( int &nSize );
   std::string getLog()const;

   //throw EncodeException
   bool writeToStream( std::ostream &o );

   /**
    * Save the BUFR to the directory given with path.
    * The file is owerwriten if it exist. This function exist mainly
    * for test.
    * 
    * Return a tuple. Where the first element is a boolean and the second element
    * is an string.
    * 
    * Return
    *    Success <true, filenameWithPath>
    *    Error   <false, errorMessage>
    */
   std::tuple<bool,std::string> saveToFile(const std::string &path, int suffix=0);

   /**
    * Create a string that is composed of stationInfo->->toIdentString() appended
    * with observation time coded as  '-DDhhmm', where DD is day of month, hh hour and mm minute.
    */
   std::string filePrefix()const;

   /**
    * This save the BUFR to a file in the directory given with path.
    * The file get the name given with filePrefix(). If isTestRun is false
    * it get added something that make the file unique in the directory.
    */
   void  saveToFile( const std::string &path,
                     bool overwrite, bool isTestRun=false);

   /**
    * Check if copyto is set to true in StationInfo, if
    * so call saveToFile( stationInfo->copyto, ... )
    *
    * If overwrite is false, a semiuniqe file name is generated.
    * The name is guaranteed to be unique if only one process is
    * trying to create a name in the same directory at the same
    * time.
    *
    * @param overwrite Overwrite the file if it exist.
    */
   void saveToFile( bool overwrite=false);
   void validBufr( bool f );
   bool validBufr()const;
   void addErrorMessage( const std::string &error );
   std::string getErrorMessage()const;

   boost::uint32_t computeCRC()const;

   //Only for Test
   TestHelper::Value getTestValue(const std::string &testId) {
      return testHelper.get(testId);
   }

};

#endif
