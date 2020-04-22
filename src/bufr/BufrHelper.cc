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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <new>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "milog/milog.h"
#include "../bufrencodehelper.h"
#include "../base64.h"
#include "../SemiUniqueName.h"
#include "../isDirWithWritePermission.h"
#include "../Strerror.h"
#include "BufrValueHelper.h"
#include "BufrHelper.h"
#include "EncodeBufrBase.h"
#include "boost/crc.hpp"


namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

using namespace std;


namespace {
void
init_sec0134(  int *ksec0, int *ksec1, int *ksec3, int *ksec4, int masterTabl=14 )
{

  /* Section 0 */
  ksec0[0] = 0;        /* Length of section 0, will be set by bufren */
  ksec0[1] = 0;        /* Total length of BUFR message, will be set by bufren */
  ksec0[2] = 4;        /* BUFR edition number */

  /*Section 1 */
  ksec1[ 0] = 22;      /* Length of section 1 (bytes). Must be set by user */
  ksec1[ 1] = 4;       /* BUFR edition number */
  ksec1[ 2] = 88;      /* Originating centre */
  ksec1[ 3] = 0;       /* ccx, Update sequence number */
  ksec1[ 4] = 0;       /* Flag (presence of section 2) */
  ksec1[ 5] = 0;       /* BUFR message type */
  ksec1[ 6] = 0;       /* BUFR message subtype */
  ksec1[ 7] = 0;       /* Version number of local table used */
  ksec1[ 8] = 0; //Year from obstime
  ksec1[ 9] = 0;//month from obstime
  ksec1[10] = 0;//day from obstime
  ksec1[11] = 0;//hour from obstime
  ksec1[12] = 0;//min from obstime
  ksec1[13] = 0;       /* BUFR master table */
  ksec1[14] = masterTabl;      /* Version number of master table used */
//  ksec1[14] = 19;      /* Version number of master table used */
  ksec1[15] = 0;       /* Originating sub-centre */
  ksec1[16] = 0; /* International sub-category (see common table C-13) */
  ksec1[17] = 0; //Secound from obstime

  /*Section 3*/
  ksec3[0] = 0;        /* Length of section 3 (bytes), will be set by bufren */
  ksec3[2] = 1;        /* Number of subsets */
  ksec3[3] = 128;      /* Flag (128 = observation data && no data compression) */

  /*Section 4*/
  ksec4[0] = 0;        /* Length of section 4 (bytes), will be set by bufren */
}



}

BufrHelper::
BufrHelper( BufrParamValidaterPtr paramValidater_,
            StationInfoPtr stationInfo_,
            BufrDataPtr bufrData_ )
   : paramValidater( paramValidater_ ),
     stationInfo( stationInfo_ ),
     data( bufrData_ ),
     kelem( KELEM ), kvals( KVALS ), kdlen( KDLEN ),
     ikdata( 0 ), iValue( 0 ), values( 0 ), iCvals( 0 ),
     ktdlen( 0 ),
     kbuflen( MAX_BUFLEN /4 ),
     kbuff( 0 ), ccx( 0 ), test( false ), validBufr_( true ),
     bufrBase( 0 )

{
   try {
      values = new Values( KVALS, paramValidater );
   }
   catch( ... ){
      kbuflen = 0;
      throw std::bad_alloc();
   }

   init_sec0134( ksec0, ksec1, ksec3, ksec4, EncodeBufrManager::masterBufrTable );
}


BufrHelper::
~BufrHelper()
{
   if( kbuff )
      delete[] kbuff;

   if( values )
      delete values;

}

StationInfoPtr
BufrHelper::
getStationInfo()const
{
   return stationInfo;
}

BufrDataPtr
BufrHelper::
getData()const
{
   return data;
}

BufrParamValidaterPtr
BufrHelper::
getParamValidater()const
{
   return paramValidater;
}

void
BufrHelper::
setGTSHeader( const std::string &TT,
              const std::string &AA )
{
   string ii=stationInfo->list();
   string CCCC=stationInfo->owner();
   gtsHeader = TT + AA;

   while( ii.size()<2)
      ii.insert(0, "0");

   ii = ii.substr( 0, 2 );

   gtsHeader += ii + " ";

   while( CCCC.size() < 4 )
      CCCC.insert(0, "X");

   gtsHeader += CCCC + " ";
}



//Section 1
void
BufrHelper::
setDataAndInternationalSubDataCategory( int dataCategory,
                                        int internationalSubCategory )
{
   ksec1[5] = dataCategory;   /* BUFR message type, table A*/

   /* International sub-category (see common table C-13) */
   ksec1[16] = internationalSubCategory;
}

void
BufrHelper::
setOriginatingCentreAndSubCentre( int originatingCenter,
                                  int subCenter )
{
   ksec1[ 2] = originatingCenter; /* Originating centre */
   ksec1[15] = subCenter;  /* Originating sub-centre */
}

void
BufrHelper::
setLocalDataSubCategory( int category )
{
    ksec1[6] = category;
}

void
BufrHelper::
setSequenceNumber( int ccx )
{
   this->ccx = ccx;
   ksec1[ 3] = ccx;
}

void
BufrHelper::
setObsTime( const pt::ptime &obstime_ )
{
   obstime = obstime_;
   boost::gregorian::date d=obstime.date();
   pt::time_duration t=obstime.time_of_day();
   ksec1[ 8] = d.year();
   ksec1[ 9] = d.month();
   ksec1[10] = d.day();
   ksec1[11] = t.hours();
   ksec1[12] = t.minutes();
   ksec1[17] = t.seconds();
}

void
BufrHelper::
addDescriptor( int descriptorId )
{
   if( descriptorId > 399999 ) {
      throw IdException( descriptorId, "Invalid descriptor.");
   }

   //cerr << "addDescriptor: " << descriptorId << endl;
   ktdlst[ ktdlen++] = descriptorId;
}

void
BufrHelper::
addDescriptorList( const BufrTemplateList &templateList )
{
   for( BufrTemplateList::const_iterator it=templateList.begin();
         it != templateList.end(); ++it )
      addDescriptor( *it );
}


//Section 3
void
BufrHelper::
addDelayedReplicationFactor( int paramid, int value,
                             const std::string &name)
{
   kdata[ikdata++] = value;
   (*values)[iValue++].insert( paramid, value, name );
}

void
BufrHelper::
addValue( int bufrParamId, float value,
          const std::string &name, bool countAsData, const std::string &testId )
{
   if( (*values)[iValue++].insert( bufrParamId, value, name ) ) {
      if ( test && !testId.empty()) {
         testHelper.setF(value, testId);
      }
	   if( countAsData ) {
		   bufrBase->setHasValidValue();
	   }
   }
}

void
BufrHelper::
addValue( int bufrParamId, int value,
          const std::string &name, bool countAsData, const std::string &testId )
{
   try {

      if( (*values)[iValue++].insert( bufrParamId, value, name ) ) {
        if ( test && !testId.empty()) {
            testHelper.setI(value, testId);
        }
    	  if( countAsData )
    		  bufrBase->setHasValidValue();
      }
   }
   catch( const TypeException &ex ) {
   //   values->log_ << "Exception: " << ex.what() << endl;
      --iValue;
      try {
         addValue( bufrParamId, static_cast<float>( value ), name, countAsData );
      }
      catch( const std::exception & /*Not used*/ ) {
         throw ex;
      }
   }
}



void
BufrHelper::
addValue( int bufrParamId, const std::string &value,
          const std::string &name, bool countAsData, const std::string &testId )
{
   string val( value );

   try {
       if( (*values)[iValue++].insert( bufrParamId, val, iCvals+1, name ) ) {
    	   //It seems that the BUFR software (fortran) code takes a
    	   //copy of the value we send it and allways takes the
    	   //numbers of bytes specified in the 'width' from the param definition
    	   //(B-tables) and copy to the BUFR. To remedy this we copy 'width' bytes
    	   //of 0xFF' to the cvals, this sets all bits in the string to 1s
    	   //(the missing indicator).

         
    	   if( value.empty() ) {
            cvalsLen[iCvals]=0;
    		   memset(cvals[iCvals++], 0xFF, val.length() );
         }else {
            cvalsLen[iCvals]=val.length()<value.length()?val.length():value.length();
    		   strncpy(cvals[iCvals++], val.c_str(), val.length() );
         }

         if ( test && !testId.empty()) {
            testHelper.setS(value, testId);
         }

         if( countAsData )
            bufrBase->setHasValidValue();
       }
   }
   catch( const std::exception  &ex ) {
       ostringstream ost;
       ost << "EXCEPTION: " << bufrParamId << " countAsData: " << (countAsData?"T":"F") << " Reason: " << ex.what();
       //cerr << ost.str() << endl;
       throw  BufrException( ost.str() );
   }
}

void
BufrHelper::
encodeBufr()
{
   int error;

   if( kbuff ) {
      delete[] kbuff;
      kbuff = 0;
   }

   try {

       kbuff = new int[kbuflen];
   }
   catch( ... ){
      kbuflen = 0;
      throw std::bad_alloc();
   }

   //cerr << "ktdlen: "<< ktdlen << endl;
   bufren_( ksec0, ksec1, ksec2, ksec3, ksec4, &ktdlen, ktdlst,
            &kdlen, kdata, &kelem, &kvals, values->values(), (char **) cvals,
            &kbuflen, kbuff, &error );

   LOGDEBUG( "kbuflen: " << kbuflen);
   LOGDEBUG( "ktdlen:  " << ktdlen );

   if( error != 0 ) {
      ostringstream o;
      o << "EncodeError errorcode: " << error << endl << values->log();
      delete[] kbuff;
      kbuff = 0;
      throw EncodeException( o.str() );
   }
}

const char*
BufrHelper::
getBufr( int &nSize )
{
   nSize = 0;

   if( ! kbuff )
      encodeBufr();

   nSize = kbuflen*4;
   return reinterpret_cast<char*>( kbuff );
}
std::string
BufrHelper::
getLog()const
{
   return values->log();
}

bool
BufrHelper::
writeToStream( std::ostream &o )
{
   int n;
   const char *buf=getBufr( n );

   if( test ) {
      o.write( buf, n );
      return o.good();
   }

   ostringstream ost;

   ost << setw(2) << setfill( '0') << obstime.date().day()
       << setw(2) << setfill( '0') << obstime.time_of_day().hours()
       << setw(2) << setfill( '0') << obstime.time_of_day().minutes();

   if( ccx > 0 ) {
      ost << " ";

      if( ccx > 24 )
         return false;

      ost << "CC" << static_cast<char>( 'A'+(ccx-1) );
   }

   o << "\r\r\nZCZC\r\r\n"
     << gtsHeader << ost.str()
     << "\r\r\n";

   o.write( buf, n );

   o << "\r\n\r\r\n\n\n\n\n\n\n\nNNNN\r\n";

   return o.good();
}

std::string
BufrHelper::
filePrefix( )const
{
   ostringstream ost;

   ost << stationInfo->toIdentString();
   ost << "-"
       << setfill('0') << setw(2) << obstime.date().day()
       << setfill('0') << setw(2) << obstime.time_of_day().hours()
       << setfill('0') << setw(2) << obstime.time_of_day().minutes();

   if( !encoderName.empty() )
      ost << "-C" << encoderName;

   if( ccx > 0 ) {
       ost << "-CC";

       if( ccx > 24 )
          ost << ccx;
       else
          ost << static_cast<char>( 'A'+(ccx-1) );
    }


    return ost.str();
}

bool
BufrHelper::
emptyBufr() const
{
    if( bufrBase ) {
        return bufrBase->emptyBufr();
    }
    return false;
}

int
BufrHelper::
nValues() const
{
    if( bufrBase ) {
          return bufrBase->nValues();
    }

    return -1;
}



std::tuple<bool,std::string> 
BufrHelper::saveToFile(const std::string &path, int suffix){
   fs::ofstream f;
   ostringstream o;
   o << path<<"/"<<filePrefix()<<"_"<< suffix <<".bufr";
   string filename=o.str();
   f.open( filename, ios_base::trunc | ios_base::binary | ios_base::out );

   if( ! f.is_open() ){
      ostringstream o;
      o << "failed to create the file '" << filename << "'.";
      return make_tuple(false, o.str());
   }

   bool fok = writeToStream( f );
   f.close();

   if( !fok ) {
      unlink( filename.c_str() );
      ostringstream o;
      o << "failed to write BUFR to: " << filename << ".";
      return make_tuple(false, o.str());
   }
   return make_tuple(true, filename);
}  

void
BufrHelper::
saveToFile( const std::string &path,
            bool overwrite, bool isTestRun)
{
   fs::ofstream f;
   string error;
   string tmppath( path +"/tmp" );
   string filename( (isTestRun?(filePrefix()+".bufr"):SemiUniqueName::uniqueName( filePrefix(), ".bufr" )) );
   string tmpfile(tmppath + "/" + filename);
   string dstfile(path + "/" + filename);

   if( ! isDirWithWritePermission( path, error ) ) {
      ostringstream o;
      o << "Save to path: '" << path << "'. "
        << "Path not a directory or permission denied."
        << "("<< error << ")";

      throw std::logic_error( o.str() );
   }

   if( ! isDirWithWritePermission( tmppath, error ) )
      tmpfile.erase();

   if( ! tmpfile.empty() )
      f.open( tmpfile, ios_base::trunc | ios_base::binary | ios_base::out );
   else
      f.open( dstfile, ios_base::trunc | ios_base::binary | ios_base::out );

   if( ! f.is_open() ){
      ostringstream ost;
      ost << "Failed to write BUFR file for station '" << stationInfo->toIdentString() << "' obstime: " << obstime
          << ". File <" << dstfile << ">.";
      LOGDEBUG( ost.str() << " Overwrite: "<< (overwrite?"true":"false") );
      throw std::logic_error( ost.str() );
   }

   bool fok = writeToStream( f );
   f.close();

   if( !fok ) {
      string file;
      if( ! tmpfile.empty()  )
         file = tmpfile;
      else
         file = dstfile;

      unlink( file.c_str() );

      ostringstream ost;
      ost << "Failed to write file: " << file << ".";
      throw std::logic_error( ost.str() );
   }

   if( ! tmpfile.empty() ) {
      if( rename( tmpfile.c_str(), dstfile.c_str() ) == -1 ) {
         ostringstream ost;
         ost << "Failed to move: " << tmpfile << " -> " << dstfile
               << ". Reason: " << Strerror( errno );
         throw std::logic_error( ost.str() );
      } else {
         LOGDEBUG( "saveToFile: moved: " << tmpfile << " -> " << dstfile );
      }
   } else {
      LOGDEBUG( "saveToFile: write: " << dstfile );
   }
}

void
BufrHelper::
saveToFile( bool overwrite)
{
   if( stationInfo->copy() ) {
      saveToFile( stationInfo->copyto(), overwrite );
   }
}
void
BufrHelper::
validBufr( bool f )
{
    validBufr_ = f;
}

bool
BufrHelper::
validBufr()const
{
    return validBufr_;
}
void
BufrHelper::
addErrorMessage( const std::string &error )
{
    if( ! errorMessage.str().empty() ) {
        errorMessage << endl;
    }

    errorMessage << error;
}

std::string
BufrHelper::
getErrorMessage()const
{
    return errorMessage.str();
}


boost::uint32_t BufrHelper::computeCRC()const{
   ostringstream o;
   for( size_t i=0; i<iValue; i++) {
      o << (*values)[i] << endl;
   }

   for(size_t i=0; i<iCvals; ++i) {
      string s(&cvals[i][0], cvalsLen[i]);
      o << "'" << s << "'" << endl;
   }

   boost::crc_32_type crcChecker;
   string msg=o.str();
   

   crcChecker.process_bytes( msg.c_str(),  msg.length() );

   cerr << msg;
   return crcChecker.checksum();
}