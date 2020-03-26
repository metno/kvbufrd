/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: bufr.cc,v 1.34.2.20 2007/09/27 09:02:23 paule Exp $

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
#include <sstream>
#include <algorithm>
#include <math.h>
#include <float.h>
#include "milog/milog.h"
#include "miutil/timeconvert.h"
#include "decodeutility/decodeutility.h"
#include "bufr.h"

/*CHANGES
 *
 * 2005.11.24
 * Bxrge Moe
 * Lagt inn støtte for FX_3 i Max_Vind_Max_Kode'en. Dette for å
 * støtte ARGOS bøyen på Svalbard.
 * 
 * 2003.05.28 Bxrge Moe
 * Endret kodingen av nedbor slik at all nedb�r mellom terminene 
 * (0,3,6,9,12,15,18 og 21) kodes med tR=5, nedbor siste time.
 *
 * 2003.05.26 Bxrge Moe
 * Endret prioritering av nedb�r. Automatisk observert nedb�r skal
 * g� foran manuell observert nedb�r. (Dette i overensstemmelse med 
 * Knut Bj�rheim (Roar Sk�lind))
 * I tillegg er all logikk med nedb�rsbereegning lagt inn i en
 * egen funksjon doNedboerKode.
 *
 * 2003.03.12 Bxrge Moe
 * Endring av beregningen av ned�r. Bruker kun RA (nedboerTot) dersom
 * ikke manuell nedb�r er gitt. Endring initiert av Obs. Div (Ragnar Brekkan)
 * Nedb�ren beregnes i nedborFromRA.
 * 
 * 2004.08.30 Bxrge Moe
 * Nedb�r vil kun legges til SYNOP dersom 'precipitation' parameteren
 * er gitt i konfigurasjonsfilen. Denne endringen er gjort for �
 * kode etter ny WMO standard hvor nedb�r altid skal v�re med hvis stasjonen
 * er satt opp til � v�re en nedb�rstasjon, Ir alltid 1. Hvis stasjonen ikke er
 * en nedb�r stasjon skal Ir settes 4.
 *
 * 2004.08.31 Bxrge Moe
 * Ny koding for RRRtr, i henhold til ny WMO sandard.
 * 
 * 2006.03.30 Bxrge Moe
 * Retter feil i kodingen av gruppe 7 (RR24) i seksjon 333.
 *
 * 2006.05.23 Bxrge Moe
 * -Lagt til støtte for automatiske målte verdier for ww (WaWa) i gruppe 7 i 
 *  seksjon 1.
 * -Lagt til støtte for automatisk målt h (HLN), laveste skybase.
 *
 * 2006-07-12 Bxrge Moe
 * -Når duggpunktet beregnes marginalt høyere, 0.5 grader C, enn  
 *  lufttemperaturen så settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tåpelig og ulogisk kode.
 * 
 * 2007-12-19 Bxrge
 * - Endret koding av gruppe 4 E'sss.
 * 
 * 2008-01-15 Bxrge
 * - Endret kodingen av vind fra knop to m/s.
 * 
 * 2008-01-16 Bxrge
 * - Lagt til st�tte for autmatisk m�lt VV (Vmor).
 * 
 * 2008-09-24 Bxrge
 * - Rettet avrundingsfeil i Gust, max vind og E'sss.
 * 2009-02-26 Bxrge
 * - #1241. Rettet feil i generereing av nedb�r for en 1 time fra RR_1. 
 *
 * 2009-03-23 Bxrge
 * - #1241. Rettet tr for en 1 times nedb�r fra RR_1. 
 *
 * 2009-03-24 Bxrge
 * - Rettet avrundingsfeil i max vind.
 *
 * 2009-09-23 Bxrge
 * - Ikke la negative verdier for Esss slippe gjennom til SYNOP.
 */

using namespace std;

namespace pt=boost::posix_time;

namespace {

   float c2kelvin( float t ) {
      if( t == FLT_MAX || t == FLT_MIN || t == -FLT_MAX ) {
         return FLT_MAX;
      }

      t += 273.15;

      return t<0?FLT_MAX:t;
   }

   float max( const KvParam &a, float b) {
      return (a<b) ? b : a;
   }
}

#define FEQ(f1, f2, d) ((fabsf((f1)-(f2)))<(d)?true:false)

bool
Bufr::
doBufr( StationInfoPtr  info,
        DataElementList &bufrData,
        BufrData        &bufr )
{
   if( bufrData.firstTime().is_special() && bufrData.size() == 0 )
      return false;

   bufr = BufrData(bufrData[bufrData.firstTime()]);

   bufr.TA     = c2kelvin( bufrData[0].TA );
   bufr.PO     = pressure( bufrData[0].PO );
   bufr.PR     = pressure( bufrData[0].PR );
   bufr.UU     = bufrData[0].UU;
   bufr.TW     = c2kelvin( bufrData[0].TW );
   bufr.SG     = bufrData[0].SG;

   doIx( info, bufrData, bufr );
   doSeaOrWaterTemperature( bufrData, bufr );
   soilTemp( bufrData, bufr );
   doEsss( bufrData, bufr );
   doGeneralWeather( bufrData, bufr );
   maxWindGust( bufrData, bufr );
   maxWindMax( bufrData, bufr );
   cloudData( bufrData, bufr );
   minMaxTemperature( bufrData, bufr );
   doPrecip( info, bufrData, bufr );
   doPressureTrend( bufrData, bufr );
   windAtObstime( bufrData[0], bufr );
   dewPoint( bufrData[0], bufr );

   return true;
}

BufrDataPtr
Bufr::
doBufr( StationInfoPtr  info,
        DataElementList &bufrData )
{
   BufrData *bufr = new BufrData();

   if( ! doBufr( info, bufrData, *bufr ) ) {
      delete bufr;
      bufr = 0;
   }

   return BufrDataPtr( bufr );
}

void 
Bufr::
windAtObstime( const DataElement &data, DataElement &res )
{  
   res.FF = FLT_MAX;
   res.DD = FLT_MAX;
   if( data.FF != FLT_MAX ) {
      if( data.FF >= 0 && data.FF <= 98 ) {
         res.FF = static_cast<float>( static_cast<int>( ( data.FF + 0.05 )*10 ) )/10;
         if( res.FF < 0.1 ) { //No wind.
            res.FF = 0;
            res.DD = 0;
            return;
         }
      }
   }

   if( data.DD != FLT_MAX ) {
      if(data.DD >= 0 && data.DD <= 360 ) {
         int dd = static_cast<int>( data.DD + 0.5);

         if( dd == 0 )
            res.DD = 360;
         else
            res.DD = dd;
      }
   }
}

void
Bufr::
doIx( StationInfoPtr     info,
      const DataElementList &bufrData,
      BufrData           &bufr )
{
   static int mannedTypeid[]={ 312, 308, 302, 0 };

   if( bufrData[0].onlyTypeid1 ) {
      bufr.IX = bufrData[0].IX;

      if( bufr.IX == FLT_MAX || (bufr.IX != 0 && bufr.IX != 1) )
         bufr.IX = 1;

      return;
   }

   if( bufr.IX.valAsInt() > 0 &&  bufr.IX.valAsInt() <  4)
       bufr.IX = 1;
   else
       bufr.IX = 0;

   for( std::list<int>::const_iterator it=bufrData[0].typeidList.begin();
         it != bufrData[0].typeidList.end(); ++it )
   {
      for( int i=0; mannedTypeid[i]!=0; ++i )
         if( *it == mannedTypeid[i] ) {
            bufr.IX = 1;
            return;
         }
   }

   //This is NOT one of the manned typeid. Check if there is
   //any parameters that occurs in manned stations only.

   if( bufrData[0].ww != FLT_MAX || bufrData[0].W1 != FLT_MAX ||
	   bufrData[0].W2 != FLT_MAX || bufrData[0].CH != FLT_MAX ||
	   bufrData[0].CL != FLT_MAX || bufrData[0].CM != FLT_MAX  ) {
	   bufr.IX = 1;
	   return;
   }
}

/*
 * Regner ut duggtemperaturen vha. formel
 * 
 * Bxrge Moe,  30.10.97
 * Pga. av maaleusikkerhet ved maaling av relativfuktighet kan maale
 * verdiene vaere stoerre enn 100%. Klimaavdelingen har bestemt foelgende
 * haandtering av verdier for relativfuktighet stoerre enn 100%:
 *
 * Hvis sensorverdien for relativ fuktighet er element i <100,104],
 * settes dugpunkt temperaturen lik maalt temperatur. Hvis verdien
 * for relativfuktighet er stoerre enn 104. Krysses verdien for
 * duggpunkt teperaturen ut (2////)
 *
 * Bxrge Moe, 1999.2.17
 * -Buggfix for feil i behandlingen av fuktighet st�rre enn 100%.
 * 
 * Bxrge Moe, 2006-07-12
 * -Når duggpunktet beregnes marginalt høyere, 0.5 grader C, enn 
 *  lufttemperaturen så settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tåpelig og ulogisk kode.
 *  
 */
void 
Bufr::
dewPoint(  const DataElement &data, BufrData &res  )
{
   int index;
   float CK[9] = {
   	6.10714,
	  	22.44294,
		272.440,
		6.10780,
		17.84362,
		245.425,
		6.10780,
		17.08085,
		234.175};
	float SVP;
   float VP;
   float Q1;
   float td;
   float fukt, temp;

   res.TD = FLT_MAX;

   fukt = data.UU;
   temp = data.TA;
   index = 0;

   if( data.TD != FLT_MAX ) {
       td = data.TD;

       if( temp != FLT_MAX && td > temp )
          td = temp;
   } else {
      if(fukt==FLT_MAX || temp==FLT_MAX)
         return;

      LOGDEBUG("dewPoint: UU=" << fukt << "  TA=" << temp);

      if(fukt>100.0){
         LOGDEBUG("dewPoint: UU(" << fukt << ")>100");

         if(fukt<=104)
            res.TD = c2kelvin( temp );
         return;
      }

      if(temp>0.0)
         index = 6;

      /* Saturation vapor pressure */
      SVP =  CK[index]*exp(CK[index+1]*temp/(CK[index+2]+temp));

      /* Actual vapor pressure */
      VP = fukt*SVP/100;

      /* Dewpoint temperature */
      Q1 = log(VP/CK[index]);
      td = CK[index+2]*Q1/(CK[index+1]-Q1);

      LOGDEBUG("devPoint: " << endl     <<
               "-- SVP=" << SVP << endl <<
               "--  VP=" <<  VP << endl <<
               "--  Q1=" <<  Q1 << endl <<
               "--  TD=" <<  td);
   }

   if(td>temp){
   	if(td < ( temp+0.5 ) )
   		td = temp;
   	else
   	   return;
   }

 	res.TD = c2kelvin( td );
} /* Dugg_Kode */

/* 25.11.97
 * Bxrge Moe
 *
 * Regner ut nattens minimumstemperatur hvis klokken er 06,
 * eller dagens maksimumstemperatur hvis klokken er 18.
 *
 * Rutinen er endret slik at vi ikke trenger 24 timer tilbake 
 * i tid med data.
 */
void 
Bufr::
minMaxTemperature(const DataElementList &sd, BufrData &res )
{
   int   nTimeStr=sd.nContinuesTimes();
   float min = FLT_MAX;
   float max = -FLT_MAX;

   res.TAN_N = FLT_MAX;
   res.TAX_N = FLT_MAX;
   res.tTAN_N = INT_MAX;
   res.tTAX_N = INT_MAX;

   if( sd[0].time().time_of_day().hours() == 6 || sd[0].time().time_of_day().hours() == 18 ) {
      if(sd[0].TAN_12 != FLT_MAX)
         res.TAN_N = sd[0].TAN_12;

      if(sd[0].TAX_12 != FLT_MAX)
         res.TAX_N = sd[0].TAX_12;

      if( res.TAN_N == FLT_MAX && nTimeStr>=12 ) {
         for(int i=0; i<12; i++){
            if(sd[i].TAN==FLT_MAX) {
               min = FLT_MAX;
               break;
            }

            if(sd[i].TAN<min)
               min=sd[i].TAN;
         }

         if( min != FLT_MAX )
            res.TAN_N = min;
      }

      if( res.TAX_N == FLT_MAX && nTimeStr>=12) {
         for(int i=0; i<12; i++){
            if( sd[i].TAX == FLT_MAX ) {
               max = FLT_MAX;
               break;
            }

            if( sd[i].TAX > max)
               max = sd[i].TAX;
         }

         if( max != -FLT_MAX && max != FLT_MAX )
            res.TAX_N = max;
      }

      if( res.TAN_N != FLT_MAX ) {
         if( sd[0].TA != FLT_MAX && sd[0].TA < res.TAN_N )
            res.TAN_N = sd[0].TA;

         res.TAN_N = c2kelvin( res.TAN_N );
      }

      if( res.TAX_N != FLT_MAX ) {
         if( sd[0].TA != FLT_MAX && sd[0].TA > res.TAX_N)
            res.TAX_N = sd[0].TA;

         res.TAX_N = c2kelvin( res.TAX_N );
      }

      res.tTAN_N = -12;
      res.tTAX_N = -12;

      return;
   }

   if( sd[0].TAX != FLT_MAX ) {
      if( sd[0].TA != FLT_MAX && sd[0].TA > sd[0].TAX)
         res.TAX_N = c2kelvin( sd[0].TA );
      else
         res.TAX_N = c2kelvin( sd[0].TAX );

      res.tTAX_N = -1;
   }

   if( sd[0].TAX != FLT_MAX ) {
      if( sd[0].TA != FLT_MAX && sd[0].TA < sd[0].TAN)
         res.TAN_N = c2kelvin( sd[0].TA );
      else
         res.TAN_N = c2kelvin( sd[0].TAN );

      res.tTAN_N = -1;
   }

}




/* 16.01.98
 * Bxrge Moe
 *
 * Beregning av Maksimalt vindkast (FG), maksimalvind (FX) 
 * siden forrige hovedobservasjon og vinden ved bufrtidspunkt (FF).
 * ( FG - Seksjon 333 911, FX - Seksjon 555 0 og FF - alle stasjoner Nddff )
 * -------------------------------------------------------------------------
 * 
 * Noen definisjoner.
 *    Automatstasjonene leverer parameterene ff, fx og fg hver time.
 *    Tilsvarende verdier for bufrtidspunktene er FF, FX og FG.
 *
 *    hvor
 *       FF - 10 minutters middel. Beregnet 10 minutter foer hver hele time.
 *       FX - er hoeyeste glidende 10 minutters middelverdi i en 69 minutters
 *            periode. ( (tt-2):51-tt:00, tt angir timen for beregningen.)
 *       FG - er hoeyeste glidende 3 sekunders middelverdi i loepet av en
 *            60 minutters periode. ((tt-1):00-tt:00, tt - timen for beregningen.)
 *
 *    Bufrtidene er 0, 3, 6, 9, 12, 15, 18, og 21. Hvor hovedtidene er
 *    0, 6, 12 og 18. Mellomtidene er 3, 9, 15 og 21.
 *
 *    La XX representere hovedtidene, og xx mellomtidene. 
 *
 * Beregningen av FF, FX og FG er gitt som foelger.
 *
 * 1) FF er enten ff(XX) eller ff(xx).
 * 2) FX(XX) er hoeyeste fx av timeverdiene XX, XX-1, XX-2, XX-3, XX-4, XX-5 og
 *    FF(XX-6) (NB! ff(XX-6), ikke fx(XX-6)).
 * 3) FX(xx) er hoeyeste fx av timeverdiene xx, xx-1, xx-2 og FF(xx-3).
 *    (NB! ff(xx-6), ikke fx(xx-6)) 
 * 4) FG(XX) er hoeyeste fg av timeverdiene XX, XX-1, XX-2, XX-3, XX-4 og XX-5.
 * 5) Dersom FG<FX, settes FG=FX.
 *
 * Koden for FG er gitt i rutinen 'maxWindGust' og koden for
 * FX er gitt i rutinen 'maxWindMax'.
 */

/* 16.01.98
 * Bxrge Moe
 *
 * Bufrgruppe: 333 911ff  (FG)
 *
 * Beregner maksimalt vindkast siden forrige hovedbufrtid.
 */ 
void 
Bufr::
maxWindGust( const DataElementList &data, BufrData &res )
{
    int   nTimeStr=data.nContinuesTimes();
    float fMax = -1.0;
    float dMax;
    std::string::iterator it;

    if( nTimeStr == 0 )
       return;
    
    res.FG_010 = data[0].FG_010;
    res.DG_010 = data[0].DG_010;

    if( ( data[0].time().time_of_day().hours() ) % 6 != 0 ) {
       if( data[0].FG_1 != FLT_MAX && data[0].FG_1 >=0 ) {
          res.FgMax.ff = data[0].FG_1;
          res.FgMax.dd = data[0].DG_1;
          res.FgMax.t = -60;
       }
       return;
    }

    if( data[0].FG_6 != FLT_MAX  && data[0].FG_6 >= 0 ) {
          res.FgMax.ff = data[0].FG_6;
          res.FgMax.dd = data[0].DG_6;
          res.FgMax.t = -360;
          return;
    }

    if(nTimeStr<6){
       CIDataElementList it = data.begin();
       pt::ptime prevTime=it->time();
       prevTime -= pt::hours(6);

       if( it->FG == FLT_MAX || it->FG<0)
     		return;

       CIDataElementList lastIt=data.find(prevTime);

       if( lastIt == data.end() || lastIt == it ||
           lastIt->FG == FLT_MAX || lastIt->FG < 0 )
          return;

       for( ++it; it != lastIt; ++it ) {
          if( it->FG != FLT_MAX ) {
             return;
          }
       }

       res.FgMax.ff = data[0].FG;
       res.FgMax.dd = data[0].DG;
       res.FgMax.t = -360;
       return;
    }


    for(int i=0; i<6; i++){
       if( data[i].FG_1 == FLT_MAX)
          return;
      
       if( data[i].FG_1 > fMax) {
          fMax = data[i].FG_1;
          dMax = data[i].DG_1;
       }
    }

    if(fMax<0)
      return;

    res.FgMax.ff = fMax;
    res.FgMax.dd = dMax;
    res.FgMax.t = -360;
}


void
Bufr::
maxWindMax(  const DataElementList &data, BufrData &res )
{
   int   nTimeStr=data.nContinuesTimes();
   int   nNeedTimes;
   float fMax;
   int   i;

   nNeedTimes=3;
   fMax=-1.0;

   if(( data[0].time().time_of_day().hours())%3 != 0) {
      if( data[0].FX_1 != FLT_MAX ) {
         res.FxMax.ff = data[0].FX_1;
         res.FxMax.t  = -60;
      }

      return;
   }

   if((data[0].time().time_of_day().hours())%6 == 0)
      nNeedTimes=6;

   if( nNeedTimes == 3 && data[0].FX_3 != FLT_MAX && data[0].FX_3 >= 0 ) {
      res.FxMax.ff = data[0].FX_3;
      res.FxMax.t  = -60*nNeedTimes;
      return;
   }

   if( nNeedTimes == 6 && data[0].FX_6 != FLT_MAX && data[0].FX_6 >= 0 ) {
      res.FxMax.ff = data[0].FX_6;
      res.FxMax.t  = -60*nNeedTimes;
      return;
   }

   if(nTimeStr < nNeedTimes && data[0].FX != FLT_MAX && data[0].FX >= 0 ){
      pt::ptime prevTime=data[0].time();
      prevTime -= pt::hours( nNeedTimes );
      CIDataElementList it=data.find(prevTime);

      if( it!=data.end() &&
          it->time()==prevTime &&
          it->FX != FLT_MAX &&
          it->FX >=0 ) {
         res.FxMax.ff = data[0].FX;
         res.FxMax.t  = -60*nNeedTimes;
         return;
      }
   }

   if( nTimeStr < nNeedTimes )
      return;

   i=nNeedTimes-1;

   if(data[i].FX_1==FLT_MAX )
      return;

   fMax=data[i].FX_1;
   i--;

   while(i>=0){
      if(data[i].FX_1==FLT_MAX)
         return;

      if(data[i].FX_1 > fMax)
         fMax = data[i].FX_1;

      i--;
   }

   if(fMax<0)
      return;

   if( data[0].FF != FLT_MAX && fMax < data[0].FF )
      fMax = data[0].FF;

   res.FxMax.ff = fMax;
   res.FxMax.t  = -60*nNeedTimes;
}


void
Bufr::
cloudData( const DataElementList &data, BufrData &res )
{


   if( data.size() == 0 )
      return;

   int CL = (data[0].CL==FLT_MAX?INT_MAX:static_cast<int>( data[0].CL ));
   int CM = (data[0].CM==FLT_MAX?INT_MAX:static_cast<int>( data[0].CM ));
   int CH = (data[0].CH==FLT_MAX?INT_MAX:static_cast<int>( data[0].CH ));
   int N=(data[0].N==FLT_MAX?INT_MAX:static_cast<int>( data[0].N ));

   N = N < 0 ? INT_MAX:N;

   res.VV = data[0].VV;

   if( res.VV == FLT_MAX && data[0].Vmor != FLT_MAX )
      res.VV = data[0].Vmor;

   /*if( CL == INT_MAX && CM == INT_MAX && CH == INT_MAX && N == INT_MAX )
      return; */

   if( N == 0  ) { //No clouds
      res.N = 0;
      res.vsci = 62;
      res.NH = 0;
      res.HL= FLT_MAX;
      res.CL = 30;
      res.CM = 20;
      res.CH = 10;
      return;
   }

   if( N == 9 ) {
      res.N = 113;
      res.vsci = 5;
      res.NH = 9;
      res.HL= FLT_MAX;
      res.CL = 62;
      res.CM = 61;
      res.CH = 60;
      return;
   }

   if( N != INT_MAX )
      res.N = (N/8.0) * 100;

   if( CL != INT_MAX )
      res.CL = 30 + CL;

   if( CM != INT_MAX )
      res.CM = 20 + CM;
   else if( CL != INT_MAX )
      res.CM = 61; //Cant be decided because of obscured by lower clouds or fog.

   if( CH != INT_MAX )
      res.CH = 10 + CH;
   else if( (CL != INT_MAX && CL != 0 ) || (CM != INT_MAX && CM != 0 ) )
      res.CH = 60; //Cant be decided because of obscured by lower clouds or fog.

   if( res.CL != FLT_MAX && res.CL >= 30 && res.CL <= 39 && res.CL != 30 ) //We have low clouds.
      res.vsci = 7;
   else if( res.CM != FLT_MAX && res.CM >= 20 && res.CM <= 29 && res.CM != 20 ) //We have midle clouds.
      res.vsci = 8;
   else if( res.CH != FLT_MAX && res.CH >= 10 && res.CH <= 19 && res.CH != 10 ) //We have high clouds.
      res.vsci = 9;
   else
      res.vsci = 63;

   res.NH = data[0].NH;
   res.HL = data[0].HL;

   if( res.HL == FLT_MAX && data[0].HLN != FLT_MAX )
      res.HL = data[0].HLN;

}

/*
 * 22.01.98
 * Bxrge Moe
 *
 * Rettet bugg i koden for generering av bufrkoden for trykk.
 * Har ogsaa lagt til test for aa se om trykkverdien er 'mulig'.
 * trykk maa vaere element i [800,1100]. (jfr. klimaavdelingen)
 */
float
Bufr::
pressure( float pressure )
{
   if( pressure == FLT_MAX)
      return FLT_MAX;

   if(pressure < 800.0 || pressure > 1100.0)
      return FLT_MAX;
        
   return pressure * 100;
} /* Trykk_Kode */


void
Bufr::
doPressureTrend( const DataElementList &data, DataElement &res )
{
   bool ok = pressureTrend( data, res );

   if( ok )
      return;

   if( data.nContinuesTimes() >= 4 )
      computePressureTrend( data, res );
}

/*Bxrge Moe
 *27.01.98
 *
 *Rettet en liten bugg i koden for tedens angivning.
 *
 */
void 
Bufr::
computePressureTrend( const DataElementList &data,
                      DataElement &res )
{
   int a;
   float dP1;
   float dP3;
   float PD3;
   float t1;
   float lim;

   float trykk1 = data[3].PO;
   float trykk2 = data[2].PO;
   float trykk3 = data[1].PO;
   float trykk4 = data[0].PO;

   if(trykk1==FLT_MAX || trykk2==FLT_MAX || trykk3==FLT_MAX || trykk4==FLT_MAX){
      return;
   }

   a      = 4;
   dP1    = trykk2 - trykk1;
   dP3    = trykk4 - trykk3;
   PD3    = trykk4 - trykk1;
   t1     =    dP3 - dP1;
   lim    = 0.01;
   
   if(PD3 > lim){
      if(t1 < (-1*lim)){
         if(dP3 < (-1*lim)) /*27.01.98 Bxrge Moe*/
            a = 0;
         else
            a = 1;
      }else{
         if(t1 > lim)
            a = 3;
         else
            a = 2;
      }
   }else if(PD3 < (-1*lim)){
      if(t1 > lim){ /*27.01.98 Bxrge Moe*/
         if(dP3 > lim)
            a = 5;
         else
            a = 6;
      }else if(t1 < (-1*lim))
         a = 8;
      else
         a = 7;
   }else if((PD3<=lim) && (PD3>=(-1*lim))){
      if(t1 > lim)
         a = 5;
      else if(t1<(-1*lim))
         a = 0;
      else
         a = 4;
   }else
      a = 4;

   res.AA = a;
   res.PP = PD3 * 100;
} /* Tendens_Kode */

/* 13.03.98
 * Bxrge Moe
 *
 * Prosedyren lager tedenskode fra manuelt intastede verdier. Kan ogsaa
 * endres til aa bruke verdier for trykktendens og karakteristikk beregnet
 * paa stasjonsnivaa.
 *
 * Trykktendensen er git med parameteren DATASTRUCTTYPE1.trykkTendens og
 * trykkarakteristikk er gitt med DATASTRUCTTYPE1._aa
 */
bool
Bufr::
pressureTrend( const DataElementList &data, DataElement &res)
{
   float PP = data[0].PP;
   float AA = data[0].AA;

   res.AA = FLT_MAX;
   res.PP = FLT_MAX;

   if( PP == FLT_MAX || AA == FLT_MAX)
      return false;

   int iAA = static_cast<int>( AA );
   res.AA = AA;

   //Adjust the sign of PP according to AA.
   if( ( iAA <= 3 && PP < 0 ) || ( iAA >= 5 && PP > 0 ) )
      PP *= -1;

   res.PP = PP * 100;
  	return true;
}







/**
 * Funksjonen lager koden 7wwW1W2 i seksjon 1, samt setter ix.
 * Manuelt observert ww går foran automatisk generert ww.
 * 
 * Den manuelt observerte ww ligger i datasettet som verGenerelt og
 * den automatiske ligger som WAWA.
 *  
 */
void
Bufr::
doGeneralWeather( const DataElementList &data, BufrData &res )
{
   bool pastWeather=false;

   res.ww = FLT_MAX;
   res.W1 = FLT_MAX;
   res.W2 = FLT_MAX;
   res.tWeatherPeriod = FLT_MAX;

   if( data.size() == 0 )
      return;


	if( data[0].ww != FLT_MAX ) {
	   res.ww = data[0].ww;
	} else if( data[0].WAWA!=FLT_MAX ){
      int i=static_cast<int>(round( data[0].WAWA) );

      if(i>=0 && i<100)
         res.ww = 100 + i;
   }

	
	if( data[0].W1 != FLT_MAX ) {
	   pastWeather = true;
	   res.W1 = data[0].W1;
	}

	if( data[0].W2 != FLT_MAX ) {
	   pastWeather = true;
	   res.W2 = data[0].W2;
	}

	if( pastWeather ) {
	   pt::ptime prevWeather;
	   CIDataElementList it = data.begin();
	   ++it;

	   for( ; it != data.end(); ++it ) {
	      if( it->ww != FLT_MAX ) {
	         prevWeather = it->time();
	         break;
	      }
	   }

	   if( prevWeather.is_special() ) {
	      int h = data[0].time().time_of_day().hours();

	      if( h%6 == 0 )
	         res.tWeatherPeriod = -6;
	      else if( h%3 == 0 )
	         res.tWeatherPeriod = -3;
	      else {
	         res.W1 = FLT_MAX;
	         res.W2 = FLT_MAX;
	      }
	   } else {
	      int d = (prevWeather- data[0].time()).hours();
	      res.tWeatherPeriod = d;
	   }
	}
    
 }
 


/**
 * Coding of E'sss.
 * 
 * The coding of E'sss is dependent on EM and SA.
 * 
 * EM -> E
 * SA -> sss
 * 
 * Når en værstasjon sender 998 vil de enten også sende E'= 1. (Hvis de har
 * utelatt E' må vi dekode E' til 1 siden 998 er en såpass bevisst handling.)
 *
 * Altså, det er E' som bestemmer om SA=-1 er flekkvis snø. I koding av bufr
 * må en altså for alle typeid bruke kombinasjonen av SA og E' eller SA og SD
 * for å kunne angi 998 i bufr.
 * SA=-1 når snødybde raporteres som "blank", utelatt (gruppe) eller "0" (Ingen
 * snø)
 * SA=-1 når snødybde raporteres som 998             (flekkvis snø)
 * SA=0  når snødybde raporteres som 997             (mindre enn 0.5 cm snø)
 * SA=-3 når snødybde raporteres som 999             (måling umulig)
 * EM=-1 når EM raporteres som "blank" eller utelatt (gruppe)
 * EM=0  er is-lag
 * EM= 1 - 9 er andel snødekke og type
 *
 * Bufr enkoding fra Kvalobs
 * Når det skal lages bufr fra kvalobs så må det kanskje ut fra dette til en
 * justering av dagens enkoder slik at koding av SA og EM blir riktig? (For 302
 * må kun SA benyttes i bufr - ikke SD.)
 * 
 */

void 
Bufr::
doEsss( const DataElementList &data, BufrData &res  )
{
   pt::ptime time = data[0].time();
   res.EM = FLT_MAX;
   res.EE = FLT_MAX;
   res.SA = FLT_MAX;

   if( (time.time_of_day().hours() % 6) != 0 )
      return;

   int iSA = (data[0].SA == FLT_MAX?INT_MAX:static_cast<int>(floor(static_cast<double>(data[0].SA) + 0.5 )));
   int iSD = (data[0].SD == FLT_MAX?INT_MAX:static_cast<int>(floor(static_cast<double>(data[0].SD) + 0.5 )));

   if( data[0].EM == FLT_MAX && data[0].EE == FLT_MAX && iSA == INT_MAX && iSD == INT_MAX)
      return;

   if( (data[0].EM == FLT_MAX && data[0].EE == FLT_MAX) && iSA != INT_MAX && iSD != INT_MAX )
      return doSaFromSD( data, res );

   if( data[0].EM == FLT_MAX && data[0].EE == FLT_MAX && iSA == INT_MAX )
      return;

   if( data[0].EE != FLT_MAX && data[0].EE >= 0 && data[0].EE < 31 ) {
      res.EE = floor( static_cast<double>( data[0].EE ) + 0.5 );
      res.EM = res.EE>=10?(res.EE-10):FLT_MAX;
   } else if( data[0].EM != FLT_MAX && data[0].EM>= 0 && data[0].EM <= 10 ) {
      res.EE = 10 + static_cast<int>( floor( static_cast<double>( data[0].EM ) + 0.5 ));
   }
   
   if( data[0].SA != FLT_MAX ) {
      if( iSA == -1 ) {
         if( res.EE != FLT_MAX  ) {
        	 if( res.EE < 10)
        		 res.SA = 0.00;
        	 else
        		 res.SA = -0.02;
         }
      } else if( iSA == 0 )
         res.SA = -0.01;
      else if( iSA == -3 )
         res.SA = FLT_MAX;
      else if( data[0].SA > 0 )
         res.SA = data[0].SA/100;
   }

   if( data[0].SS_24 != FLT_MAX )
	   res.SS_24 = data[0].SS_24/100;
}

void
Bufr::
doSaFromSD( const DataElementList &data, BufrData &res )
{
    pt::ptime time = data[0].time();
    res.SA = FLT_MAX;

    if( (time.time_of_day().hours() % 6) != 0 )
       return;

    int iSA = (data[0].SA == FLT_MAX?INT_MAX:static_cast<int>(floor(static_cast<double>(data[0].SA) + 0.5 )));
    int iSD = (data[0].SD == FLT_MAX?INT_MAX:static_cast<int>(floor(static_cast<double>(data[0].SD) + 0.5 )));

    if( iSA == INT_MAX && iSD == INT_MAX)
        return;

    if( iSA >= 0 ) {
        res.SA = data[0].SA/100;
        return;
    }

    if( iSD == INT_MAX )
        return;

    if( iSD >=1 && iSD <= 3  )
        res.SA = -0.02;
    else if( iSD == 0 || iSD == -1 )
        res.SA = 0;
}

void
Bufr::
doSeaOrWaterTemperature(  const DataElementList &data, BufrData &res )
{
   res.TW = FLT_MAX;

  	if( data[0].TW != FLT_MAX || data[0].TWF != FLT_MAX ) {
  	   res.TW = data[0].TW!=FLT_MAX?data[0].TW:data[0].TWF;
  	   res.TW = c2kelvin( res.TW );
  	}
}

/**
 * GressTemperaturen angis klokken 06, og angir minimums
 * temperaturen de siste 12 timene. 
 * For automatiske m�lteverdier trenger vi 12 timestrenger,
 * parameteren har navnet TGN.
 *
 * For manuelt m�lt verdi har parametern navnet TGN_12 og
 * vi trenger bare en timestreng.
 */
void 
Bufr::
soilTemp( const DataElementList &data, BufrData &res )
{
   int nTimeStr=data.nContinuesTimes();

   res.TGN_12 = FLT_MAX;

   if( nTimeStr == 0 )
      return;

   if( (data[0].time().time_of_day().hours() != 6) && 
      (data[0].time().time_of_day().hours() != 18) )
      return;

   if( data[0].TGN_12 != FLT_MAX )
      res.TGN_12 = data[0].TGN_12;

   if( res.TGN_12 == FLT_MAX && nTimeStr >= 12 ) {
      float min = FLT_MAX;
      int i = 0;

      while( i < 12 ){
         if( data[i].TGN == FLT_MAX ) {
            min=FLT_MAX;
            break;
         }

         if( data[i].TGN < min )
            min = data[i].TGN;
         i++;
      }

      if( min != FLT_MAX )
         res.TGN_12 = min;
   }

   if( res.TGN_12 != FLT_MAX )
      res.TGN_12 = c2kelvin( res.TGN_12 );
}


/*
 * Funksjonen lagar nedboerkoda for nedboermengde i perioden.
 *
 * Lagt til ir for � styre kodingen av RRRtr.
 * 
 * ir==1, det har falt m�lbar nedb�r.
 * ir==3, t�rt, det har ikke falt nedb�r, kode ==> 6000tr 
 * ir==4, 
 */

void
Bufr::
precip( BufrData &bufr,
        float    RR1,
        float    totalNedboer,
        float    h_tr,
        float    fRR24 )
{
   int time = bufr.time().time_of_day().hours();

   if( time==6 ){
      if( fRR24 != FLT_MAX &&  fRR24 < 1000.0 ) {
         bufr.precip24.hTr = -24;
         bufr.precip24.RR = fRR24;
      }
   }

   //Allways set the time periode, even when the precipitation is missing.

   if( (time % 6) == 0 ) {
      int tr = -12; // 6 and 18

      if( (time % 12) == 0 )  // 0 and 12
        tr = -6;

      bufr.precipRegional.hTr = tr;

      if( h_tr != FLT_MAX && static_cast<int>( h_tr ) == tr && totalNedboer != FLT_MAX )
         bufr.precipRegional.RR = totalNedboer;

   }

   if( h_tr == FLT_MAX || totalNedboer == FLT_MAX)
      return;

   if( RR1 != FLT_MAX ) {
      bufr.precipNational.hTr = -1;
      bufr.precipNational.RR = RR1;
   } else if( (time % 6) != 0 ) {
      bufr.precipNational.hTr = h_tr;
      bufr.precipNational.RR = totalNedboer;
   }
 } /* Nedboer_Kode */



void
Bufr::doPrecip( StationInfoPtr     info,
                const DataElementList &bufrData,
                BufrData           &bufr )
{
  	ostringstream ost;
  	float nedboerTotal=0.0;
  	float fRR24=FLT_MAX;
  	float h_tr=FLT_MAX;
  	float RR1=FLT_MAX;

  	if( bufrData.size() < 0 )
  	   return;

  	precipitationParam=NoPrecipitation;
  	StationInfo::TStringList precipDef=info->precipitation();

  	if( precipDef.size() > 0 ){
  	   string rr=*precipDef.begin();

  	   if(rr=="RA"){
  	      precipitationParam=PrecipitationRA;
  	   }else if(rr=="RR_1" || rr=="RR"){
  	      precipitationParam=PrecipitationRR;
  	   }else if(rr=="RR_3" || rr=="RR_6"
  	         || rr=="RR_12" || rr=="RR_24" || rr=="RR_N" ){
  	      precipitationParam=PrecipitationRR_N;
  	   }else if( rr=="RRRtr"){
  	      precipitationParam=PrecipitationRRR;
  	   }else if( rr=="RR_01"){
  	      //NOT implemented (Not needed) yet
  	   }
  	}

  	if(precipitationParam==NoPrecipitation)
  	   return;


  	ost << "doPrecip: sisteTid: " << pt::to_kvalobs_string(bufrData[0].time()) << endl;;

  	if(precipitationParam==PrecipitationRA){
    	h_tr = precipFromRA( RR1, nedboerTotal, fRR24, bufrData );
    	ost << "doPrecip: EPrecipitationParam: RA    (Automatisk)" << endl
			<< "                   RR_24: " << fRR24          << endl
			<< "                  nedb�r: " << nedboerTotal   << endl
			<< "                    h_tr: " << h_tr             << endl;
  	}else if(precipitationParam==PrecipitationRR){
    	h_tr = precipFromRR( RR1, nedboerTotal, fRR24, bufrData );
    	ost << "doPrecip: EPrecipitationParam: RR    (Automatisk)" << endl
    	    << "                nedb�r: " << nedboerTotal << endl
    	    << "                  h_tr: " << h_tr << endl;
  	}else if(precipitationParam==PrecipitationRR_N){
    	ost << "doPrecip: EPrecipitationParam: RR_N, hvor N=1,3,6,12,24" << endl;
    	h_tr = precipFromRrN( RR1, nedboerTotal, fRR24, bufrData );
    	ost << "                 nedb�r: " << nedboerTotal << endl
    	    << "                   h_tr: " << h_tr << endl;
  	}else if(precipitationParam==PrecipitationRRR){
    	ost << "doPrecip: PrecipitationParam: RRR  (Manuell)" << endl;
    	h_tr=precipFromRRRtr(nedboerTotal, fRR24, bufrData);

    	if( bufrData[0].RR_1 != FLT_MAX )
    	   RR1 = bufrData[0].RR_1;
    	else
    	   RR1 = precipFromRA( 1, bufrData );

    	ost << "                   RR_24: " << fRR24          << endl
          << "                  nedb�r: " << nedboerTotal   << endl;
  	}else {
    	ost << "PrecipitationParam: UNKNOWN" << endl;
    	return;
  	}

  	LOGDEBUG( ost.str() );

  	precip( bufr, RR1, nedboerTotal, h_tr, fRR24 );
}



/**
 * 2003.03.12 Bxrge Moe
 *
 * nedborFromRA, beregner nedb�ren fra RA (Akumulert nedb�r).
 * Nedb�ren beregnes ved � ta differansen mellom  RA fra bufrtidspunktet og
 * RA 12 (evt 24) timer tilbake. Dette betyr at nedb�ren bergnes p� f�lgende
 * m�te for bufrtidspunktene 06, 12, 18 og 24. Bruke notasjonen RA(t) for �
 * angi b�tteinholdet ved timen t. Eks RA(12) er b�tteinnholdet kl 12.
 * 
 * bufrtidspunkt kl 00 og 12,  6 timers nedb�r:
 *    nedb�r= RA(t)-RA(t-6)
 *
 * bufrtidspunkt kl 6 og 18,  12 timers nedb�r:
 *    nedb�r=RA(t)-RA(t-12)
 *
 * Nedb�ren raporteres bare dersom den er over en gitt grense. For
 * �yeblikket er den hardjodet til 0.15. Dette kan endres dersom
 * obsdiv finner det n�dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse p� 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedb�ren for.
 * \return ir
 */


float
Bufr::
precipFromRA( float &RR1,
              float &precip,
              float &fRR24,
              const DataElementList &sd )
{
   int   nTimes;
   pt::ptime t = sd.begin()->time();
   int   time = t.time_of_day().hours();

   RR1 = FLT_MAX;

   if( sd[0].RR_1 != FLT_MAX )
      RR1 = sd[0].RR_1 > 0 ? sd[0].RR_1 : 0;

   //RR1 = precipFromRA( 1, sd );

   precip = FLT_MAX;
   fRR24 = FLT_MAX;

   if( time == 6 || time == 18)
      nTimes=12;
   else if( time == 0 || time == 12)
      nTimes=6;
   else
      nTimes=1;

   if( time == 6 ) {
      fRR24 = precipFromRA_6( 24, sd );
   }

   if( nTimes == 1 )
      precip = RR1;
   else
      precip = precipFromRA_6( nTimes, sd );

   LOGDEBUG("Time:          " << t << endl
         << "  precipitation = " <<  precip << endl
         << "          RR_24 = " <<  fRR24 << endl
         << "             R1 = " <<  RR1 << endl
         << "         nTimes = " << nTimes << endl );

   return ( precip != FLT_MAX ? -1*nTimes : FLT_MAX ) ;
}


float
Bufr::
precipFromRA( int hours, const DataElementList &sd, const pt::ptime &from )
{

   const float limit = 0.19;
   const float bucketFlush = -10.0;
   pt::ptime t;
   pt::ptime t2;
   DataElement d1;
   DataElement d2;
   CIDataElementList it;
   float precip = FLT_MAX;

   if( from.is_special() ) {
      d1=*sd.begin();
   } else {
      it = sd.find( from );
      if( it == sd.end() )
         return FLT_MAX;

      d1 = *it;
   }
   t = d1.time();
   t2 =t;
   t2 -= pt::hours( hours );

   it=sd.find( t2 );

   if( it != sd.end() && it->time() == t2 ) {
      d2 = *it;

      if( d1.RA != FLT_MAX && d2.RA != FLT_MAX ){
         precip = d1.RA - d2.RA;

         if( precip > bucketFlush){
            if( precip <= limit)
               precip = 0.0;  //No precipitation
         }else{
            //The bucket is flushed.
            precip = FLT_MAX;
         }
      }
   }

   return precip;
}

float
Bufr::
precipFromRA_6( int hours, const DataElementList &sd )
{
   float sum = 0.0;
   float rr;
   int hoursComputed=0;
   pt::ptime from = sd.begin()->time();

   while( hoursComputed < hours ){
      rr = precipFromRA( 6, sd, from );
      if( rr == FLT_MAX )
         return FLT_MAX;

      sum += rr;
      hoursComputed += 6;
      from -= pt::hours( 6 );
   }

   return sum;
}
/*
 * bufrtidspunkt kl 00 og 12,  6 timers nedb�r: RR_6
 * bufrtidspunkt kl 6 og 18,  12 timers nedb�r: RR_12
 * 
 * S�ker i nedb�rparametrene RR_N, hvor N=1, 3, 6, 12, 24
 * S�ker nedb�ren fra den f�rste som er gitt, s�ker fra 24, 12, .. ,1
 */
float
Bufr::
precipFromRrN( float &RR1,
               float &precip,
               float &fRR24,
               const DataElementList &sd)
{
   float h_tr = FLT_MAX;
   int t = sd.begin()->time().time_of_day().hours();

   RR1 = FLT_MAX;

   if( sd[0].RR_1 != FLT_MAX )
      RR1 = max( sd[0].RR_1, 0 );

   precip = FLT_MAX;
   fRR24 = FLT_MAX;

   if( t==6 && sd[0].RR_24 != FLT_MAX) {
      fRR24 = max( sd[0].RR_24, 0 );
   }

   if((t==6 || t==18) && sd[0].RR_12 != FLT_MAX){
      precip = max( sd[0].RR_12, 0 );
      h_tr = -12;
   }else if( ( t == 12 || t == 0 ) && sd[0].RR_6 != FLT_MAX){
      precip = max( sd[0].RR_6, 0 );
      h_tr = -6;
   }

   if( precip == FLT_MAX){
      if( sd[0].RR_1 != FLT_MAX){
         precip = max( sd[0].RR_1, 0 );
         h_tr = -1;
      }else if(sd[0].RR_2 != FLT_MAX){
         precip = max( sd[0].RR_2, 0 );
         h_tr = -2;
      }else if( sd[0].RR_3 != FLT_MAX ){
         precip = max( sd[0].RR_3, 0 );
         h_tr = -3;
      }else if( sd[0].RR_6 != FLT_MAX){
         precip = max( sd[0].RR_6, 0 );
         h_tr = -6;
      }else if( sd[0].RR_9 != FLT_MAX ){
         precip = max( sd[0].RR_9, 0 );
         h_tr = -9;
      }else  if( sd[0].RR_12 != FLT_MAX){
         precip = max( sd[0].RR_12, 0 );
         h_tr = -12;
      }else if( sd[0].RR_15 != FLT_MAX){
         precip = max( sd[0].RR_15, 0 );
         h_tr = -15;
      }else if( sd[0].RR_18 != FLT_MAX){
         precip = max( sd[0].RR_18, 0 );
         h_tr = -18;
      } else if( sd[0].RR_24 != FLT_MAX){
         precip = max( sd[0].RR_24, 0);
         h_tr = 24;
      }
   }

   if( precip == FLT_MAX )
      return FLT_MAX;

   if(t==6 && fRR24==FLT_MAX && sd.size()>1){
      CIDataElementList it = sd.begin();
      pt::ptime tt = it->time();

      tt -= pt::hours(12);

      CIDataElementList it2=sd.find(tt);

      if( it2 != sd.end() && it2->time() == tt ){
         bool hasPrecip = true;
         fRR24 = 0.0;

         //If there is measured no precip in the 12 hour time
         //periode the nedboer12Time=-1.0.
         //We cant just test for for nedboer12Time==-1.0. 


         if(it->RR_12!=FLT_MAX){
            if( it->RR_12 >= 0 )
               fRR24 += it->RR_12;
            else if( it->RR_12 <= -1.5f )
               hasPrecip=false;
         }else if( static_cast<int>(it->IR) != 3)
            hasPrecip=false;

         if( it2->RR_12 != FLT_MAX ){
            if( it2->RR_12 >= 0 )
               fRR24+=it2->RR_12;
            else if( it2->RR_12<=-1.5f )
               hasPrecip=false;
         }else if( static_cast<int>(it2->IR) != 3 )
            hasPrecip=false;

         if( !hasPrecip )
            fRR24=FLT_MAX;
      }
   }

   if( precip < 0.05)
      precip = 0;

   return h_tr;
}  


/**
 * Nedb�r fra manuelle nedb�rstasjoner. Bruker ITR for � sjekke
 * om det er angitt manuell nedb�r. Hvis ITR er satt nedb�ren gitt.
 * Bruker ITR for � finne rett nedb�rparameter.
 *
 *  ITR    Nedb�r parameter
 *  -----------------------
 *    1    nedboer6Time  (RR_6)
 *    2    nedboer12Time (RR_12)
 *    3    nedboer18Time (RR_18) 
 *    4    nedboer24Time (RR_24)
 *    5    nedboer1Time  (RR_1)
 *    6    nedboer2Time  (RR_2)  
 *    7    nedboer3Time  (RR_3)
 *    8    nedboer9Time  (RR_9)  
 *    9    nedboer15Time (RR_15) 
 *
 * Hvis ITR har en gyldig verdi og nedb�ren er -1 angir dette t�rt.
 */
float
Bufr::
precipFromRRRtr( float &nedbor,
                 float &fRR24,
                 const DataElementList &sd )
{
   float h_tr = FLT_MAX;
	nedbor=FLT_MAX;
  	fRR24 =FLT_MAX;

  	if( sd.begin()->time().time_of_day().hours() == 6 ) {
  		CIDataElementList it=sd.begin();
    	pt::ptime tt=it->time();
	
    	tt -=pt::hours( 12 );
    
    	CIDataElementList it2=sd.find(tt);
    	
    	if( it2 != sd.end() && it2->time() == tt ){
    		bool hasPrecip=true;
			
    		//If there is measured no precip in the 12 hour time
    		//periode the nedboer12Time=-1.0.
    		//We cant just test for for nedboer12Time==-1.0.
         
    		//cerr << "nedboer12Time:  " << it->nedboer12Time << endl
    		//    <<"nedboer12Time2: " << it2->nedboer12Time << endl;
                    
			if( it->RR_12 != FLT_MAX ){
				if( it->RR_12 >= 0.0 )
	  			   fRR24 = it->RR_12;
				else if( it->RR_12 < -1.001f || it->RR_12 > -0.999 )
				   hasPrecip = false;
				else if( static_cast<int>( it->RR_12 ) == -1 )
				   fRR24 = -1;
			}else if( static_cast<int>( it->IR ) == 3 )
			   fRR24 = -1;
			else
			   hasPrecip = false;

			if( it2->RR_12 != FLT_MAX){
				if( it2->RR_12 >= 0.0) {
					if( fRR24 == FLT_MAX || static_cast<int>( fRR24 ) == -1 )
						fRR24 = 0.0;

					fRR24 += it2->RR_12;
				} else if( it2->RR_12 < -1.001 || it2->RR_12 > 0.999 )
					hasPrecip = false;
				else {
				   if( fRR24 == FLT_MAX )
				      fRR24 = -1;
				}
			}else if( static_cast<int>( it2->IR ) != 3 )
				hasPrecip = false;

			if( !hasPrecip )
				fRR24 = FLT_MAX;
			else if( FEQ( fRR24, 0.0, 0.01 ) )
				fRR24 = -0.1; //Trace of precipitation.
    	}
    	
    	if( fRR24 == FLT_MAX ){
    		//Do we have an RR_24 precip.
    		fRR24=sd.begin()->RR_24;
    		
    		if( fRR24 != FLT_MAX ) {
    		   if( fRR24 < 0.0)
    		      fRR24 = FLT_MAX;
    		   else if( fRR24 > -1.1 && static_cast<int>( fRR24 ) == -1 )
    		      fRR24 = 0.0;
    		}
    	} else if( fRR24 >= -1.01 && fRR24 <= -0.99 ) // fRR24 == -1
    	   fRR24 = 0.0;
  	}

   //cerr << "sd[0].ITR: [" << (sd[0].ITR[0]-'0') << "]" << endl;

  	if( sd[0].ITR  == FLT_MAX) {
  	   if( static_cast<int>( sd[0].IR ) == 3 )
  	      nedbor = 0.0;
  	   else {
  	      nedbor = FLT_MAX;
  	      return FLT_MAX;
  	   }

  	   if( (sd.begin()->time().time_of_day().hours() % 6) != 0 ) {
  	      nedbor = FLT_MAX;
  	      return FLT_MAX;
  	   } else if( sd.begin()->time().time_of_day().hours() == 12 || sd.begin()->time().time_of_day().hours() == 18 )
  	      return -6;
  	   else
  	      return -12;
  	}

  	switch( static_cast<int>( sd[0].ITR ) ){
  	case 1: 
    	if(sd[0].RR_6!=FLT_MAX){
      		nedbor = max( sd[0].RR_6, 0 );
      		h_tr = -6;
     	}
    	break;
  	case 2:
    	if(sd[0].RR_12!=FLT_MAX){
      		nedbor = max( sd[0].RR_12, 0 );
      		h_tr=-12;
     	}
    	break;
  	case 3:
    	if(sd[0].RR_18!=FLT_MAX){
      		nedbor = max( sd[0].RR_18, 0 );
      		h_tr=-18;
   		}
    	break;
  	case 4:
    	if(sd[0].RR_24!=FLT_MAX){
      		nedbor = max( sd[0].RR_24, 0 );
      		h_tr=-24;
     	}
    	break;
  	case 5:
    	if(sd[0].RR_1!=FLT_MAX){
      		nedbor = max( sd[0].RR_1, 0 );
      		h_tr=-1;
     	}
    	break;
  	case 6:
    	if(sd[0].RR_2!=FLT_MAX){
      		nedbor = max( sd[0].RR_2, 0 );
      		h_tr=-2;
    	}
    	break;
  	case 7:
    	if(sd[0].RR_3!=FLT_MAX){
      		nedbor = max( sd[0].RR_3, 0 );
      		h_tr=3;
     	}
    	break;
  	case 8:
    	if(sd[0].RR_9!=FLT_MAX){
      		nedbor = max( sd[0].RR_9, 0 );
      		h_tr=-9;
    	}
    	break;
  	case 9:
    	if(sd[0].RR_15!=FLT_MAX){
      		nedbor = max( sd[0].RR_15, 0 );
      		h_tr=-15;
    	}
    	break;
  	default:
    	return FLT_MAX;
  	}

  	if(nedbor==FLT_MAX) 
    	return FLT_MAX;

  	//cerr << "RRRtr(+): nedbor=" << nedbor << " h_tr= " << h_tr << endl;
  	if( static_cast<int>( round( nedbor ) ) == -1 ) //t�rt
  	   nedbor = 0.0;
  	//cerr << "RRRtr(-): nedbor=" << nedbor << " h_tr= " << h_tr << endl;
 
  	return h_tr;
}  



/**
 * 2003.03.14 Bxrge Moe
 *
 * nedborFromRR, beregner nedb�ren fra RR (Times nedb�r).
 * Nedb�ren beregnes ved � summere RR fra bufrtidspunktet og
 * 12 (evt 6) timer tilbake. 

 * bufrtidspunkt kl 00 og 12,  6 timers nedb�r:
 *    nedb�r= RR(t)+RR(t-1)+ .... +RR(t-6)
 *
 * bufrtidspunkt kl 6 og 18,  12 timers nedb�r:
 *    nedb�r=RR(t)+RR(t-1)+ .... +RR(t-12)
 *
 * Nedb�ren raporteres bare dersom den er over en gitt grense. For
 * �yeblikket er den hardkodet til 0.15. Dette kan endres dersom
 * obsdiv finner det n�dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse p� 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedb�ren for.
 * \return ir
 */

float
Bufr::
precipFromRR(  float &RR1, float &nedbor, float &fRR24, const DataElementList &sd )
{
  	const float limit=0.049;
  	int   nTimeStr=sd.nContinuesTimes();
  	int   time=sd.begin()->time().time_of_day().hours();
  	int   nTimes;
  	float sum=0;

  	RR1 = FLT_MAX;

  	if( sd[0].RR_1 != FLT_MAX )
  	   RR1 = max( sd[0].RR_1, 0 );

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(time==6 || time==18)
    	nTimes=12;
  	else if(time==0 || time==12)
    	nTimes=6;
  	else
    	nTimes=1;
  	
  	if(nTimeStr<nTimes)
    	return FLT_MAX;

  	for(int i=0; i<nTimes; i++){
    	if(sd[i].RR_1==FLT_MAX)
      	return FLT_MAX;
    
    	if(sd[i].RR_1>=0)
      	sum+=sd[i].RR_1;
  	}

  	nedbor=sum;

  	if(time==6 && nTimeStr>=24){
    	int i;
    	sum=0.0;

    	for(i=0; i<24; i++){
    		if(sd[i].RR_1==FLT_MAX)
				break;
    		else if(sd[i].RR_1>=0.0)
				sum+=sd[i].RR_1;
    	}
    
    	if(i==24)
    		fRR24=sum;
  	}
  
  	if(nedbor<=limit)
    	nedbor=0.0;

  	return -1*nTimes;
}




Bufr::
Bufr(EPrecipitation pre)
  :debug(false), test( false), precipitationParam(pre)
    
{
}

Bufr::
Bufr():debug(false), test( false ), precipitationParam(PrecipitationRA)
{
}

Bufr::
~Bufr()
{
}

















