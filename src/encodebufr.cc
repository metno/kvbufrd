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
#include <errno.h>
#include <milog/milog.h>
#include "bufrencodehelper.h"
#include <stdlib.h>
#include <sstream>
#include <fstream>

using namespace std;

namespace {
bool
set_sec0134( const StationInfoPtr station, const DataElement &data,
             int *ksec0, int *ksec1, int *ksec3, int *ksec4 );

void set_values( const StationInfoPtr station,
                 const BufrData &data,
                 Values &values, char cvals[][80], int kdata[]);
}



BufrEncoder::
BufrEncoder(StationInfoPtr station_)
   : station( station_ ), kbuff( 0 )
{

};


void
BufrEncoder::
encodeBufr( const BufrData &data )
{ /* pbopen variables */
  int fd;
  int error;
  string filename;

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
        kbuff = new int[MAX_BUFLEN /4];
     }
     catch( ... ){
        throw BufrEncodeException( "NOMEM to allocate a buffer to hold the bufr message.");
     }
  }

  /* Set input parameters to bufren */
  set_sec0134( station, data, ksec0, ksec1, ksec3, ksec4);
  set_values( station, data, values, cvals, kdata);

  ktdlen = 1;
  ktdlst[0] = 307080;

  /* Encode BUFR message */
  bufren_( ksec0, ksec1, ksec2, ksec3, ksec4, &ktdlen, ktdlst,
           &kdlen, kdata, &kelem, &kvals, values.values(), (char **) cvals,
           &kbuflen, kbuff, &error );

  if( error != 0 ) {
     ostringstream o;
     o << "Failed to encode bufr for station '" << station->wmono() << "' obstime: " << data.time()
       << ". bufren error code: " << error << endl << values.log();
     delete[] kbuff;
     kbuff = 0;
     throw BufrEncodeException( o.str() );
  }
}

void
BufrEncoder::
saveToFile()const
{
   if( ! kbuff ) {
      ostringstream o;
      o << "Missing bufr encoding for station '" << station->wmono() << "' obstime: " << obstime;
      throw BufrEncodeException( o.str() );
   }

   if( station->copy() ) {
      saveToFile( station->copyto(), false );
   }
}


void
BufrEncoder::
saveToFile( const std::string &path, bool overwrite )const
{
   ostringstream ost;
   ofstream      f;
   struct stat   sbuf;

   if(stat( path.c_str(), &sbuf)<0){
      ostringstream o;
      o << "Save to: '" << path << "'. ";
      if(errno==ENOENT || errno==ENOTDIR){
         o << "Invalid path!";
      }else if(errno==EACCES){
         o << "Permission denied!";
      }else{
         o << "stat unknown error!";
      }

      throw BufrEncodeException( o.str() );
   }

   if(!S_ISDIR(sbuf.st_mode)){
      ostringstream o;
      o << "Saveto: <" << path << "> not a directory!";
      throw BufrEncodeException( o.str() );
   }

   for( int i = 0; i<100; ++i ) {
      ost.str("");
      if( i == 0 ) {
         ost << path << "/" << station->wmono() << "-"
               << setfill('0') << setw(2) << obstime.day() << setw(2)
               << obstime.hour()
               << ".bufr";
      }else{
         ost << path << "/" <<  station->wmono() << "-"
               << setfill('0') << setw(2) << obstime.day() << setw(2)
               << obstime.hour() << "-" << setfill('0') << setw(2) << i
               << ".bufr";
      }

      if( stat( ost.str().c_str(), &sbuf) < 0 ) {
         if( !(errno == ENOENT || errno == ENOTDIR) ) {
            ostringstream o;
            o << "Unexpected error from stat: errno: " << errno ;
            throw BufrEncodeException( o.str() );
         }
      }

      f.open( ost.str().c_str(), ios_base::trunc | ios_base::binary | ios_base::out );

      if( f.is_open() ){
         LOGINFO("Writing BUFR to file: " << ost.str());
         f.write( reinterpret_cast< ofstream::char_type* >( kbuff ), kbuflen*4 );
         f.close();
         return;
      }
   }
   ost.str("");
   ost << "Failed to write BUFR file for station '" << station->wmono() << "' obstime: " << obstime;
   throw BufrEncodeException( ost.str() );
}

namespace {

bool
set_sec0134( const StationInfoPtr station,
             const DataElement &data,
             int *ksec0, int *ksec1, int *ksec3, int *ksec4)
{
  int year, month, day, hour, minute, second; /* Termin time */ 
  miutil::miTime obsTime=data.time();

  if( obsTime.undef() )
     return false;

  ksec0[0] = 0;        /* Length of section 0, will be set by bufren */
  ksec0[1] = 0;        /* Total length of BUFR message, will be set by bufren */
  ksec0[2] = 4;        /* BUFR edition number */

  ksec1[ 0] = 22;      /* Length of section 1 (bytes). Must be set by user */ 
  ksec1[ 1] = 4;       /* BUFR edition number */
  ksec1[ 2] = 88;      /* Originating centre */
  ksec1[ 3] = 0;       /* Update sequence number */
  ksec1[ 4] = 0;       /* Flag (presence of section 2) */
  ksec1[ 5] = 0;       /* BUFR message type */
  ksec1[ 6] = 0;       /* BUFR message subtype */
  ksec1[ 7] = 0;       /* Version number of local table used */
  ksec1[ 8] = obsTime.hour();
  ksec1[ 9] = obsTime.month();
  ksec1[10] = obsTime.day();
  ksec1[11] = obsTime.hour();
  ksec1[12] = obsTime.min();
  ksec1[13] = 0;       /* BUFR master table */
  ksec1[14] = 13;      /* Version number of master table used */
  ksec1[15] = 0;       /* Originating sub-centre */
  ksec1[16] = 0;       /* International sub-category (see common table C-13) */
  ksec1[17] = obsTime.sec();

  ksec3[0] = 0;        /* Length of section 3 (bytes), will be set by bufren */ 
  ksec3[2] = 1;        /* Number of subsets */ 
  ksec3[3] = 128;      /* Flag (128 = observation data && no data compression) */ 

  ksec4[0] = 0;        /* Length of section 4 (bytes), will be set by bufren */ 
}

/* Populate values and cvals with values according to WMO BUFR template 307080 */
void set_values(const StationInfoPtr station,
                const BufrData       &data,
                Values &values, char cvals[][80], int kdata[])
{
   double miss = RVIND;
   int idx, i;

   float t_ww;          /* Present and past weather */
   miutil::miTime obstime = data.time();


   if( (obstime.hour()%6) == 0 )
      t_ww = -6;
   else if( (obstime.hour()%3) == 0 )
      t_ww = -3;
   else
      t_ww = FLT_MAX;

   /* Fixed surface station identification, time, horizontal and vertical coordinates */
   values[0].toBufr( "II", static_cast<int>( station->wmono()/1000 ) ) ;        /* 001001 WMO block number  II*/
   values[1].toBufr( "iii", static_cast<int>( station->wmono()%1000 ) );        /* 001002 WMO station number  iii*/
   values[2] = 1020;      /* Pointer to cvals (001015 Station or site name) */
   values[3].toBufr( "ix", data.IX );       /* 002001 Type of station ix*/
   values[4].toBufr( "Year", obstime.year() );      /* 004001 Year */
   values[5].toBufr( "Month", obstime.month() );     /* 004002 Month */
   values[6].toBufr( "YY", obstime.day() );        /* 004003 Day */
   values[7].toBufr( "GG", obstime.hour() );        /* 004004 Hour */
   values[8].toBufr( "gg", obstime.min() );;        /* 004005 Minute */
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
   values[30].toBufr( "vsc", 7 );      /* 008002 Vertical significance (surface observations) */
   values[31].toBufr( "Nh", data.NH );       /* 020011 Cloud amount (of low or middle clouds) */
   values[32].toBufr( "h", data.HL );        /* 020013 Height of base of cloud */
   values[33].toBufr( "CL", data.CL );       /* 020012 Cloud type (low clouds CL) */
   values[34].toBufr( "CM", data.CM );       /* 020012 Cloud type (middle clouds CM) */
   values[35].toBufr( "CH", data.CH );       /* 020012 Cloud type (high clouds CH) */

   /* Individual cloud layers or masses */
   /* Looks like Norwegian stations do not report these parameters, except for
    * those sending synop. So the loop below can probably be simplified */
   values[36].toBufr( "num-cloud-layers", static_cast<int>(data.cloudExtra.size()) ); /* 031001 Delayed descriptor replication factor */
   idx = 37;
   for (i=0; i < data.cloudExtra.size(); i++) {
      values[idx++].toBufr( "vsci[i]", data.cloudExtra[i].vsci );/* 008002 Vertical significance (surface observations) */
      values[idx++].toBufr( "Ns[i]", data.cloudExtra[i].Ns );  /* 020011 Cloud amount */
      values[idx++].toBufr( "C[i]", data.cloudExtra[i].C );   /* 020012 Cloud type */
      values[idx++].toBufr( "hshs[i]", data.cloudExtra[i].hshs );/* 020013 Height of base of cloud */
   }

   /* Clouds with bases below station level */
   /* These are not reported for Norwegian observations. Are we allowed to set
    * delayed replication to 0? */
   values[idx++] = 1;     /* 031001 Delayed descriptor replication factor */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020011 Cloud amount (N') */
   values[idx++] = miss;  /* 020012 Cloud type (C') */
   values[idx++] = miss;  /* 020014 Height of top of cloud (H'H') */
   values[idx++] = miss;  /* 020014 Height of top of cloud (Ct) */

   /* Direction of cloud drift (56DLDMDH) */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
   values[idx++] = miss;  /* 008002 Vertical significance (surface observations) */
   values[idx++] = miss;  /* 020054 True direction from which clouds are moving */
   values[idx++] = RVIND; /* 008002 Vertical significance, set to missing to cancel the previous value */

   /* Direction and elevation of clouds (57CDaec) */
   values[idx++] = miss;  /* 005021 Bearing or azimuth */
   values[idx++] = miss;  /* 007021 Elevation (see note 2) */
   values[idx++] = miss;  /* 020012 Cloud type */
   values[idx++] = RVIND; /* 005021 Bearing or azimuth, set to missing to cancel the previous value */
   values[idx++] = RVIND; /* 007021 Elevation, set to missing to cancel the previous value */

   /* State of ground, snow depth, ground minimum temperature */
   values[idx++].toBufr( "EE", data.EM );    /* 020062 State of the ground (with or without snow) */
   values[idx++].toBufr( "sss", data.SA );   /* 013013 Total snow depth */
   values[idx++].toBufr( "snTgTg", data.TGN );/* 012113 Ground minimum temperature, past 12 hours */

   /* Basic synoptic "period data" */

   /* Present and past weather */
   values[idx++].toBufr( "ww", data.ww );    /* 020003 Present weather (see note 1) */
   values[idx++].toBufr( "t_ww", t_ww );  /* 004024 Time period or displacement (for W1 and W2) */
   values[idx++].toBufr( "W1", data.W1 );    /* 020004  Past weather (1) (see note 2) */
   values[idx++].toBufr( "W2", data.W2 );    /* 020005  Past weather (2) (see note 2) */

   /* Sunshine data (1 hour and 24 hour period) */
   values[idx++] = 1;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "SS", FLT_MAX );    /* 014031 Total sunshine OT_1 */
   values[idx++] = 24;    /* 004024 Time period or displacement */
   values[idx++].toBufr( "SSS", FLT_MAX );   /* 014031 Total sunshine OT_24*/

   /* Precipitation measurement */
   values[idx++].toBufr( "h_P", station->heightPrecip() );   /* 007032 Height of sensor above local ground (for precipitation measurement) */
   values[idx++].toBufr( "tR[0] (Regional)", data.precipRegional.hTr ); /* 004024 Time period or displacement (regional) */
   values[idx++].toBufr( "RRR[0] (Regional)", data.precipRegional.RR );/* 013011 Total precipitation/total water equivalent */

   values[idx++].toBufr( "tR[1] (National)",  data.precipNational.hTr ); /* 004024 Time period or displacement (national) */
   values[idx++].toBufr( "RRR[1] (National)", data.precipNational.RR );/* 013011 Total precipitation/total water equivalent */

   /* Extreme temperature data */
   values[idx++].toBufr( "h_T",  station->heightTemperature() );   /* 007032 Height of sensor above local ground (for temperature measurement) */
   values[idx++].toBufrIf("t_TxTxTx", -12, data.TAX_12 != FLT_MAX );   /* 004024 Time period or displacement */
   values[idx++] = 0;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "snTxTxTx", data.TAX_12 ); /* 012111 Maximum temperature, at height and over period specified */
   values[idx++].toBufrIf("t_TnTnTn", -12, data.TAN_12 != FLT_MAX );   /* 004024 Time period or displacement */
   values[idx++] = 0;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "snTnTnTn", data.TAN_12 ); /* 012112 Minimum temperature, at height and over period specified */

   /* Wind data */
   values[idx++].toBufr( "h_W", station->heightWind() );   /* 007032 Height of sensor above local ground (for wind measurement) */
   values[idx++].toBufr( "iw", 8+4 );    /* 002002 Type of instrumentation for wind measurement */
   values[idx++].toBufr( "Wind (time significance)", 2 );     /* 008021 Time significance (=2: time averaged) */
   values[idx++].toBufr( "Wind - Time periode", -10 );   /* 004025 Time period or displacement (minutes)*/
   values[idx++].toBufr( "dd", data.DD );    /* 011001 Wind direction */
   values[idx++].toBufr( "ff", data.FF );    /* 011002 Wind speed */
   values[idx++] = miss;  /* 008021 Time significance */


   values[idx++].toBufrIf( "t_910ff[0]", -10, data.FG_010 != FLT_MAX );/* 004025 Time period or displacement (minutes) */
   values[idx++] = miss;  /* 011043 Maximum wind gust direction */
   values[idx++].toBufr( "ff910[0]", data.FG_010 );/* 011041 Maximum wind gust speed */

   values[idx++].toBufr( "t_911ff[1]", data.tFG );/* 004025 Time period or displacement (minutes) */
   values[idx++] = miss;  /* 011043 Maximum wind gust direction */
   values[idx++].toBufr( "ff911[1]", data.FG ) ;/* 011041 Maximum wind gust speed */
   values[idx++].toBufr( "h_W", station->heightWind() ); /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */

   /* Evaporation data */
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 002004 Type of instrumentation for evaporation measurement or crop type for evaporation */
   values[idx++] = miss;  /* 013033 Evaporation/evapotranspiration */

   /* Radiation data (1 hour and 24 hour period) */
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

   /* Temperature change (54g0sndT) */
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 004024 Time period or displacement */
   values[idx++] = miss;  /* 012049 Temperature change over period specified */

   strcpy(cvals[0], station->name().c_str() );/* Station or site name */

   /* Delayed replication factors */
   kdata[0] = static_cast<int>(data.cloudExtra.size()); /* Number of cloud layers */
   kdata[1] = 1;          /* Number of cloud layers with bases below station level */
}



}
