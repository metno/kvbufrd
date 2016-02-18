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
#include <milog/milog.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "miutil/timeconvert.h"
#include "bufrencodehelper.h"
#include "base64.h"
#include "SemiUniqueName.h"
#include "bufr/bufrdefs.h"
#include "bufr/BufrValueHelper.h"

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

using namespace std;

namespace {
bool
set_sec0134( const StationInfoPtr station, const DataElement &data, int ccx,
             int *ksec0, int *ksec1, int *ksec3, int *ksec4 );

void set_values( const StationInfoPtr station,
                 const BufrData &data,
                 Values &values, char cvals[][80], int kdata[]);

bool
isDirWithWritePermission( const std::string &path,
                          std::string &error );
std::string
myStrerror(int errnum );

}



BufrEncoder::
BufrEncoder(StationInfoPtr station_, bool test_)
   : station( station_ ), kbuff( 0 ), test( test_ )
{

};

BufrEncoder::
~BufrEncoder()
{
   if( kbuff )
      delete[] kbuff;
}
void
BufrEncoder::
encodeBufr( const BufrData &data, int ccx_ )
{ /* pbopen variables */
  int fd;
  int error;
  string filename;
  ccx = ccx_;
  obstime = data.time();

/* bufren variables */
  int kelem = KELEM, kvals = KVALS, kdlen = KDLEN;
  int ksec0[3], ksec1[40], ksec2[64], ksec3[4], ksec4[2];
  int kdata[KDLEN];      /* integer array containing data needed for data
		                    * descriptor expansion (delayed replication factors) */
  Values values( KVALS );  /* expanded data values */
  char cvals[KVALS][80]; /* String values - index to which are extracted
                          * from values array */
  int ktdlen;            /* number of data descriptors in section 3 */
  int ktdlst[KELEM];     /* array containing data descriptors in section 3 */

/* pbwrite variables */
  int nbytes;            /* number of bytes in bufr messages */

  if( ! kbuff ) {
     try {
        kbuflen = MAX_BUFLEN /4;
        kbuff = new int[kbuflen];
     }
     catch( ... ){
        kbuflen = 0;
        throw EncodeException( "NOMEM to allocate a buffer to hold the bufr message.");
     }
  }

  /* Set input parameters to bufren */
  set_sec0134( station, data, ccx, ksec0, ksec1, ksec3, ksec4);
  set_values( station, data, values, cvals, kdata);

  LOGDEBUG("Bufr values: " << values.log() );

  ktdlen = 3;
  ktdlst[0] = 307079;

  //The following descriptors is used only internally by met.no
  ktdlst[1] = 4025;  // 0 04 025 Time displacement (Before the obstime)
  ktdlst[2] = 11042; // 0 11 042 Maximum wind speed (10 min mean wind).

  /* Encode BUFR message */
  bufren_( ksec0, ksec1, ksec2, ksec3, ksec4, &ktdlen, ktdlst,
           &kdlen, kdata, &kelem, &kvals, values.values(), (char **) cvals,
           &kbuflen, kbuff, &error );

  LOGDEBUG( "kbuflen: " << kbuflen);
  LOGDEBUG( "ktdlen:  " << ktdlen );

  if( error != 0 ) {
     ostringstream o;
     o << "Failed to encode bufr for station '" << station->wmono() << "' obstime: " << data.time()
       << ". bufren error code: " << error << endl << values.log();
     delete[] kbuff;
     kbuff = 0;
     throw EncodeException( o.str() );
  }
}

const char*
BufrEncoder::
getBufr( int &nSize )const
{
   nSize = 0;
   if( ! kbuff )
      return 0;

   nSize = kbuflen*4;
   return reinterpret_cast<char*>( kbuff );
}

void
BufrEncoder::
saveToFile( bool overwrite)const
{
   if( ! kbuff ) {
      ostringstream o;
      o << "Missing bufr encoding for station '" << station->wmono() << "' obstime: " << obstime;
      throw EncodeException( o.str() );
   }

   if( station->copy() ) {
      saveToFile( station->copyto(), overwrite );
   }
}

std::string
BufrEncoder::
wmoHeader()const
{
   string header;
   string sTmp;
   string tmpOwner=station->owner();
   char tmp[16];
   char dateTime[16];
   int  ccx_=ccx;

   if( ! test ) {
     int day=obstime.date().day();
     int hour=obstime.time_of_day().hours();
      sprintf( dateTime,"%02d%02d", day, hour );
      header="\r\r\nZCZC\r\r\n";
      switch( hour ){
      case 0:
      case 6:
      case 12:
      case 18:
         header+="ISMD";
         break;
      case 3:
      case 9:
      case 15:
      case 21:
         header+="ISID";
         break;
      default:
         header+="ISND";
         break;
      }

      //Prepend the string with 0
      //until it is at least 2 byte long.
      sTmp = station->list();
      while( sTmp.length()<2 )
         sTmp.insert(0, "0");

      sprintf(tmp,"%2.2s ", sTmp.c_str() );
      header+=tmp;

      while( tmpOwner.length()<4)
         tmpOwner.insert(0, " ");

      tmpOwner+=" ";
      header+=tmpOwner;
      header+=dateTime;
      header.append("00");

      if( ccx_ > 0 ) {
         if( ccx_ >= 10 )
            ccx_ = 9;
         sprintf(tmp, " CC%c", 'A'+(ccx_-1));
         header+=tmp;
      }
      header+="\r\r\n";
   }
   return header;
}


std::string
BufrEncoder::
filePrefix()const
{
   ostringstream ost;

   if( station->wmono() > 0 ){
       ost << "wmono_" << station->wmono();
    } else {
       ost << "sid_";
       StationInfo::TLongList ids=station->definedStationID();
       for( StationInfo::TLongList::const_iterator it= ids.begin();
            it != ids.end(); ++it ) {
          if( it != ids.begin() )
                ost << "_";
          ost << *it;
       }
    }

    ost << "-"
        << setfill('0') << setw(2) << obstime.date().day()
        << setfill('0') << setw(2) << obstime.time_of_day().hours();


    if( ccx > 0 ){
       char cc = static_cast<char>('A'+(ccx-1));
       if( ccx > 26 )
          cc = 'x';

       ost << "-CC" << cc;
    }

    return ost.str();
}


void
BufrEncoder::
saveToFile( const std::string &path, bool overwrite )const
{
   fs::ofstream      f;
   string error;
   string tmppath( path +"/tmp" );
   string filename( SemiUniqueName::uniqueName( filePrefix(), ".bufr" ) );
   string tmpfile(tmppath + "/" + filename);
   string dstfile(path + "/" + filename);


   if( ! kbuff ) {
       ostringstream o;
       o << "Missing bufr encoding for station '" << station->wmono() << "' obstime: " << obstime;
       throw EncodeException( o.str() );
   }

   if( ! isDirWithWritePermission( path, error ) ) {
      ostringstream o;
      o << "Save to path: '" << path << "'. "
        << "Path not a directory or permission denied."
        << "("<< error << ")";

      throw EncodeException( o.str() );
   }

   if( ! isDirWithWritePermission( tmppath, error ) )
      tmpfile.erase();

   if( ! tmpfile.empty() )
      f.open( tmpfile, ios_base::trunc | ios_base::binary | ios_base::out );
   else
      f.open( dstfile, ios_base::trunc | ios_base::binary | ios_base::out );

   if( ! f.is_open() ){
      ostringstream ost;
      ost << "Failed to write BUFR file for station '" << station->wmono() << "' obstime: " << obstime
          << ". File <" << dstfile << ">.";
      LOGDEBUG( ost.str() << " Overwrite: "<< (overwrite?"true":"false") );
      throw EncodeException( ost.str() );
   }

   writeToStream( f );
   f.close();

   if( ! tmpfile.empty() ) {
      if( rename( tmpfile.c_str(), dstfile.c_str() ) == -1 ) {
         ostringstream ost;
         ost << "Failed to move: " << tmpfile << " -> " << dstfile
               << ". Reason: " << myStrerror( errno );
         throw EncodeException( ost.str() );
      } else {
         LOGDEBUG( "saveToFile: moved: " << tmpfile << " -> " << dstfile );
      }
   } else {
      LOGDEBUG( "saveToFile: write: " << dstfile );
   }
}

bool
BufrEncoder::
writeToStream( std::ostream &out )const
{
   out << wmoHeader();
   out.write( reinterpret_cast< ofstream::char_type* >( kbuff ), kbuflen*4 );

   if( ! test )
      out << "\r\n\r\r\n\n\n\n\n\n\n\nNNNN\r\n";

   return out.good();
}


namespace {

bool
set_sec0134( const StationInfoPtr station,
             const DataElement &data,
             int ccx,
             int *ksec0, int *ksec1, int *ksec3, int *ksec4)
{
  int year, month, day, hour, minute, second; /* Termin time */ 
  pt::ptime obsTime=data.time();

  if( obsTime.is_special() )
     return false;

  /* Section 0 */
  ksec0[0] = 0;        /* Length of section 0, will be set by bufren */
  ksec0[1] = 0;        /* Total length of BUFR message, will be set by bufren */
  ksec0[2] = 4;        /* BUFR edition number */

  /*Section 1 */
  ksec1[ 0] = 22;      /* Length of section 1 (bytes). Must be set by user */ 
  ksec1[ 1] = 4;       /* BUFR edition number */
  ksec1[ 2] = 88;      /* Originating centre */
  ksec1[ 3] = ccx;       /* Update sequence number */
  ksec1[ 4] = 0;       /* Flag (presence of section 2) */
  ksec1[ 5] = 0;       /* BUFR message type */
  ksec1[ 6] = 0;       /* BUFR message subtype */
  ksec1[ 7] = 0;       /* Version number of local table used */
  ksec1[ 8] = obsTime.date().year();
  ksec1[ 9] = obsTime.date().month();
  ksec1[10] = obsTime.date().day();
  ksec1[11] = obsTime.time_of_day().hours();
  ksec1[12] = obsTime.time_of_day().minutes();
  ksec1[13] = 0;       /* BUFR master table */
  ksec1[14] = 14;      /* Version number of master table used */
  ksec1[15] = 0;       /* Originating sub-centre */

  /* International sub-category (see common table C-13) */
  switch( obsTime.time_of_day().hours() ){
  case 0:
  case 6:
  case 12:
  case 18:
     ksec1[16] = 2; //ISM
     break;
  case 3:
  case 9:
  case 15:
  case 21:
     ksec1[16] = 1; //ISI
     break;
  default:
     ksec1[16] = 0; //ISN
     break;
  }

  ksec1[17] = obsTime.time_of_day().seconds();

  /*Section 3*/
  ksec3[0] = 0;        /* Length of section 3 (bytes), will be set by bufren */ 
  ksec3[2] = 1;        /* Number of subsets */ 
  ksec3[3] = 128;      /* Flag (128 = observation data && no data compression) */ 

  /*Section 4*/
  ksec4[0] = 0;        /* Length of section 4 (bytes), will be set by bufren */ 
}

/* Populate values and cvals with values according to WMO BUFR template 307080 */
void set_values(const StationInfoPtr station,
                const BufrData       &data,
                Values &values, char cvals[][80], int kdata[])
{
   double miss = RVIND;
   int idx, i;
   int iDelay=0;

   float t_ww;          /* Present and past weather */
   pt::ptime obstime = data.time();

   if( (obstime.time_of_day().hours()%6) == 0 )
      t_ww = -6;
   else if( (obstime.time_of_day().hours()%3) == 0 )
      t_ww = -3;
   else
      t_ww = FLT_MAX;

   /* Fixed surface station identification, time, horizontal and vertical coordinates */
   values[0].toBufr( "II", static_cast<int>( station->wmono()/1000 ) ) ;        /* 001001 WMO block number  II*/
   values[1].toBufr( "iii", static_cast<int>( station->wmono()%1000 ) );        /* 001002 WMO station number  iii*/
   values[2] = 1020;      /* Pointer to cvals (001015 Station or site name) */
   values[3].toBufr( "ix", data.IX );       /* 002001 Type of station ix*/
   values[4].toBufr( "Year", obstime.date().year() );      /* 004001 Year */
   values[5].toBufr( "Month", obstime.date().month() );     /* 004002 Month */
   values[6].toBufr( "YY", obstime.date().day() );        /* 004003 Day */
   values[7].toBufr( "GG", obstime.time_of_day().hours() );        /* 004004 Hour */
   values[8].toBufr( "gg", obstime.time_of_day().minutes() );;        /* 004005 Minute */
   values[9].toBufr( "Latitude", station->latitude() );       /* 005001 Latitude (high accuracy) */
   values[10].toBufr( "Longitude", station->longitude() );      /* 006001 Longitude (high accuracy) */
   values[11].toBufr( "Station height", station->height() );       /* 007030 Height of station ground above mean sea level */

   /* Basic synoptic "instantaneous data" */

   /* Pressure data */
   values[12].toBufr( "height-pressure", station->heightPressure() );       /* 007031 Height of barometer above mean sea level */
   values[13].toBufr( "P0P0P0P0", data.PO ); /* 010004 Pressure */
   values[14].toBufr( "PPPP", data.PR );     /* 010051 Pressure reduced to mean sea level */
   values[15].toBufr( "ppp", data.PP );      /* 010061 3-hour pressure change */
   values[16].toBufr( "a", data.AA );        /* 010063 Characteristic of pressure tendency */
   values[17].toBufr( "p24p24p24", FLT_MAX );/* 010062 24-hour pressure change */

   values[18].toBufr( "a3", FLT_MAX );       /* 007004 Pressure (standard level) */
   values[19].toBufr( "hhh", FLT_MAX );      /* 010009 Geopotential height */

   /* Temperature and humidity data */
   values[20].toBufr( "h_t", station->heightTemperature() );      /* 007032 Height of sensor above local ground (or deck of marine platform) */
   values[21].toBufr( "snTTT", data.TA );    /* 012101 Temperature/dry-bulb temperature */
   values[22].toBufr( "snTdTdTd", data.TD ); /* 012103 Dew-point temperature */
   values[23].toBufr( "UUU", data.UU );      /* 013003 Relative humidity */

   /* Visibility data */
   values[24].toBufr("h_V", station->heightVisability() );      /* 007032 Height of sensor above local ground (for visibility measurement) */
   values[25].toBufr( "VV", data.VV );       /* 020001 Horizontal visibility */

   /* Precipitation past 24 hours */
   values[26].toBufr( "h_P", station->heightPrecip() );      /* 007032 Height of sensor above local ground (for precipitation measurement) */
   values[27].toBufr( "R24R24R24R24", data.precip24.RR ); /* 013023 Total precipitation past 24 hours */
   values[28] = RVIND;    /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */

   /* Cloud data */
   values[29].toBufr( "N", data.N );        /* 020010 Cloud cover (total) */
   values[30].toBufr( "vsc", data.vsci );      /* 008002 Vertical significance (surface observations) */
   values[31].toBufr( "NH", data.NH );       /* 020011 Cloud amount (of low or middle clouds) */
   values[32].toBufr( "HL", data.HL );        /* 020013 Height of base of cloud */
   values[33].toBufr( "CL", data.CL );       /* 020012 Cloud type (low clouds CL) */
   values[34].toBufr( "CM", data.CM );       /* 020012 Cloud type (middle clouds CM) */
   values[35].toBufr( "CH", data.CH );       /* 020012 Cloud type (high clouds CH) */

   /* Individual cloud layers or masses */
   /* Looks like Norwegian stations do not report these parameters, except for
    * those sending synop. So the loop below can probably be simplified */
   values[36].toBufr( "num-cloud-layers", static_cast<int>(data.cloudExtra.size()) ); /* 031001 Delayed descriptor replication factor */
   kdata[iDelay++] = static_cast<int>(data.cloudExtra.size()); /* Number of cloud layers */
   idx = 37;
   for (i=0; i < data.cloudExtra.size(); i++) {
      values[idx++].toBufr( "vsci[i]", data.cloudExtra[i].vsci );/* 008002 Vertical significance (surface observations) */
      values[idx++].toBufr( "Ns[i]", data.cloudExtra[i].Ns );  /* 020011 Cloud amount */
      values[idx++].toBufr( "C[i]", data.cloudExtra[i].C );   /* 020012 Cloud type */
      values[idx++].toBufr( "hshs[i]", data.cloudExtra[i].hshs );/* 020013 Height of base of cloud */
   }

   /* 3 02 036
    * Clouds with bases below station level */
   /* These are not reported for Norwegian observations. Are we allowed to set
    * delayed replication to 0? */
   values[idx++] = 0;     /* 031001 Delayed descriptor replication factor */
   kdata[iDelay++] = 0;
#if 0
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020011 Cloud amount (N') */
   values[idx++] = miss;  /* 020012 Cloud type (C') */
   values[idx++] = miss;  /* 020014 Height of top of cloud (H'H') */
   values[idx++] = miss;  /* 020017 Height of top of cloud (Ct) */
#endif

   /* 3 02 047
    * Direction of cloud drift (56DLDMDH)
    */
   values[idx++] = 0;     /* 031000 Delayed descriptor replication factor */
   kdata[iDelay++] = 0;
#if 0
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
#endif
   values[idx++] = RVIND; /* 008002 Vertical significance, set to missing to cancel the previous value */

   /* 3 02 048
    * Direction and elevation of clouds (57CDaec)
    */
   values[idx++] = 0;     /* 0 31 000 replication factor. */
   kdata[iDelay++] = 0;
#if 0
   values[idx++] = miss;  /* 005021 Bearing or azimuth */
   values[idx++] = miss;  /* 007021 Elevation (see note 2) */
   values[idx++] = miss;  /* 020012 Cloud type */
   values[idx++] = RVIND; /* 005021 Bearing or azimuth, set to missing to cancel the previous value */
   values[idx++] = RVIND; /* 007021 Elevation, set to missing to cancel the previous value */
#endif

   /* 3 02 037
    *  State of ground, snow depth, ground minimum temperature
    */
   values[idx++].toBufr( "EE", data.EE );    /* 020062 State of the ground (with or without snow) */
   values[idx++].toBufr( "sss", data.SA );   /* 013013 Total snow depth */
   values[idx++].toBufr( "snTgTg", data.TGN_12 );/* 012113 Ground minimum temperature, past 12 hours */

   /* State of the sea. */
   values[idx++] = (data.SG.valid()?1:0); /* 0 31 000 replication factor. */
   kdata[iDelay++]=(data.SG.valid()?1:0);
   if( data.SG.valid() ) {
      values[idx++].toBufr( "S", data.SG );  // 0 22 061 State of sea
      values[idx++].toBufr( "VS", FLT_MAX ); // 0 20 058 Visibility seawards from coastal station
   }

   /* 3 02 056
    * Sea/water surface temperature, method of measurement, and depth below sea surface
    */
   values[idx++] = (data.TW.valid()?1:0); /* 0 31 000 replication factor. */
   kdata[iDelay++]=(data.TW.valid()?1:0);
   if( data.TW.valid() ) {
      values[idx++].toBufr( "Method of measurement", 14 );          //0 02 038 Method of sea/water temperature measurement. 14 = Other
      values[idx++].toBufr("Sea/water depth of measurement", 0.5f ); //0 07 063 Depth below sea/water surface.
      values[idx++].toBufr("TW", data.TW );                         //0 22 043 Sea/water temperature
      values[idx++].toBufr("Sea/water depth of measurement", FLT_MAX ); //0 07 063 Depth below sea/water surface.
   }                                                                    //(Set to missing to cancel the previous value. (What does this mean?)

   /* 3 02 055 Icing and Ice.
    * At the momment we do not report this.
    */
   values[idx++] = 0;  /* 0 31 000 replication factor. */
   kdata[iDelay++]=0;
#if 0
   values[idx++].toBufr( "EsEs", FLT_MAX );  //0 20 031 Ice deposit (thicknes).
   values[idx++].toBufr( "Rs", FLT_MAX );    //0 20 032 Rate of ice accreation.
   values[idx++].toBufr( "Is", FLT_MAX );    //0 20 033 Cause of ice accreation.
   values[idx++].toBufr( "Ci", FLT_MAX );    //0 20 034 Sea ice concentration.
   values[idx++].toBufr( "Bi", FLT_MAX );    //0 20 035 Amount and type of ice.
   values[idx++].toBufr( "Zi", FLT_MAX );    //0 20 036 Ice situation.
   values[idx++].toBufr( "Si", FLT_MAX );    //0 20 037 Ice development.
   values[idx++].toBufr( "Di", FLT_MAX );    //0 20 038 Bearing of ice edge.
#endif

   /* 3 02 043
    * Basic synoptic "period data"
    */
   /* Present and past weather */
   values[idx++].toBufr( "ww", data.ww );    /* 020003 Present weather (see note 1) */
   values[idx++].toBufr( "t_ww", t_ww );  /* 004024 Time period or displacement (for W1 and W2) */
   values[idx++].toBufr( "W1", data.W1 );    /* 020004  Past weather (1) (see note 2) */
   values[idx++].toBufr( "W2", data.W2 );    /* 020005  Past weather (2) (see note 2) */

   /* Sunshine data (1 hour and 24 hour period) */
   values[idx++] = -1;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "SS", FLT_MAX );    /* 014031 Total sunshine OT_1 */
   values[idx++] = -24;    /* 004024 Time period or displacement */
   values[idx++].toBufr( "SSS", FLT_MAX );   /* 014031 Total sunshine OT_24*/

   /* Precipitation measurement */
   values[idx++].toBufr( "h_P", station->heightPrecip() );   /* 007032 Height of sensor above local ground (for precipitation measurement) */
   values[idx++].toBufr( "tR[0] (Regional)", data.precipRegional.hTr ); /* 004024 Time period or displacement (regional) */
   values[idx++].toBufr( "RRR[0] (Regional)", data.precipRegional.RR );/* 013011 Total precipitation/total water equivalent */
   values[idx++].toBufr( "tR[1] (National)",  data.precipNational.hTr ); /* 004024 Time period or displacement (national) */
   values[idx++].toBufr( "RRR[1] (National)", data.precipNational.RR );/* 013011 Total precipitation/total water equivalent */

   /* Extreme temperature data */
   values[idx++].toBufr( "h_T",  station->heightTemperature() );   /* 007032 Height of sensor above local ground (for temperature measurement) */
   values[idx++].toBufrIf("tTAX_N_1", data.tTAX_N, data.tTAX_N != INT_MAX );   /* 004024 Time period or displacement */
   values[idx++].toBufrIf("tTAX_N_2", 0, data.tTAX_N != INT_MAX );     /* 004024 Time period or displacement */
   values[idx++].toBufr( "TAX_N", data.TAX_N ); /* 012111 Maximum temperature, at height and over period specified */
   values[idx++].toBufrIf("tTAN_N_1", data.tTAN_N, data.tTAN_N != INT_MAX );   /* 004024 Time period or displacement */
   values[idx++].toBufrIf("tTAN_N_2", 0, data.tTAN_N != INT_MAX );     /* 004024 Time period or displacement */
   values[idx++].toBufr( "TAN_N", data.TAN_N ); /* 012112 Minimum temperature, at height and over period specified */

   /* Wind data */
   values[idx++].toBufr( "h_W", station->heightWind() );   /* 007032 Height of sensor above local ground (for wind measurement) */
   values[idx++].toBufr( "iw", 8 );    /* 002002 Type of instrumentation for wind measurement.( 8 = certified instrument m/s */
   values[idx++].toBufr( "Wind (time significance)", 2 );     /* 008021 Time significance (=2: time averaged) */
   values[idx++].toBufr( "Wind - Time periode", -10 );   /* 004025 Time period or displacement (minutes)*/
   values[idx++].toBufr( "dd", data.DD );    /* 011001 Wind direction */
   values[idx++].toBufr( "ff", data.FF );    /* 011002 Wind speed */
   values[idx++].toBufr( "Wind gust(time significance)", 2 );  /* 008021 Time significance */
   values[idx++].toBufrIf( "t_910ff[0]", -10, data.FG_010 != FLT_MAX );/* 004025 Time period or displacement (minutes) */
   values[idx++].toBufr( "ff910 Direction", data.DG_010 );  /* 011043 Maximum wind gust direction */
   values[idx++].toBufr( "ff910 Speed", data.FG_010 );/* 011041 Maximum wind gust speed */

   values[idx++].toBufr( "t_911ff", data.FgMax.t );/* 004025 Time period or displacement (minutes) */
   values[idx++].toBufr( "ff911 Direction", data.FgMax.dd );  /* 011043 Maximum wind gust direction */
   values[idx++].toBufr( "ff911 Speed", data.FgMax.ff ) ;/* 011041 Maximum wind gust speed */
   values[idx++].toBufr( "h_W", FLT_MAX ); /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */

   /* 3 02 044
    * Evaporation data */
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 002004 Type of instrumentation for evaporation measurement or crop type for evaporation */
   values[idx++] = miss;  /* 013033 Evaporation/evapotranspiration */

   /* 3 02 045
    * Radiation data (1 hour and 24 hour period) */
   values[idx++] = 0;
   kdata[iDelay++]=0;
#if 0
   values[idx++] = -1;    /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 014002 Long-wave radiation, integrated over period specified */
   values[idx++] = miss;  /* 014004 Short-wave radiation, integrated over period specified */
   values[idx++] = miss;  /* 014016 Net radiation, integrated over period specified */
   values[idx++] = miss;  /* 014028 Global solar radiation (high accuracy), integrated over period specified */
   values[idx++] = miss;  /* 014029 Diffuse solar radiation (high accuracy), integrated over period specified */
   values[idx++] = miss;  /* 014030 Direct solar radiation (high accuracy), integrated over period specified */
   values[idx++] = -24;   /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 014002 Long-wave radiation, integrated over period specified */
   values[idx++] = miss;  /* 014004 Short-wave radiation, integrated over period specified */
   values[idx++] = miss;  /* 014016 Net radiation, integrated over period specified */
   values[idx++] = miss;  /* 014028 Global solar radiation (high accuracy), integrated over period specified */
   values[idx++] = miss;  /* 014029 Diffuse solar radiation (high accuracy), integrated over period specified */
   values[idx++] = miss;  /* 014030 Direct solar radiation (high accuracy), integrated over period specified */
#endif

   /* 3 02 046
    * Temperature change (54g0sndT) */
   values[idx++] = 0;
   kdata[iDelay++]=0;
#if 0
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 012049 Temperature change over period specified */
#endif

   //Data elements only used by met.no.
   values[idx++].toBufr("FxMax when", data.FxMax.t );  //004025 Time period or displacement.
   values[idx++].toBufr("FxMax", data.FxMax.ff );      //011042 Maximum wind speed (10 min mean).

   LOGDEBUG( "Encodebufr name: " << station->name() );
   memset( cvals[0], ' ', 20 ); //Make sure the value is space padded.
   strncpy(cvals[0], station->name().c_str(), min( static_cast<int>(station->name().length()), 20 ) );
}

bool
isDirWithWritePermission( const std::string &path_, std::string &error )
{
   try {
      fs::path path( path_ );

      if( fs::exists( path ) &&
          fs::is_directory( path ) &&
          access( path_.c_str(), W_OK | X_OK ) > -1 )
         return true;
      else
         error = myStrerror( errno );
   }
   catch( const fs::filesystem_error &ex ) {
      error=ex.what();
   }

   return false;


}
std::string
myStrerror(int errnum )
{
   char buf[1024];
   char *p=0;
   std::ostringstream ost;

#ifdef _GNU_SOURCE
   p = strerror_r( errnum, buf, 1024 );
#else
   if( strerror_r( errnum, buf, 1024) == 0 )
      p = buf;
#endif

   if( p )
      ost << p;
   else
      ost << "errno: " << errnum;
   return ost.str();
}


}
