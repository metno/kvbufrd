/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: bufr.h,v 1.8.2.3 2007/09/27 09:02:23 paule Exp $

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
#ifndef __BUFR_H__
#define __BUFR_H__

#include <vector>
#include <string>
#include "BufrData.h"
#include "StationInfo.h"

#define GROUPSIZE                  6

/*
 * Quick and dirty fix to let the windspeed be in m/s, ie dont
 * convert to knop
 */

#define KNOPFAKTOR     1
#define IW 1             //Wind unit i m/s
//#define KNOPFAKTOR     1.94384449244
//#define IW 4  //Wind unit in knop
class Bufr
{
    Bufr(const Bufr &);
    Bufr& operator=(const Bufr &);

 public:
    enum EPrecipitation{NoPrecipitation,
                        PrecipitationRA, //akumulert nedbï¿½r   (Automatisk)
			PrecipitationRR, //Fra 1 times nedbï¿½r (Automatisk)
			PrecipitationRRR, //Fra manuell nedbï¿½r
			//PrecipitationRR_24 //Fra RR_24
			PrecipitationRR_N //Fra RR_1, RR_3, RR_6, RR_12 eller RR_24
    };

 protected:
    bool           debug;
    bool           test; //Used for unit testing
    std::string    errorMsg;
    EPrecipitation precipitationParam;

    int  Vis_ir_Kode(const std::string &str);
    bool Sjekk_Gruppe(int grpNr, std::string &kode, const std::string &str);
    void Tid_Kode(std::string &kode, int time);
    void windAtObstime( const BufrData &data, BufrData &res );
    void Naa_Vind_Kode(std::string &kode, float retn, float hast);
    void Temp_Kode(std::string &kode, float temp);
    

    /**
     * Nedboer_Kode, create the precipitation code
     * for 6RRRtr and 555 .... 4RtWdWdWd. It assumes that we shall create
     * a precipitation for data set.
     *
     * @param kode the 6RRRtr code on return.
     * @param vertilleggKode the 4RtWdWdWd on return.
     * @param totalNedboer the total precipitation for the period given by tr.
     * @param time the observation time to report the precipitation for.
     * @param tr the period the precipitation totalNedboer is acumulated for.
     * @param ir a value that give information about the percipitation in
     *        the tr period: 1 it has been precipitation, 3 No precipitation, 4
     *        no mearsument in the period.
     *
     * @see doNedboerKode
     */
    void precip(std::string &kode,           //RRRtr
		      std::string &vertilleggKode, //555 ... 4RtWdWdWd
		      std::string &sRR24Kode,          //333 ... 7RR24
		      float totalNedboer,
		      float fRR24,
		      int time,
		      int &tr,
		      int ir);
    void doPrecip(StationInfoPtr     info,
                  const BufrDataList &bufrData,
                  BufrData           &bufr );
    int  precipFromRA(float &nedbor, float &fRR24, int &tr, const BufrDataList &sd);
    int  precipFromRR(float &nedbor, float &fRR24, int &tr, const BufrDataList &sd);
    int  precipFromRrN(float &nedbor,
			float &fRR24, 
			int   &tr, 
			const BufrDataList &sd);
    int  precipFromRRRtr(float &nedbor,
			 float &fRR24, 
			 int   &tr, 
			 const BufrDataList &sd);
    int rr24FromRrN( const BufrDataList &sd, float &fRR24);
 	 
    void dewPoint( const BufrData &data, BufrData &res );
    void minMax(std::string &kode, BufrDataList &sd);
    void maxMin(std::string &kode, BufrDataList &sd);
    void maxWindGust(std::string &kode, BufrDataList &sd);
    void maxWindMax( BufrData::Wind &wind, BufrDataList &sd);
    float pressure( float presure );
    void doPressureTrend( const BufrDataList &data, BufrData &res );
    void computePressureTrend( const BufrDataList &data,
                               BufrData &res);
    bool pressureTrend( const BufrData &data, BufrData &res );
    void cloudCower( const BufrData &data, BufrData &res );
    void Hoyde_Sikt_Kode(std::string &kode, const BufrData &data);
    int  ix_Kode(const std::string &str);
    bool doGeneralWeather( BufrData &res, const BufrData &data);
    bool seaTemp( const BufrData &data, BufrData &res );
    bool SjekkEsss(std::string &kode, const std::string &str);
    void doEsss( std::string &kode, const BufrData &data );
    void GressTempKode(std::string &kode, BufrDataList &sd);
    void SplittStreng(std::string &streng, std::string::size_type index);

 public:
    
    
    Bufr(EPrecipitation precipitation);
    Bufr();

    ~Bufr();

    std::string getErrorMsg()const { return errorMsg;}
    void        setDebug(){ debug=true;}
    void        setTest( bool flag ) { test = flag; }
    
    /**
     * doBufr,
     *
     * \param create_CCA_template, angir at vi skal lage en template
     *        pï¿½ formen 'CCCXXX'  som senere skal skiftes
     *        ut med aktuell CCA verdi.
     */
    bool    doBufr( int                  bufrno,
                    const std::string    &usteder,
                    int                  listenummer,
                    std::string          &bufr,
                    StationInfoPtr       info,
                    BufrDataList         &bufrData,
                    bool                 create_CCA_template=false);

    bool doBufr( StationInfoPtr       info,
                 BufrDataList         &bufrData,
                 BufrData             &bufr,
                 bool                 create_CCA_template=false
               );

    /**
     * replaceCCCXXX erstatter CCCXXX templaten, hvis den finnes,
     * med verdien angitt med ccx. Hvis ccx er 0 skal vi bare fjerne
     * templaten, for ccx=1 får vi CCA, for ccx=2 får vi CCB osv. Hvis
     * ccx > 26, blir templaten bare fjernet.
     */
    static void replaceCCCXXX(std::string &bufr, int ccx);
};

#endif
