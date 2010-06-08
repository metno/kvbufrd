/* PS 2007-10-06
 *
 * Usage: encodebufr
 *
 * Example program in C to show how a SYNOP may be encoded into BUFR using
 * WMO template 307080. Calling Fortran library libbufr. 
 *
 * Note: ought to be rewritten using WMO template 307079
 *
 * Output: BUFR file containing the encoded BUFR message, named 'synop.bufr'
 */

#include <stdlib.h>

#define KELEM 4000    /* Max number of elements in values array. */
#define KVALS 20000   /* Max number of elements in CVALS array.
                       * Note that 200000 (as used in example Fortran programs 
                       * from ECMWF) is too big; results in segmentation
		       * violation for cvals */
#define KDLEN 200     /* Max number of elements in kdata array */
#define MAX_BUFLEN 200000 /* Max lenght (in bytes) of bufr message */
#define RVIND 1.7E38  /* 'Missing' value */
#define DEBUG 0

extern void pbopen_(int*,char*,char*,int*,int,int);
extern void pbclose_(int*,int*);
extern void pbwrite_(int* ounit, int* kbuff, int* nbytes, int* kerr);
extern void bufren_(int* ksec0, int* ksec1, int* ksec2, int* ksec3, int* ksec4,
		    int* ktdlen, int* ktdlst, int* kdlen, int* kdata,
                    int* kelem, int* kvals, double* values, char** cvals,
		    int* kbufl, int* kbuff, int* kerr);
void set_sec0134(int* ksec0, int* ksec1, int* ksec3, int* ksec4);
void set_values(double values[], char cvals[][80], int kdata[]);
void get_termin(int *year, int *month, int *day,
		int *hour, int *minute, int *second);
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
	      char name[]);


int main(int argc, char *argv[]) {
/* pbopen variables */
  int ounit;
  int kerr;

/* bufren variables */
  int kelem = KELEM, kvals = KVALS, kdlen = KDLEN;
  int ksec0[3], ksec1[40], ksec2[64], ksec3[4], ksec4[2];
  int kdata[KDLEN];      /* integer array containing data needed for data
		          * descriptor expansion (delayed replication factors) */
  double values[KVALS];  /* expanded data values */
  char cvals[KVALS][80]; /* String values - index to which are extracted
                          * from values array */
  int ktdlen;            /* number of data descriptors in section 3 */
  int ktdlst[KELEM];     /* array containing data descriptors in section 3 */
  int kbuflen;           /* length of bufr message (words) */
  int kbuff[MAX_BUFLEN/4]; /* integer array containing bufr message */

/* pbwrite variables */
  int nbytes;            /* number of bytes in bufr messages */

  char* outfile = "synop.bufr";

  /* Open bufr file - for write */
  pbopen_(&ounit, outfile, "w", &kerr, strlen(outfile), 2);
  if (kerr != 0) {
    printf("ERROR: pbopen returned %d\n",kerr);
    exit(1);
  }

  /* Set input parameters to bufren */
  set_sec0134(ksec0, ksec1, ksec3, ksec4);
  set_values(values, cvals, kdata);

  ktdlen = 1;
  ktdlst[0] = 307080;

  /* Encode BUFR message */
  bufren_(ksec0, ksec1, ksec2, ksec3, ksec4, &ktdlen, ktdlst,
	  &kdlen, kdata, &kelem, &kvals, values, (char **) cvals,
	  &kbuflen, kbuff, &kerr);
  if (kerr != 0) {
    printf("ERROR: bufren returned %d\n", kerr);
    exit(1);
  }
  
  /* Write BUFR message to file */
  nbytes = kbuflen*4;
  pbwrite_(&ounit, kbuff, &nbytes, &kerr);
  if (kerr < 0) {
    printf("ERROR: pbwrite returned %d\n", kerr);
    exit(1);
  }

  /* Close bufr file */
  pbclose_(&ounit, &kerr);
  if (kerr != 0) {
    printf("ERROR: pbclose returned %d\n",kerr);
    exit(1);
  }
}


void set_sec0134(int* ksec0, int* ksec1, int* ksec3, int* ksec4) {
  int year, month, day, hour, minute, second; /* Termin time */ 

  get_termin(&year,&month,&day,&hour,&minute,&second);

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
  ksec1[ 8] = year;
  ksec1[ 9] = month;
  ksec1[10] = day;
  ksec1[11] = hour;
  ksec1[12] = minute;
  ksec1[13] = 0;       /* BUFR master table */
  ksec1[14] = 13;      /* Version number of master table used */
  ksec1[15] = 0;       /* Originating sub-centre */
  ksec1[16] = 0;       /* International sub-category (see common table C-13) */
  ksec1[17] = second;

  ksec3[0] = 0;        /* Length of section 3 (bytes), will be set by bufren */ 
  ksec3[2] = 1;        /* Number of subsets */ 
  ksec3[3] = 128;      /* Flag (128 = observation data && no data compression) */ 

  ksec4[0] = 0;        /* Length of section 4 (bytes), will be set by bufren */ 
}

/* Populate values and cvals with values according to WMO BUFR template 307080 */
void set_values(double values[], char cvals[][80], int kdata[]) {
  double miss = RVIND;
  int idx, i;

  double Year, Month, YY, GG, gg;   /* Observation time */
  double II, iii, ix, lat, lon, ha; /* Static main station data */
  double hP, h_T, h_V, h_R, h_W;    /* Height of sensors */
  double P0P0P0P0, PPPP, ppp, a, p24p24p24, a3, hhh; /* Pressure data */
  double snTTT, snTdTdTd, UUU;      /* Temperature and humidity data */
  double VV;                        /* Visibility data */
  double N, vsc, Nh, h, CL, CM, CH; /* Cloud data in 302004 */
  double num_layers, vsci[4], Ns[4], C[4], hshs[4]; /* Cloud data in 302005 (individual cloud layers) */
  double EE, sss, snTgTg;           /* Ground info */
  double ww, t_ww, W1, W2;          /* Present and past weather */
  double SS, SSS;                   /* Sunshine data */
  double R24R24R24R24, tR[2], RRR[2]; /* Precipitation data */
  double snTxTxTx, snTnTnTn;        /* Extreme temperature data */
  double iw, dd, ff, t_911ff[2], ff911[2]; /* Wind data */
  char name[20];

  get_data(&Year, &Month, &YY, &GG, &gg,
	   &II, &iii, &ix, &lat, &lon, &ha, &hP, &h_T, &h_V, &h_R, &h_W,
	   &P0P0P0P0, &PPPP, &ppp, &a, &p24p24p24, &a3, &hhh,
	   &snTTT, &snTdTdTd, &UUU, &VV,
	   &N, &vsc, &Nh, &h, &CL, &CM, &CH, &num_layers, vsci, Ns, C, hshs,
	   &EE, &sss, &snTgTg, &ww, &t_ww, &W1, &W2, &SS, &SSS,
	   &R24R24R24R24, tR, RRR, &snTxTxTx, &snTnTnTn,
	   &iw, &dd, &ff, t_911ff, ff911,
	   name);

  /* Fixed surface station identification, time, horizontal and vertical coordinates */
  values[0] = II;        /* 001001 WMO block number */
  values[1] = iii;       /* 001002 WMO station number */
  values[2] = 1020;      /* Pointer to cvals (001015 Station or site name) */ 
  values[3] = ix;        /* 002001 Type of station */
  values[4] = Year;      /* 004001 Year */
  values[5] = Month;     /* 004002 Month */
  values[6] = YY;        /* 004003 Day */
  values[7] = GG;        /* 004004 Hour */
  values[8] = gg;        /* 004005 Minute */
  values[9] = lat;       /* 005001 Latitude (high accuracy) */
  values[10] = lon;      /* 006001 Longitude (high accuracy) */
  values[11] = ha;       /* 007030 Height of station ground above mean sea level */

  /* Basic synoptic "instantaneous data" */

  /* Pressure data */
  values[12] = hP;       /* 007031 Height of barometer above mean sea level */
  values[13] = P0P0P0P0; /* 010004 Pressure */
  values[14] = PPPP;     /* 010051 Pressure reduced to mean sea level */
  values[15] = ppp;      /* 010061 3-hour pressure change */
  values[16] = a;        /* 010063 Characteristic of pressure tendency */
  values[17] = p24p24p24;/* 010062 24-hour pressure change */

  values[18] = a3;       /* 007004 Pressure (standard level) */
  values[19] = hhh;      /* 010009 Geopotential height */

  /* Temperature and humidity data */
  values[20] = h_T;      /* 007032 Height of sensor above local ground (or deck of marine platform) */
  values[21] = snTTT;    /* 012101 Temperature/dry-bulb temperature */
  values[22] = snTdTdTd; /* 012103 Dew-point temperature */
  values[23] = UUU;      /* 013003 Relative humidity */

  /* Visibility data */
  values[24] = h_V;      /* 007032 Height of sensor above local ground (for visibility measurement) */
  values[25] = VV;       /* 020001 Horizontal visibility */

  /* Precipitation past 24 hours */
  values[26] = h_R;      /* 007032 Height of sensor above local ground (for precipitation measurement) */
  values[27] = R24R24R24R24; /* 013023 Total precipitation past 24 hours */
  values[28] = RVIND;    /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */

  /* Cloud data */
  values[29] = N;        /* 020010 Cloud cover (total) */
  values[30] = vsc;      /* 008002 Vertical significance (surface observations) */
  values[31] = Nh;       /* 020011 Cloud amount (of low or middle clouds) */
  values[32] = h;        /* 020013 Height of base of cloud */
  values[33] = CL;       /* 020012 Cloud type (low clouds CL) */
  values[34] = CM;       /* 020012 Cloud type (middle clouds CM) */
  values[35] = CH;       /* 020012 Cloud type (high clouds CH) */

  /* Individual cloud layers or masses */
  /* Looks like Norwegian stations do not report these parameters, except for
   * those sending synop. So the loop below can probably be simplified */
  values[36] = num_layers; /* 031001 Delayed descriptor replication factor */
  idx = 37;
  for (i=0; i < num_layers; i++) {
    values[idx++] = vsci[i];/* 008002 Vertical significance (surface observations) */
    values[idx++] = Ns[i];  /* 020011 Cloud amount */
    values[idx++] = C[i];   /* 020012 Cloud type */
    values[idx++] = hshs[i];/* 020013 Height of base of cloud */
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
  values[idx++] = EE;    /* 020062 State of the ground (with or without snow) */
  values[idx++] = sss;   /* 013013 Total snow depth */
  values[idx++] = snTgTg;/* 012113 Ground minimum temperature, past 12 hours */

  /* Basic synoptic "period data" */

  /* Present and past weather */
  values[idx++] = ww;    /* 020003 Present weather (see note 1) */
  values[idx++] = t_ww;  /* 004024 Time period or displacement (for W1 and W2) */
  values[idx++] = W1;    /* 020004  Past weather (1) (see note 2) */
  values[idx++] = W2;    /* 020005  Past weather (2) (see note 2) */

  /* Sunshine data (1 hour and 24 hour period) */
  values[idx++] = 1;     /* 004024 Time period or displacement */
  values[idx++] = SS;    /* 014031 Total sunshine */
  values[idx++] = 24;    /* 004024 Time period or displacement */
  values[idx++] = SSS;   /* 014031 Total sunshine */

  /* Precipitation measurement */
  values[idx++] = h_R;   /* 007032 Height of sensor above local ground (for precipitation measurement) */
  values[idx++] = tR[0]; /* 004024 Time period or displacement (regional) */
  values[idx++] = RRR[0];/* 013011 Total precipitation/total water equivalent */
  values[idx++] = tR[1]; /* 004024 Time period or displacement (national) */
  values[idx++] = RRR[1];/* 013011 Total precipitation/total water equivalent */

  /* Extreme temperature data */
  values[idx++] = h_T;   /* 007032 Height of sensor above local ground (for temperature measurement) */
  values[idx++] = -12;   /* 004024 Time period or displacement */
  values[idx++] = 0;     /* 004024 Time period or displacement */
  values[idx++] = snTxTxTx; /* 012111 Maximum temperature, at height and over period specified */
  values[idx++] = -12;   /* 004024 Time period or displacement */
  values[idx++] = 0;     /* 004024 Time period or displacement */
  values[idx++] = snTnTnTn; /* 012112 Minimum temperature, at height and over period specified */

  /* Wind data */
  values[idx++] = h_W;   /* 007032 Height of sensor above local ground (for wind measurement) */
  values[idx++] = iw;    /* 002002 Type of instrumentation for wind measurement */
  values[idx++] = 2;     /* 008021 Time significance (=2: time averaged) */
  values[idx++] = -10;   /* 004025 Time period or displacement (minutes)*/
  values[idx++] = dd;    /* 011001 Wind direction */
  values[idx++] = ff;    /* 011002 Wind speed */
  values[idx++] = miss;  /* 008021 Time significance */
  values[idx++] = t_911ff[0];/* 004025 Time period or displacement (minutes) */
  values[idx++] = miss;  /* 011043 Maximum wind gust direction */
  values[idx++] = ff911[0];/* 011041 Maximum wind gust speed */
  values[idx++] = t_911ff[1];/* 004025 Time period or displacement (minutes) */
  values[idx++] = miss;  /* 011043 Maximum wind gust direction */
  values[idx++] = ff911[1];/* 011041 Maximum wind gust speed */
  values[idx++] = RVIND; /* 007032 Height of sensor above local ground (set to missing to cancel the previous value) */

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

  strcpy(cvals[0], name);/* Station or site name */

  /* Delayed replication factors */
  kdata[0] = num_layers; /* Number of cloud layers */
  kdata[1] = 1;          /* Number of cloud layers with bases below station level */

  if (DEBUG > 0) {
    printf("VALUES\n");
    for (i=0; i < idx; i++) {
      printf("  %d  %g\n",i,values[i]);
    }
  }
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

void get_termin(int *year, int *month, int *day,
		int *hour, int *minute, int *second) {
  *year = 2007;
  *month = 10;
  *day = 2;
  *hour = 6;
  *minute = 0;
  *second = 0;
}


