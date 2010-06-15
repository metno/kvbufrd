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


#include "bufrencodehelper.h"
#include <stdlib.h>
#include <sstream>

using namespace std;

namespace {

void
saveBufr( const StationInfoPtr station, int *buf, int buflen );

/**
 *
 * @exception BufrEncodeException
 * @param station
 * @param filename if thge filename is set use it. If not set generate a filename and return it on exit.
 * @return An open file descriptor on success.
 */
int
openBufrFile( const StationInfoPtr station, std::string &filename );

void
closeBufrFile( const std::string &filename, int fd );


bool
set_sec0134( const StationInfoPtr station, const DataElement &data,
             int *ksec0, int *ksec1, int *ksec3, int *ksec4 );

void set_values( const StationInfoPtr station,
                 const DataElement &data,
                 Values &values, char cvals[][80], int kdata[]);
}


void
encodeBufr( const DataElement &data, StationInfoPtr station )
{ /* pbopen variables */
  int fd;
  int error;
  string filename;

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
  int kbuflen;           /* length of bufr message (words) */
  int kbuff[MAX_BUFLEN/4]; /* integer array containing bufr message */

/* pbwrite variables */
  int nbytes;            /* number of bytes in bufr messages */

  char* outfile = "synop.bufr";


  /* Set input parameters to bufren */
  set_sec0134( station, data, ksec0, ksec1, ksec3, ksec4);
  set_values( station, data, values, cvals, kdata);

  ktdlen = 1;
  ktdlst[0] = 307080;

  /* Encode BUFR message */
  bufren_(ksec0, ksec1, ksec2, ksec3, ksec4, &ktdlen, ktdlst,
	  &kdlen, kdata, &kelem, &kvals, values.values(), (char **) cvals,
	  &kbuflen, kbuff, &error);

  if( error != 0 ) {
    printf("ERROR: bufren returned %d\n", error );
    exit(1);
  }
  
  saveBufr( station, kbuff, kbuflen );

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
                const DataElement &data,
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
   values[20] = station->heightTemperature();      /* 007032 Height of sensor above local ground (or deck of marine platform) */
   values[21].toBufr( "snTTT", data.TA );    /* 012101 Temperature/dry-bulb temperature */
   values[22].toBufr( "snTdTdTd", data.TD ); /* 012103 Dew-point temperature */
   values[23].toBufr( "UUU", data.UU );      /* 013003 Relative humidity */

   /* Visibility data */
   values[24] = station->heightVisability();      /* 007032 Height of sensor above local ground (for visibility measurement) */
   values[25].toBufr( "VV", data.VV );       /* 020001 Horizontal visibility */

   /* Precipitation past 24 hours */
   values[26] = station->heightPrecip();      /* 007032 Height of sensor above local ground (for precipitation measurement) */
   values[27].toBufr( "R24R24R24R24", data.RR_24 ); /* 013023 Total precipitation past 24 hours */
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
   values[idx++] = station->heightPrecip();   /* 007032 Height of sensor above local ground (for precipitation measurement) */
   if( obstime.hour() == 6 || obstime.hour()==18 ){
      values[idx++].toBufr( "tR[0]", -12 ); /* 004024 Time period or displacement (regional) */
      values[idx++].toBufr( "RRR[0]", data.RR_12 );/* 013011 Total precipitation/total water equivalent */
   } else {
      values[idx++].toBufr( "tR[0]", FLT_MAX ); /* 004024 Time period or displacement (regional) */
      values[idx++].toBufr( "RRR[0]", FLT_MAX );/* 013011 Total precipitation/total water equivalent */
   }

   if( data.RR_1 != FLT_MAX ) {
      values[idx++].toBufr( "tR[1]", -1 ); /* 004024 Time period or displacement (national) */
      values[idx++].toBufr( "RRR[1]", data.RR_1 );/* 013011 Total precipitation/total water equivalent */
   } else {
      values[idx++].toBufr( "tR[1]", FLT_MAX ); /* 004024 Time period or displacement (national) */
      values[idx++].toBufr( "RRR[1]", FLT_MAX );/* 013011 Total precipitation/total water equivalent */
   }

   /* Extreme temperature data */
   values[idx++] = station->heightTemperature();   /* 007032 Height of sensor above local ground (for temperature measurement) */
   values[idx++] = -12;   /* 004024 Time period or displacement */
   values[idx++] = 0;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "snTxTxTx", data.TAX_12 ); /* 012111 Maximum temperature, at height and over period specified */
   values[idx++] = -12;   /* 004024 Time period or displacement */
   values[idx++] = 0;     /* 004024 Time period or displacement */
   values[idx++].toBufr( "snTnTnTn", data.TAN_12 ); /* 012112 Minimum temperature, at height and over period specified */

   /* Wind data */
   values[idx++] = station->heightWind();   /* 007032 Height of sensor above local ground (for wind measurement) */
   values[idx++].toBufr( "iw", 8+4 );    /* 002002 Type of instrumentation for wind measurement */
   values[idx++].toBufr( "Wind (time significance)", 2 );     /* 008021 Time significance (=2: time averaged) */
   values[idx++].toBufr( "Wind - Time periode", -10 );   /* 004025 Time period or displacement (minutes)*/
   values[idx++].toBufr( "dd", data.DD );    /* 011001 Wind direction */
   values[idx++].toBufr( "ff", data.FF );    /* 011002 Wind speed */
   values[idx++] = miss;  /* 008021 Time significance */


   values[idx++].toBufr( "t_911ff[0]", FLT_MAX );/* 004025 Time period or displacement (minutes) */
   values[idx++] = miss;  /* 011043 Maximum wind gust direction */
   values[idx++].toBufr( "ff911[0]", FLT_MAX );/* 011041 Maximum wind gust speed */

   if( data.FG_1 != FLT_MAX ) {
      values[idx++].toBufr( "t_911ff[1]", -60 );/* 004025 Time period or displacement (minutes) */
      values[idx++] = miss;  /* 011043 Maximum wind gust direction */
      values[idx++].toBufr( "ff911[1]", data.FG_1 ) ;/* 011041 Maximum wind gust speed */
      values[idx++] = RVIND; /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */
   } else {
      values[idx++].toBufr( "t_911ff[1]", FLT_MAX );/* 004025 Time period or displacement (minutes) */
      values[idx++] = miss;  /* 011043 Maximum wind gust direction */
      values[idx++].toBufr( "ff911[1]", FLT_MAX ) ;/* 011041 Maximum wind gust speed */
      values[idx++] = RVIND; /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */
   }
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


/* This subroutine is merely initializing data. Should be replaced
 * with fetching data from Kvalobs.
 *
 * Note about names of variables: When a variable corresponds to a
 * parameter in synop FM 12, I have used the synop symbol for that
 * variable, except that the value is hard coded to missing value for
 * several parameters not currently used in Norwegian observations. In
 * addition, I have introduced symbols h_... for 007031 'Height of
 * sensor above local ground' (e.g. h_T for temperature), lat for
 * latitude, lon for longitude, ha for height of station, vsc for
 * vertical significance clouds, vsci[] for vertical significance
 * individual clouds, num_layers for number of cloud layers, EE for E
 * and E' (combined to one variable in BUFR), t_ww for the time period
 * past weather (W1 and W2) covers, t_911ff[] for the time period for
 * max wind gust
 */
#if 0
void get_data(double *Year, double *Month, double *YY, double *GG, double *gg,
	      double *II, double *iii, double *ix, double *lat, double *lon, double *ha,
	      double *hP, double *h_T, double *h_V, double *h_R, double *h_W,
	      double *P0P0P0P0, double *PPPP, double *ppp, double *a,
	      double *p24p24p24, double *a3, double *hhh,
	      double *snTTT, double *snTdTdTd, double *UUU, double *VV,
	      double *N, double *vsc, double *Nh, double *h, double *CL, double *CM, double *CH,
	      double *num_layers, double vsci[], double Ns[], double C[], double hshs[],
	      double *EE, double *sss, double *snTgTg,
	      double *ww, double *t_ww, double *W1, double *W2,
	      double *SS, double *SSS,
	      double *R24R24R24R24, double tR[], double RRR[],
	      double *snTxTxTx, double *snTnTnTn,
	      double *iw, double *dd, double *ff, double t_911ff[], double ff911[],
	      char name[]) {

  *Year = 2007;
  *Month = 10;
  *YY = 2;
  *GG = 6;
  *gg = 20;
  *II = 1;
  *iii = 492;
  *ix = 2;
  *lat = 59.9427;
  *lon = 10.7207;
  *ha = 94;
  *hP = 96;
  *h_T = 2;
  *h_V = RVIND;
  *h_R = RVIND;
  *h_W = 10;
  *P0P0P0P0 = 1012.2 * 100; /* 173=PO? (Pa)*/
  *PPPP = 1024.1 * 100; /* 178=PR? (Pa) */
  *ppp = 0.5 * 100; /* 177=PP? (Pa) */
  *a = RVIND; /* What is 'a' in Kvalobs? */
  *p24p24p24 = RVIND;
  *a3 = RVIND;
  *hhh = RVIND;
  *snTTT = -2.2 + 273.15; /* (K) 211=TA? */
  *snTdTdTd = RVIND; /* 217=TD? */
  *UUU = 90; /* (%) 262=UU? */
  *VV = 65000; /* (m) 273=VV? */
  *R24R24R24R24 = 0; /* (kg/m2) 110=RR_24? */
  *N = 40; /* (%) 15=NN? Note: NN(=3 here) is in octas */
           /* NN = 9 shall be coded in BUFR as 113 % */
  *vsc = 7; /* See B/C 1.4.4.2 */
  *Nh = 10; /* (%) 14=NH? Note: NH(=1 here) is in octas */
  *h = 1500; /* (m) 55=HL? */
  *CL = 5 + 30; /* 23=CL? */
  *CM = 7 + 20; /* 24=CM? */
  *CH = 8 + 10; /* 22=CH? */
  *num_layers = 1; /* Looks like no other formats than synop (typeid=1)
		   * reports individual cloud layers (333 8NsChshs in synop) */
  vsci[0] = RVIND; 
  Ns[0] = RVIND; /* 25=NS1? */
  C[0] = RVIND; /* 305=CC1? */
  hshs[0] = RVIND; /* 301=HS1? */
  *EE = 31; /* 7=EM && SD=18? 
            * 31 is missing value. How do we decode EM/SD=-1? */
  *sss = RVIND; /* SA=112? */
  *snTgTg = RVIND; /* TG=221? */
  *ww = 2; /* WW=41 && WAWA=49? Note: ww=WW but ww=WAWA+100 */
  *t_ww = -6; /* Period in hours since last main synoptic hour,
	      * = -6 for termin 0,6,12,18
	      * = -3 for termin 3,9,15,21 */
  *W1 = 2; /* W1=42 && WA1=47? Note: W1 (BUFR) = WA1 + 10 */
  *W2 = 0; /* W2=43 && WA2=48? Note: W2 (BUFR) = WA2 + 10 */
  *SS = RVIND; /* OT_1=121? */
  *SSS = RVIND; /* OT_24=122? */
  tR[0] = -12; /* Regional decision (-12 for 6,18; RVIND for other termins?)*/
  RRR[0] = 0; /* RR_12=109? Set to RVIND for termin not 6,18?*/
  tR[1] = -1; /* National decision. Should we use RVIND here?*/
  RRR[1] = RVIND; /* Blindern reports RA, so we could calculate R_1... */
  *snTxTxTx = 8.3 + 273.15; /* (K) TAX_12=216? */
  *snTnTnTn = 4.7 + 273.15; /* (K) TAN_12=214? */
  *iw = 8 + 4; /* 8 = Bit 1 set: 'Certified instruments'
		* 4 = Bit 2 set: 'Originally measured in knots' */
  *dd = 59; /* DD=61? (degree) */
  *ff = 0.8; /* (m/s) FF=81? */
  t_911ff[0] = -10; 
  ff911[0] = 1.2; /* (m/s) FG_010=84? */
  t_911ff[1] = -60;
  ff911[1] = 1.3; /* (m/s) FG_1=90? */
  
  strcpy(name, "OSLO - BLINDERN");
} 
#endif

void
saveBufr( const StationInfoPtr station, int *buf, int buflen )
{

}


int
openBufrFile( const StationInfoPtr station )
{
   int error;
   int fd;
   string filename;

    /* Open bufr file - for write */
    pbopen_( &fd, const_cast<char*>(filename.c_str()), "w", &error, filename.length(), 2 );
    if ( error != 0 ) {
       ostringstream o;

       o << "Bufr: " << "Cant open file <" << filename << ">. Errorkode: " << error;
      throw BufrEncodeException( o.str() );
      return false;
    }

}

void
closeBufrFile( const std::string &filename, int fd )
{
   int error;
   pbclose_( &fd, &error);

   if( error != 0 ) {
      ostringstream o;

      o << "Bufr: " << "Cant close file <" << filename << ">. Errorkode: " << error;
      throw BufrEncodeException( o.str() );
   }
}


}
