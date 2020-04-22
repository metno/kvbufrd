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
#include <memory>
#include "boost/date_time/posix_time/ptime.hpp"
#include "BufrData.h"
#include "StationInfo.h"
#include "bufr/BufrHelper.h"

#define GROUPSIZE                  6

class EncodeBufrManager;



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
                        PrecipitationRA, //akumulert nedb�r   (Automatisk)
			PrecipitationRR, //Fra 1 times nedb�r (Automatisk)
			PrecipitationRRR, //Fra manuell nedb�r
			//PrecipitationRR_24 //Fra RR_24
			PrecipitationRR_N //Fra RR_1, RR_3, RR_6, RR_12 eller RR_24
    };

 protected:
    bool           debug;
    bool           test; //Used for unit testing
    std::string    errorMsg;
    EPrecipitation precipitationParam;
    std::shared_ptr<EncodeBufrManager> encodeBufrManager;

    void windAtObstime( const DataElement &data, DataElement &res );
    
    void doIx( StationInfoPtr     info,
               const DataElementList &bufrData,
               BufrData           &bufr );
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
    void precip( BufrData &bufr,
                 float    RR1,
                 float    totalNedboer,
                 float    h_tr,
                 float    fRR24 );
    void doPrecip( StationInfoPtr     info,
                   const DataElementList &bufrData,
                   BufrData           &bufr );
    float precipFromRA( int hours, const DataElementList &sd, const boost::posix_time::ptime &from=boost::posix_time::ptime() ); //Helper method

    /**
     * Get the precipitation as 6 hours intervals.
     * @param hours
     * @param sd
     * @return
     */
    float precipFromRA_6( int hours, const DataElementList &sd ); //Helper method
    float precipFromRA( float &RR1, float &nedbor, float &fRR24, const DataElementList &sd );
    float precipFromRR( float &RR1, float &nedbor, float &fRR24, const DataElementList &sd );
    float precipFromRrN( float &RR1,
                         float &nedbor,
                         float &fRR24,
                         const DataElementList &sd );
    float  precipFromRRRtr( float &nedbor,
                          float &fRR24,
                          const DataElementList &sd );
 	 
    void dewPoint( const DataElement &data, BufrData &res );
    void minMaxTemperature(const DataElementList &sd, BufrData &res );
    void maxWindGust( const DataElementList &sd, BufrData &res  );
    void maxWindMax( const DataElementList &sd, BufrData &res );
    void cloudData( const DataElementList &data, BufrData &res );
    float pressure( float presure );
    void doPressureTrend( const DataElementList &data, DataElement &res );
    void computePressureTrend( const DataElementList &data,
                               DataElement &res);
    bool pressureTrend( const DataElementList &data, DataElement &res );
    void doGeneralWeather( const DataElementList &data, BufrData &res );
    static void doEsss( const DataElementList &data, BufrData &res );
    static void doSaFromSD( const DataElementList &data, BufrData &res );
    void soilTemp( const DataElementList &data, BufrData &res );
    void doSeaOrWaterTemperature(  const DataElementList &data, BufrData &res );


 public:
    
    
   Bufr(EPrecipitation precipitation);
   Bufr();

   ~Bufr();

   std::string getErrorMsg()const { return errorMsg;}
   void        setDebug(){ debug=true;}
   void        setTest( bool flag ) { test = flag; }

   /**
    * doBufr only compute the date used to encode BUFR it does NOT
    * encode the BUFR
    */
   bool doBufr( StationInfoPtr       info,
                DataElementList         &bufrData,
                BufrData             &bufr
              );

   /**
    * doBufr only compute the date used to encode BUFR it does NOT
    * encode the BUFR.
    * This is only a shorthand for 
    * doBufr StationInfoPtr       info,
    *            DataElementList         &bufrData,
    *            BufrData             &bufr
    *          );
    */
   BufrDataPtr doBufr( StationInfoPtr  info,
                       DataElementList &bufrData );


   /**
    * encodeBufr, call doBufr end endcode the BUFR in one go. It does
    * not set CCX or write the BUFR to file. 
    * Returns a pointer to BufrHelper on success or a null ptr on fail.
    */
   std::shared_ptr<BufrHelper> encodeBufr(StationInfoPtr  info,
                        DataElementList &bufrData);
    

};

#endif
