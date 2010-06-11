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
#include <milog/milog.h>
#include <decodeutility/decodeutility.h>
#include "bufr.h"

/*CHANGES
 *
 * 2005.11.24
 * Bxrge Moe
 * Lagt inn stÃ¸tte for FX_3 i Max_Vind_Max_Kode'en. Dette for Ã¥
 * stÃ¸tte ARGOS bÃ¸yen pÃ¥ Svalbard.
 * 
 * 2003.05.28 Bxrge Moe
 * Endret kodingen av nedbor slik at all nedbï¿½r mellom terminene 
 * (0,3,6,9,12,15,18 og 21) kodes med tR=5, nedbor siste time.
 *
 * 2003.05.26 Bxrge Moe
 * Endret prioritering av nedbï¿½r. Automatisk observert nedbï¿½r skal
 * gï¿½ foran manuell observert nedbï¿½r. (Dette i overensstemmelse med 
 * Knut Bjï¿½rheim (Roar Skï¿½lind))
 * I tillegg er all logikk med nedbï¿½rsbereegning lagt inn i en
 * egen funksjon doNedboerKode.
 *
 * 2003.03.12 Bxrge Moe
 * Endring av beregningen av nedï¿½r. Bruker kun RA (nedboerTot) dersom
 * ikke manuell nedbï¿½r er gitt. Endring initiert av Obs. Div (Ragnar Brekkan)
 * Nedbï¿½ren beregnes i nedborFromRA.
 * 
 * 2004.08.30 Bxrge Moe
 * Nedbï¿½r vil kun legges til SYNOP dersom 'precipitation' parameteren
 * er gitt i konfigurasjonsfilen. Denne endringen er gjort for ï¿½
 * kode etter ny WMO standard hvor nedbï¿½r altid skal vï¿½re med hvis stasjonen
 * er satt opp til ï¿½ vï¿½re en nedbï¿½rstasjon, Ir alltid 1. Hvis stasjonen ikke er
 * en nedbï¿½r stasjon skal Ir settes 4.
 *
 * 2004.08.31 Bxrge Moe
 * Ny koding for RRRtr, i henhold til ny WMO sandard.
 * 
 * 2006.03.30 Bxrge Moe
 * Retter feil i kodingen av gruppe 7 (RR24) i seksjon 333.
 *
 * 2006.05.23 Bxrge Moe
 * -Lagt til stÃ¸tte for automatiske mÃ¥lte verdier for ww (WaWa) i gruppe 7 i 
 *  seksjon 1.
 * -Lagt til stÃ¸tte for automatisk mÃ¥lt h (HLN), laveste skybase.
 *
 * 2006-07-12 Bxrge Moe
 * -NÃ¥r duggpunktet beregnes marginalt hÃ¸yere, 0.5 grader C, enn  
 *  lufttemperaturen sÃ¥ settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tÃ¥pelig og ulogisk kode.
 * 
 * 2007-12-19 Bxrge
 * - Endret koding av gruppe 4 E'sss.
 * 
 * 2008-01-15 Bxrge
 * - Endret kodingen av vind fra knop to m/s.
 * 
 * 2008-01-16 Bxrge
 * - Lagt til støtte for autmatisk målt VV (Vmor).
 * 
 * 2008-09-24 Bxrge
 * - Rettet avrundingsfeil i Gust, max vind og E'sss.
 * 2009-02-26 Bxrge
 * - #1241. Rettet feil i generereing av nedbør for en 1 time fra RR_1. 
 *
 * 2009-03-23 Bxrge
 * - #1241. Rettet tr for en 1 times nedbør fra RR_1. 
 *
 * 2009-03-24 Bxrge
 * - Rettet avrundingsfeil i max vind.
 *
 * 2009-09-23 Bxrge
 * - Ikke la negative verdier for Esss slippe gjennom til SYNOP.
 */

using namespace std;

namespace {

   float c2kelvin( float t ) {
      if( t == FLT_MAX || t == FLT_MIN )
         return t;

      t += 273.15;

      return t<0?FLT_MAX:t;
   }
}

#define FEQ(f1, f2, d) ((fabsf((f1)-(f2)))<(d)?true:false)

bool
Bufr::
doBufr( StationInfoPtr  info,
        BufrDataList    &bufrData,
        BufrData        &bufr,
        bool            create_CCA_template )
{
   if( bufrData.firstTime().undef() && bufrData.size() == 0 )
      return false;

   bufr = BufrData();
   bufr.time( bufrData.firstTime() );


   bufr.TAN_12 = c2kelvin( bufrData[0].TAN_12 );
   bufr.TAX_12 = c2kelvin( bufrData[0].TAX_12 );
   bufr.TGN    = c2kelvin( bufrData[0].TGN );
   bufr.TGN_12 = c2kelvin( bufrData[0].TGN_12 );
   bufr.TW     = c2kelvin( bufrData[0].TW );
   bufr.TWM    = c2kelvin( bufrData[0].TWM );
   bufr.TWN    = c2kelvin( bufrData[0].TWN );
   bufr.TWX    = c2kelvin( bufrData[0].TWX );
   bufr.TA     = c2kelvin( bufrData[0].TA );
   bufr.TAM    = c2kelvin( bufrData[0].TAM );
   bufr.TAN    = c2kelvin( bufrData[0].TAN );
   bufr.TAX    = c2kelvin( bufrData[0].TAX );
   bufr.PO     = pressure( bufrData[0].PO );
   bufr.PR     = pressure( bufrData[0].PR );





   doPrecip( info, bufrData, bufr );
   doPressureTrend( bufrData, bufr );
   cloudCower( bufrData[0], bufr );
   windAtObstime( bufrData[0], bufr );
   dewPoint( bufrData[0], bufr );


   return true;
}


#if 0

bool
Bufr::
doBufr( StationInfoPtr info,
        BufrDataList   &bufrData,
        BufrData       &bufr,
        bool           create_CCA_template )
{
	milog::LogContext context("bufr");

	if( !info ) {
		LOGERROR( "Cant create bufr. Missing station information.");
		return false;
	}

	using std::string;

	string utsteder = info->owner();
	int wmono = info->wmono();
    int listenummer = atoi( info->list().c_str() );
    int    ir;
    int    ix;
    char   tmp[512];
    string buf;
    string dato_tid;
    string bufrStr;
    string tempStr;
    string tidsKode;
    string luftTempKode;
    string RRRtr;
    string nedboerKodeSect3;
    string nedboerKodeSect1;
    string duggTempKode;
    string minMaxKode;
    string maxMinKode;
    string naaVindKode;
    string maxVindGustKode;
    string maxVindMaxKode;
    string trykkQFFKode;
    string trykkQFEKode;
    string tendensKode;
    string skydekkeKode;
    string hoyde_siktKode;
    string verGenereltKode;
    string verTilleggKode;
    string skyerKode;
    string snoeMarkKode;
    string skyerEkstraKode1;
    string skyerEkstraKode2;
    string skyerEkstraKode3;
    string skyerEkstraKode4;
    string sjoeTempKode;
    string gressTemp;
    bool   verGenerelt;
    string rr24Kode;
    int    ITR;
    string tmpUtsteder(utsteder);
    BufrData sisteTid;
    int       nTimeStr=0;
    

    bufr = BufrData(); //Clear the result bufr.
    
    verGenerelt    = false;
     
    errorMsg.erase();

    if(bufrData.size()==0){
      errorMsg="No data!";
      return false;
    }

    nTimeStr=bufrData.nContinuesTimes();
    
    sisteTid=*bufrData.begin();
    IBufrDataList it=bufrData.end();
    it--;

	 {
     		precipitationParam=NoPrecipitation;
      	StationInfo::TStringList precip=info->precipitation();
     
      	if(precip.size()>0){
			string rr=*precip.begin();
	
			if(rr=="RA"){
	 			 precipitationParam=PrecipitationRA;
			}else if(rr=="RR_1" || rr=="RR"){
	  			precipitationParam=PrecipitationRR;
			}else if(rr=="RR_3" || rr=="RR_6" 
		  			 || rr=="RR_12" || rr=="RR_24"){
	  			precipitationParam=PrecipitationRR_N;
			}else if( rr=="RRRtr"){
	  			precipitationParam=PrecipitationRRR;
			}else if( rr=="RR_01"){
	  			//NOT implemented (Not needed) yet
			}
      	}
    }

    if(precipitationParam==NoPrecipitation)
      	ir=4;
    else
      	ir=1;

  
    LOGDEBUG("nTimeStr (cont): " << nTimeStr << endl <<
	    	 "Tot times:       " << bufrData.size() << endl <<
	     	 "sisteTid:        " << sisteTid.time() << endl <<
	    	 "fï¿½rsteTid:       " << it->time() << endl <<
	    	 "nedbor12t:       " << sisteTid.RR_12 << endl <<
	    	 //	     "nedbormen:       " << sisteTid.nedboermengde << endl <<
		     "verTillegg:      " << sisteTid.verTillegg << endl <<
		     "nedboerTot (RA): " << sisteTid.RA);
    
  
    sprintf(tmp,"%02d%02d",sisteTid.time().day(), sisteTid.time().hour() );
    dato_tid=tmp;
    
    Sjekk_Gruppe(4,verTilleggKode,sisteTid.verTillegg);
    
    if(ir==1){
      	doPrecip(RRRtr,
				      verTilleggKode,
		   			  rr24Kode,
		   			  ITR,
		    		  bufrData);
    }

    LOGDEBUG("Etter doNedboerKode:"
	    	 << "\n   RRRtr:            " << RRRtr
	    	 << "\n   verTilleggKode:   " << verTilleggKode
	    	 << "\n   stasjonHarNedbï¿½r: " << (ir==1?"true":"false")
	    	 << "\n   ir:               " << ir
	    	 << "\n   ITR:              " << ITR);

	 // 7wwW1W2
    verGenerelt=doVerGenerelt(verGenereltKode, ix, sisteTid);
       
    /* Lagar tidskoda SM|SI|SN */
    Tid_Kode(tidsKode, sisteTid.time().hour());
    
    /* Lufttemperatur i bufr */
    Temp_Kode(luftTempKode, sisteTid.TA);

    /* Reknar ut duggtemp. vha. lufttemp. og fuktigheit */
    dewPoint(duggTempKode, sisteTid.TA, sisteTid.UU);
 
    /* Reknar ut nattens min.temp ELLER dagens max.temp  */
    minMax(minMaxKode, bufrData);

    /* Reknar ut nattens max ELLER dagens min */
    maxMin(maxMinKode, bufrData);

    if(sisteTid.DD!=FLT_MAX && sisteTid.FF!=FLT_MAX){
      	Naa_Vind_Kode(naaVindKode, 
			   	      sisteTid.DD,
		  			  sisteTid.FF);
    }else{
      	naaVindKode="////";
    }
    
    /* Reknar ut max. vindgust kode sidan forrige hovud obs. (12t int.) */
    maxWindGust(maxVindGustKode, bufrData);

    /* Reknar ut max. vindkast kode sidan forrige hovud obs. (6t int.) */
    Max_Vind_Max_Kode(maxVindMaxKode, bufrData);

      //Regner ut gressTempereaturen
    GressTempKode(gressTemp, bufrData);

    pressure(4,trykkQFFKode, sisteTid.PR);
    pressure(3,trykkQFEKode, sisteTid.PO);
    
    if(!sisteTid.AA.empty() && sisteTid.PP!=FLT_MAX ){
      	pressureTrend(tendensKode, sisteTid);
    }else if(nTimeStr>=4){
		pressureTrend(tendensKode,
				     sisteTid.time().hour(),
		    		 bufrData[3].PO,
		    		 bufrData[2].PO,
		    		 bufrData[1].PO,
		    		 sisteTid.PO);
    }

  
    cloudCower(skydekkeKode, sisteTid.skydekke);
    Hoyde_Sikt_Kode(hoyde_siktKode, sisteTid);
    //SjekkEsss(snoeMarkKode, sisteTid.snoeMark);
    doEsss( snoeMarkKode, sisteTid );
    Sjekk_Gruppe(8, skyerKode, sisteTid.skyer);
    Sjekk_Gruppe(8, skyerEkstraKode1, sisteTid.skyerEkstra1);
    Sjekk_Gruppe(8, skyerEkstraKode2, sisteTid.skyerEkstra2);
    Sjekk_Gruppe(8, skyerEkstraKode3, sisteTid.skyerEkstra3);
    Sjekk_Gruppe(8, skyerEkstraKode4, sisteTid.skyerEkstra4);
    Sjekk_Gruppe(0, sjoeTempKode, sisteTid.sjoeTemp);
    
    if( ! test ) {
    	bufr="\r\r\nZCZC\r\r\n";
    	bufr+=tidsKode;
    	bufr+="NO";
    	sprintf(tmp,"%02d ", listenummer);
    	bufr+=tmp;
    
    	while(tmpUtsteder.length()<4)
    		tmpUtsteder.insert(0," ");

		tmpUtsteder+=" ";
		bufr+=tmpUtsteder;
		bufr+=dato_tid;
		bufr.append("00");

		if(create_CCA_template)
			bufr+=" CCCXXX";
    }

    if(ir==1){
      	if(!(sisteTid.time().hour()%6) && ITR>=1 && ITR<=4)
			nedboerKodeSect1=RRRtr;
      	else if(RRRtr.find("////")!=string::npos)
			ir=4;
      	else{
			ir=2;
			nedboerKodeSect3=RRRtr;
      	}
    }

  
    /**
     * Changed the wind unit from knop to m/s
     * 
     * AAXX DDhhW 
     * 
     * Where DD is the day.
     *       hh is the termin (hour).
     *       W  wind unit, 
     *          W = 4 - knop
     *          W = 1 - m/s
     * 
     * IW is defined in bufr.h
     */
    sprintf(tmp,"\r\nAAXX %s%1d", dato_tid.c_str(), IW);
    bufr+=tmp;
      
    sprintf(tmp, "\r\n%05d %1d%1d", wmono, ir, ix);
    bufr+=tmp;
    bufr+=hoyde_siktKode;
    bufr+=string(" ")+skydekkeKode+naaVindKode;
    bufr+=" 1";
    bufr+=luftTempKode+duggTempKode;

    bufr+= trykkQFEKode+trykkQFFKode+tendensKode;
    
    if(ir==1 && !nedboerKodeSect1.empty())
      	bufrStr+=nedboerKodeSect1;
    
    if(verGenerelt)
		bufrStr+=verGenereltKode;
    
    if(skyerKode.length()>0 && 
       skydekkeKode[0]!='0' && 
       skydekkeKode[0]!='9')
		bufrStr+=skyerKode;
    
    
    //Seksjon 222
    //Kyststajoner som fï¿½r beskjed
    if(sisteTid.time().hour()==12){
      	if(SjoeTempKode(sjoeTempKode, sisteTid)){
			bufrStr+=" 222// ";
			bufrStr+=sjoeTempKode;
      	}
    }
    
    
    /**
     * COMMENT:
     * komentert ut rr24Kode intil vi bestemmer oss 
     * for ï¿½ ha den med.
     */
    if(!minMaxKode.empty()       ||
       !nedboerKodeSect3.empty() ||
       !maxVindGustKode.empty()  ||
       !snoeMarkKode.empty()     ||
       !rr24Kode.empty()         ||
       !skyerEkstraKode1.empty() ||
       !skyerEkstraKode2.empty() ||
       !skyerEkstraKode3.empty() ||
       !skyerEkstraKode4.empty()){
      	/* Gruppe 333 */
      	bufrStr+=" 333 ";
      	bufrStr+=minMaxKode;
      	bufrStr+=snoeMarkKode;
      	bufrStr+=nedboerKodeSect3;
      	bufrStr+=rr24Kode;
      	bufrStr+=skyerEkstraKode1;
      	bufrStr+=skyerEkstraKode2;
      	bufrStr+=skyerEkstraKode3;
      	bufrStr+=skyerEkstraKode4;
      	bufrStr+=maxVindGustKode;
    }
    
    if(!maxVindMaxKode.empty() || 
       !maxMinKode.empty()     || 
       !verTilleggKode.empty() ||
       !gressTemp.empty()){
      	/* Gruppe 555 */
      	bufrStr+=" 555 ";
      	bufrStr+=maxVindMaxKode;
      	bufrStr+=maxMinKode;
      	bufrStr+=gressTemp;
      	bufrStr+=verTilleggKode;
    }

    // Avslutting av data-streng 
    bufrStr+="=";
   
    // Datastrengen kan ikke vere lenger enn 69 karakterar 
    SplittStreng(bufrStr, 69);
    
    bufr+=bufrStr;
    
    if( ! test ) {
    	bufr+="\r\n\r\r\n\n\n\n\n\n\n\nNNNN\r\n";
    }

    return true;
}
#endif





/*
** Returnerar  ein streng ; anten "SM","SI",el. "SN"
*/
void 
Bufr::Tid_Kode(std::string &kode, int time)
{
   switch(time){
   case 0:
   case 6:
   case 12:
   case 18:
     	kode="SM";
      	break;
   case 3:
   case 9:
   case 15:
   case 21:
      	kode="SI";
      	break;
   default:
      	kode="SN";
      	break;
   }
   return;
} /* LagTidKode */





/*****
 * Bxrge Moe, 20.12.96
 * Bugfix:
 *   Avrundingsfeil. 'vindretningen' ble feil avrundet.
 *   Dette kan tyde paa en bug i matterutinen (math.h) 'rint'. 
 *   0.5 ble avrundet til 0, og ikke 1 som forventet. Denne feilen gjelder 
 *   bare for argument minde enn 1. For argument > 1, ble resultatet korrekt.
 * 
 * Jeg har ogsaa lagt inn en test om vindhastigheten er stoerre enn 98 m/s.
 * Hvis det er tilfellet antas det feil i maalingen. Det samme gjoer vi 
 * dersom vindretningen er stoerre enn 360 grader. Dette kan virke 
 * unoedvendig siden dette ogsaa blir testet og skal vaere korrigert paa 
 * stasjonen (Grensefeil). Men det kan forekomme grensefeil i dataene 
 * fra stasjonene. 
 *
 * Bxrge Moe, 26.06.97
 * Lagt inn korrigering for maaleunoeyaktighet i minimumsverdiene for
 * FF og DD.
 *     FF < FF(min) -> FF=0.0
 *     DD < DD(min) -> DD=360
 *
 * FF(min) og DD(min) er satt til
 *    FF(min)=0.1 m/s    og    DD(min)=1 gr
 *
 * Dette i hennhold til anbefalinger fra instrument avdelingen. 
 * Ved Ragnar Breakkan.
 *
 * Foelgende grenser er definert for automatstasjoner:
 *  vindhastighet: [0,98]  (m/s)
 *  vindretning:   [0,360] (grader)
 *
 * Bufrkode gruppe 3 har foelgende form:
 *   Nddff 
 *   Hvor:
 *     N - Samlet skydekke. Angis ikke for automatstasjoner.
 *         (angis som / ). For hybridstasjoner kan denne verdien
 *         tastes inn av opperatoer ("skytitter").
 *    dd - Vindretningen. Angis i nermeste 10'de grad. 
 *         Gyldige verdier [00,36]. Verdien 00 gis for vindstille.
 *    ff - Vindhastighet i knop. Gyldige verdier [0,99]. Hvis vindhastigheten
 *         er mindre enn 1 knop settes dd=00. slik at bufren blir /0000.
 *         Hvis vindhastigheten er stoerre enn 99 knop. Faar vi en ekstra
 *         gruppe umiddelbart etter. Bufren blir da /dd99 00fff, fff er
 *         vinden i knop.
 *
 * Parameterene 'retn' og 'hast' til funksjonen er gitt i grader og m/s.
 * 'hast' maa omregnes til knop foer den settes i bufr koden.
 *  
 ******/
void 
Bufr::windAtObstime( const BufrData &data, BufrData &res )
{  
   if( data.FF != FLT_MAX ) {
      if( data.FF >= 0 && data.FF <= 98 ) {
         res.FF = static_cast<float>( static_cast<int>( ( data.FF + 0.05 )*10 ) )/10;
         if( res.FF < 1.0 ) { //No wind.
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


#if 0
void
Bufr::Naa_Vind_Kode(std::string &kode, float retn, float hast)
{
  	char tmp[30];
  	
  	kode="////";
  
  	if( hast < 0.1 )
    	hast = 0.0;
  
  	if( retn < 1 )
    	retn = 360;
  
  	if( fabs(retn) > 360){ /* Grensefeil */
    	if(hast > 98 ){     /* Grensefeil */
    		kode="////";
    	}else{
    		hast *=KNOPFAKTOR;  
    		hast = floor( (double) hast+0.5);
      
    		if(hast<1.0)
    			kode="0000";
    		else if(hast >= 99.0){
    			sprintf(tmp,"//99 00%03.0f",hast);
				kode=tmp;
    		}else{
				sprintf(tmp,"//%02.0f",hast);
				kode=tmp;
    		}
    	}
  	}else{
    	if(retn>=5)
    		retn = floor( (double)(retn/10)+0.5 );
    	else   
    		retn = 36;
    
    	if(fabs(hast) > 98 ){     /* Grensefeil */
    		sprintf(tmp,"%02.0f//",retn);
    		kode=tmp;
    	}else{
    		hast *= KNOPFAKTOR;
    		hast = floor( (double) hast+0.5);
      
    		if(hast<1.0)
				kode="0000";
    		else if(hast>=99.0){
				sprintf(tmp,"%02.0f99 00%03.0f",retn,hast);
				kode=tmp;
    		}else{
				sprintf(tmp,"%02.0f%02.0f",retn,hast);
				kode=tmp;
    		}
    	}
  	}
}
#endif


/*
** Lagar temperaturkode pï¿½ format SnTTT der Sn er forteikn
*/
void 
Bufr::Temp_Kode(std::string &kode, float temp)
{
 	char stmp[30];
  
  	kode="////";
  
  	/* Ugyldige verdiar */
  	if(temp==FLT_MAX)
    	return;
  
  
  	if((temp>=0.0)&&(temp<200.0)){
    	/* Positiv verdi (0 som prefix)*/
    	sprintf(stmp,"0%03.0f",fabs(rint((double)temp*10)));
  	}else if((temp<0.0)&&(temp>-200.0)){
    	/* Negativ verdi (1 som prefix)*/
    	sprintf(stmp,"1%03.0f",fabs(rint((double)temp*10)));
  	}else{
    	return;
  	}

  	kode=stmp;
} /* Temp_Kode */


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
 * -Buggfix for feil i behandlingen av fuktighet stï¿½rre enn 100%.
 * 
 * Bxrge Moe, 2006-07-12
 * -NÃ¥r duggpunktet beregnes marginalt hÃ¸yere, 0.5 grader C, enn 
 *  lufttemperaturen sÃ¥ settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tÃ¥pelig og ulogisk kode.
 *  
 */
void 
Bufr::dewPoint(  const BufrData &data, BufrData &res  )
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

   fukt = data.UU;
   temp = data.TA - 273.15;
   index = 0;

   if(fukt==FLT_MAX || temp==FLT_MAX)
     	return;

   
   LOGDEBUG("dewPoint: UU=" << fukt << "  TA=" << temp);

  	if(fukt>100.0){
   	LOGDEBUG("dewPoint: UU(" << fukt << ")>100");
     
		if(fukt<=104)
		   res.TD = data.TA;
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
Bufr::minMax(std::string &kode, BufrDataList &sd)
{
    int         nTimeStr=sd.nContinuesTimes();
    float       min;
    float       max;
    std::string str;

    min =  200.0;
    max = -200.0;

    if(nTimeStr<12){
      	switch(sd[0].time().hour()){
      	case 6:
			if(sd[0].TAN_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAN_12);
			kode=" 2";
			kode+=str;
			return; 
	
      	case 18:
			if(sd[0].TAX_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAX_12);
			kode=" 1";
			kode+=str;
			break;
	
      	default:
			return;
      	}
      
      	return;
    }


    switch(sd[0].time().hour()){
    case 6: /* Nattens minimumstemp. */
		for(int i=0; i<12; i++){
	  		if(sd[i].TAN==FLT_MAX)
	    		return;
	  
	  		if(sd[i].TAN<min)
	     		min=sd[i].TAN;
		}
	
		if(sd[0].TA<min)
	  		min=sd[0].TA;
	
		/* Ved feil paa data vil min ha defaultverdien 200 */
		if(min>199.0)
	    	return;
	
		/* Temperatur kode paa format TTT */         
		Temp_Kode(str, min);
		kode=" 2";
		kode+=str;
		return;
	
    case 18: /* Dagens maksimumstemp. */
		for(int i=0; i<12; i++){
	  		if(sd[i].TAX==FLT_MAX)
	    		return;
	  
	  		if(sd[i].TAX>max)
	    		max=sd[i].TAX;
		}

		if(sd[0].TA>max)
	    	max = sd[0].TA;         

		/* Ved feil paa data vil max ha defaultverdien -200 */
		if(max<-199.0)
	    	return;
	
		Temp_Kode(str, max);
		kode=" 1";
		kode+=str;
		return;
	
    	default:
      		return;
    }
} /* Min_Max_Kode */


/* 25.11.97
 * Bxrge Moe
 *
 * Regner ut nattens maksimumstemperatur hvis klokken er 06,
 * eller dagens minimumstemperatur hvis klokken er 18.
 *
 * Rutinen er endret slik at vi ikke trenger 24 timer tilbake 
 * i tid med data.
 *
 * nTimeStr holder antall timer med data vi har.
 *
 */
void 
Bufr::maxMin(std::string &kode, BufrDataList &sd)
{
    int nTimeStr=sd.nContinuesTimes();
    float min;
    float max;
    std::string str;

    min =  200.0;
    max = -200.0;

    kode.erase();

    if(nTimeStr<12){
      	switch(sd[0].time().hour()){
      	case 6:
			if(sd[0].TAX_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAX_12);
			kode=" 1";
			kode+=str;
			break; 
	
      	case 18:
			if(sd[0].TAN_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str,sd[0].TAN_12);
			kode=" 2";
			kode+=str;
			break; 
	
      	default:
			return;
      }
      
      return;
    }
    
    switch(sd[0].time().hour()){
    case 6:
      	for(int i=0; i<12; i++){
			if(sd[i].TAX==FLT_MAX)
	  			return;
	
		if(sd[i].TAX>max)
	  		max = sd[i].TAX;
      	}
      
      	if(sd[0].TA>max)
			max = sd[0].TA;         
      
      	if(max<-199.0)
			return;
            
      	Temp_Kode(str,max);
      	kode=" 1";
      	kode+=str;
      	return;
      
	case 18:
      	for(int i=0; i<12; i++){
			if(sd[i].TAN==FLT_MAX)
	  			return;
	
			if(sd[i].TAN<min)
	  			min=sd[i].TAN;
      	}
      
      	if(sd[0].TA<min)
			   min=sd[0].TA;         
        
      	if(min>199.0)
			return;
            
      	Temp_Kode(str,min);
      	kode=" 2";
     	kode+=str;
      	return;
      
    default:
    	return;
    }
} /* Max_Min_Kode */


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
 *       ff - 10 minutters middel. Beregnet 10 minutter foer hver hele time.
 *       fx - er hoeyeste glidende 10 minutters middelverdi i en 69 minutters
 *            periode. ( (tt-2):51-tt:00, tt angir timen for beregningen.)
 *       fg - er hoeyeste glidende 3 sekunders middelverdi i loepet av en
 *            60 minutters periode. ((tt-1):00-tt:00, tt - timen for beregn.)
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
 * Koden for FG er gitt i rutinen 'Max_Vind_Gust_Kode' og koden for
 * FX er gitt i rutinen 'Max_Vind_Max_Kode'.
 */

/* 16.01.98
 * Bxrge Moe
 *
 * Bufrgruppe: 333 911ff  (FG)
 *
 * Beregner maksimalt vindkast siden forrige hovedbufrtid.
 *
 * nTimeStr er en global variabel som holder antall timer med data vi har.
 * nTimeStr settes  i rutinen LagTabell.
 *
 * 'tab' er en array paa 24 element. nTimeStr elementer er gyldig. For aa
 * kunne beregne FG trenger vi 6 timestrenger.
 *
 * 13.03.98 
 * Bxrge Moe
 * Lagt til stoette for manuell intasting av av FG. Den intastede verdien
 * ligger i variabelen _fg (DATASTRUCTTYPE1._fg) og er gitt i knop. Hvis
 * _fg ikke er gitt er lengden av _fg lik 0. (_fg er deklarert som en char _fg[20]).
 *
 * Hvis det er nok time verdier til aa beregne FG, har disse
 * prioritet forran den manuelle hvis den finnes.
 */

/* 18.11.98
 * Bxrge Moe
 *
 * Rettet bugg for generering av Gust for 'pio'.
 */ 
void 
Bufr::maxWindGust(std::string &kode, BufrDataList &sd)
{
    int   nTimeStr=sd.nContinuesTimes();
    float fMax;
    std::string::iterator it;
    char  stmp[30];

    fMax=-1.0;
    kode.erase();
    
    if((sd[0].time().hour())%6 != 0)
		return;

    if(nTimeStr<6){
     	if(sd[0].FG==FLT_MAX || sd[0].FG<0)
     		return;

      fMax=sd[0].FG;
      fMax *= KNOPFAKTOR;
      fMax = floor((double) fMax + 0.5);
      
      if(fMax>=99.0 && fMax<=176){
      	sprintf(stmp, " 91199 00%03.0f", fMax);
      	kode=stmp;
      }else if(fMax<99.0){
      	sprintf(stmp, " 911%02.0f", fMax);
      	kode=stmp;
      }

      return;
    }

    

    for(int i=0; i<6; i++){
      if(sd[i].FG_1==FLT_MAX)
			return;
      
      if(sd[i].FG_1>fMax)
			fMax=sd[i].FG_1;
    }

    if(fMax<0)
      return;
    
    fMax *= KNOPFAKTOR;
    fMax = floor((double) fMax+0.5);
    
    if(fMax>=99.0)
     	sprintf(stmp, " 91199 00%03.0f", fMax);
    else
     	sprintf(stmp, " 911%02.0f", fMax);

    kode=stmp;
}


/* 16.01.98
 * Bxrge Moe
 *
 * Bufrgruppe 555 0STzFxFx
 *
 * Regner ut maksimal middelvind siden forrige hovedtermin,
 * dvs. kl. 0, 6, 12  eller 18, og hvor mange timer siden det inntraff. 
 *
 * nTimeStr angir antall timer med kontinuerlig data vi har.
 *
 *
 * 13.03.98 
 * Bxrge Moe
 *
 * Lagt til stoette for observert TzFXFX. De intastede verdien
 * ligger i variabelene ITZ og FX. FX er gitt i 
 * m/s.  
 *
 * Hvis det er nok time verdier til aa beregne tzFXFX, har disse
 * prioritet forran den observerte hvis den finnes.
 *
 * 2005.11.24
 * Bxrge Moe
 * Lagt inn stÃ¸tte for FX_3.
 */
void 
Bufr::maxWindMax( BufrData::Wind &wind, BufrDataList &sd)
{
   int   nTimeStr=sd.nContinuesTimes();
   int   nNeedTimes;
   float fMax;
   int   iMax;
   int   iNaaMax;
   int   iMaxIndex;
   int   iTidsAngiv;
   int   i;
   char  cTid;

   nNeedTimes=3;
   fMax=-1.0;

   if((sd[0].time().hour())%3 != 0)
      return;
       
   if((sd[0].time().hour())%6 == 0)
      nNeedTimes=6;
    
   if(nTimeStr < nNeedTimes){
      fMax=FLT_MAX;
    	
      if(sd[0].FX!=FLT_MAX && sd[0].FX>=0){
         fMax=sd[0].FX;
      }else if(sd[0].FX_3!=FLT_MAX && sd[0].FX_3>=0){
         fMax=sd[0].FX_3;
    		
         if((sd[0].time().hour()%6)==0){ //Hovedtermin!
            miutil::miTime prevTime=sd[0].time();
            prevTime.addHour(-3);
            CIBufrDataList it=sd.find(prevTime);
    			
            if(it!=sd.end() && it->time()==prevTime &&
               it->FX_3!=FLT_MAX && it->FX_3>=0){
    			 
               if(it->FX_3>fMax)
                  fMax=it->FX_3;
            }else{
               fMax=FLT_MAX;
            }
         }
      }
    			   
      if(fMax==FLT_MAX){
         return;
      }

      fMax*=KNOPFAKTOR;
      
      if( sd[0].ITZ != FLT_MAX )
         wind.i = sd[0].ITZ;

      //Guard against rounding error.
      fMax = floor( (double) fMax+0.5);
      
      if(fMax>=0 && fMax<176){
         wind.ff = fMax;
      }

      if( wind.ff == FLT_MAX || wind.i == FLT_MAX )
         wind = BufrData::Wind();   //Reset to undefind.

      return;
   }
    
   i=nNeedTimes-1;
    
   if(sd[i].FX_1==FLT_MAX )
      return;
    
   fMax=sd[i].FX_1;
   iMaxIndex=i;
   i--;
    
   while(i>=0){
      if(sd[i].FX_1==FLT_MAX){
         return;
      }
      
      if(sd[i].FX_1>=fMax){
         fMax=sd[i].FX_1;
         iMaxIndex=i;
      }

      i--;
   }
   
   if(fMax<0)
      return;

   if(iMaxIndex<3)
      iTidsAngiv=iMaxIndex+1;
   else if(iMaxIndex<6)
      iTidsAngiv=4;
   else if(iMaxIndex<9)
      iTidsAngiv=5;
   else if(iMaxIndex<12)
      iTidsAngiv=6;
   else{
      return;
   }

   if( fMax <= sd[0].FF ) {
      iTidsAngiv = 0;
      fMax = sd[0].FF;
   }

   iMax = (int)floor((double) fMax + 0.5);
   iNaaMax = (int) floor((double) sd[0].FF + 0.5 );
    
   wind.ff = iMax;
   wind.i  = iTidsAngiv;
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
Bufr::pressure( float pressure )
{
   double dTrykk;

   if( pressure == FLT_MAX)
      return FLT_MAX;

   if(pressure < 800.0 || pressure > 1100.0)
      return FLT_MAX;
        
   return pressure * 100;
} /* Trykk_Kode */


void
Bufr::doPressureTrend( const BufrDataList &data, BufrData &res )
{
   bool ok = pressureTrend( data[0], res );

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
Bufr::computePressureTrend( const BufrDataList &data,
                     BufrData &res )
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

   a      = 4;
   dP1    = trykk2 - trykk1;
   dP3    = trykk4 - trykk3;
   PD3    = trykk4 - trykk1;
   t1     =    dP3 - dP1;
   lim    = 0.01;

   /* Ved ulovlege verdiar */
   if( trykk1==FLT_MAX ||
       trykk2==FLT_MAX ||
       trykk3==FLT_MAX ||
       trykk4==FLT_MAX){
      /* Bxrge Moe
       * 5.8.2002
       *
       * Endret return 'kode' til en tom streng nï¿½r verdiene
       * er ugyldig.
       */
      //kode=" 5////";
      return;
   }

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
   res.PP = pressure( PD3 );
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
Bufr::pressureTrend( const BufrData &data, BufrData &res)
{
   float trend = FLT_MAX;

  	if( data.AA != FLT_MAX )
    	trend = data.AA;
  
  
  	if(data.PP == FLT_MAX){
    	if( trend == FLT_MAX )
      		return false;
    
    	res.AA = trend;
    	return true;
  	}

  	res.AA = trend;
  	res.PP = pressure( data.PP );
  	return true;
}





/*
**
*/
void 
Bufr::cloudCower( const BufrData &data, BufrData &res )
{
   int N;
   if( data.N == FLT_MAX )
      return;
   N = static_cast<int>( data.N );

   if( N == 9  )
      res.N = 113;
   else if( N == 0 )
      res.N = 0;
   else
      res.N = N/8 * 100;
} /* Skydekke_Kode */

/*
**
*/
void 
Bufr::Hoyde_Sikt_Kode(std::string &kode, const BufrData &data)
{
   float Vmor=data.Vmor;
   float VV=data.VV;
   float HLN=data.HLN;
   float HL=data.HL;

   kode="///";
   
	if( VV == FLT_MAX && Vmor!=FLT_MAX )
	   VV = Vmor;
	
	if( HL == FLT_MAX && HLN != FLT_MAX )
	   HL = HLN;
	   
	if( HL != FLT_MAX ) {
	   char ch=decodeutility::HLKode( HL );
	   kode[0]=ch;
	}
	
	if( VV != FLT_MAX ) {
	   string s=decodeutility::VVKode( VV );
	   
	   if( ! s.empty() )
	      kode.replace(1, 2, s);
	}
}


/*
**
*/
int 
Bufr::ix_Kode(const std::string &str)
{
  	if(str.length()!=2 || !(isdigit(str[1])))
    	return 6;
  
  	return atoi(&(str.c_str())[1]);

} /* ix_Kode */


/**
 * Funksjonen lager koden 7wwW1W2 i seksjon 1, samt setter ix.
 * Manuelt observert ww gÃ¥r foran automatisk generert ww.
 * 
 * Den manuelt observerte ww ligger i datasettet som verGenerelt og
 * den automatiske ligger som WAWA.
 *  
 */
bool 
Bufr::doGeneralWeather(BufrData &res, const BufrData &data)
{
	bool verGenerelt;
	
	
	if( data.IX == FLT_MAX )
	   res.IX = 6;

	verGenerelt = data.ww != FLT_MAX && data.W1 != FLT_MAX && data.W2 != FLT_MAX;
    
   if(verGenerelt){
   	res.IX = 1;
   }else if( data.WAWA!=FLT_MAX ){
   	int i=static_cast<int>(round( data.WAWA) );
   	
   	if(i>=0 && i<100){
   	   res.ww = i;
    		res.IX = 7;
    		verGenerelt=true;
   	}else{
   		res.IX = 6;
   	}
   }
   
   if( static_cast<int>( res.IX ) == 1 && !verGenerelt)
   	res.IX=6;
   
   return verGenerelt;
}	
 

/**
 * Bï¿½rge Moe
 * 1999.06.10
 *
 * Endringer av tolkning av Esss gitt av klimaavdelingen.
 * En Esss kode pï¿½ formen /000 skal ikke tas med i bufren
 * men den angir en lovlig verdi for koden. I fï¿½lge bufr
 * kodingen (spec) er ikke sss=0 en lovlig verdi, men er tillatt
 * slik at man kan sende Esss hele ï¿½ret, ogsï¿½ nï¿½r det ikke er
 * snï¿½.
 */
bool 
Bufr::SjekkEsss(std::string &kode, const std::string &str)
{
  	std::string::const_iterator it;
  
  	if(str.length()!=4 || str=="////" || str=="/000"){
    	kode="";
    	return false;
  	}else{
    	it=str.begin();
    
    	while(it!=str.end()){
     		if(!(isdigit(*it) || *it == '/')){
				kode="";
				return false;
     		}
     		it++;
    	}
    
    	kode=" 4";
    	kode+=str;
  	}
  
  	return true;
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
doEsss( std::string &kode, const BufrData &data )
{
   kode.erase();
   
   char buf[16];
   string em;
   string sa;
   int    iSA;
   
   if( data.SA == FLT_MAX )
   	iSA = INT_MAX;
   else
   	iSA = (int) floor((double) data.SA + 0.5 );
   
   if( data.EM == FLT_MAX && iSA == INT_MAX )
      return;
   
   if( data.EM == FLT_MAX || data.EM < 0 || data.EM > 10 )
      em = "/";
   else {
      sprintf( buf, "%01.0f", data.EM );
      em = buf;
   }
   
   if( iSA == INT_MAX  || iSA < -3 || iSA > 996 )
      sa = "///";
   else if( iSA == -1 ) {
	   if( em =="/" )
		   sa = "///";
	   else
		   sa = "998";
   }else if( iSA == 0 )
	   sa = "997";
   else if( iSA == -3 )
	   sa = "999";
   else if( iSA < 0 )
	   sa="///";
   else {
      sprintf( buf, "%03d", iSA );
      sa = buf;
   }

      
   //Creates the code 4E'sss
   kode = " 4" + em + sa;
}


/*
** Sjekkar kode for grupper med 4 teikn;
** maa innehalde anten tal eller '/'
*/
bool
Bufr::Sjekk_Gruppe(int grpNr, std::string &kode, const std::string &str)
{
  	std::string::const_iterator it;
  	char tmp[30];
  
  	//std::cerr <<  "Sjekk_Gruppe (in) grpnr(" << grpNr << "): " << str << "\n";  

  	if(str.length()!=4 || str=="////"){
    	kode="";
    	//std::cerr <<  "Sjekk_Gruppe (err1): \n";  
    	return false;
  	}else{
    	it=str.begin();
    	while(it!=str.end()){
      
     		if(!isdigit(*it) && *it != '/'){
				kode="";
				//std::cerr <<  "Sjekk_Gruppe (err2): \n";  
				return false;
     		}
      		it++;
    	}
  
    	sprintf(tmp," %1d%4s",grpNr,str.c_str());
    	kode=tmp;
  	}
  
  	//std::cerr <<  "Sjekk_Gruppe (ut): " << kode << "\n";  
  	return true;

} /* Sjekk_Gruppe */

/*
** Sjekkar den manuelt inntasta koda "ir"
*/
int 
Bufr::Vis_ir_Kode(const std::string &str)
{
   	std::string s;
   	std::string::iterator it;

   	s=str;
   	it=s.begin();
   
   	if(it==s.end())
    	return 4;
   
   	if(s.size()!=2 || !(isdigit(*it)))
      	return 4;

   	s.erase(1);

   	return atoi(s.c_str());

} /* Vis_ir_Kode */

bool 
Bufr::seaTemp( const BufrData &data, BufrData &res)
{
  
  	if(data.time().hour()!=12 || data.TW == FLT_MAX ) {
  	   res.TW = FLT_MAX;
  	   return false;
  	}
  
  	return true;
}

/**
 * GressTemperaturen angis klokken 06, og angir minimums
 * temperaturen de siste 12 timene. 
 * For automatiske mï¿½lteverdier trenger vi 12 timestrenger,
 * parameteren har navnet TGN.
 *
 * For manuelt mï¿½lt verdi har parametern navnet TGN_12 og
 * vi trenger bare en timestreng.
 */
void 
Bufr::GressTempKode(std::string &kode, BufrDataList &sd)
{
  	int nTimeStr=sd.nContinuesTimes();
  	float min;
  	int i;
  	std::string str;
  
  	min=FLT_MAX;
  
  	kode.erase();
  
  	if(sd[0].time().hour()!=6)
    	return;
  
  	if(nTimeStr<13){
    	if(sd[0].TGN_12==FLT_MAX)
      		return;
    
    	Temp_Kode(kode, sd[0].TGN_12);
    	kode.insert(0, " 3");
    
    	return;
  	}
  
  	i=0;
  
  	while(i<12){
    	if(sd[i].TGN!=FLT_MAX && sd[i].TGN<min)
      		min=sd[i].TGN;
    
    	i++;
  	}
  
  	if(min==FLT_MAX)
    	return;
  
  	Temp_Kode(kode, min);
  	kode.insert(0, " 3");
  
  	return;
}


/*
 * Funksjonen lagar nedboerkoda for nedboermengde i perioden.
 *
 * Lagt til ir for ï¿½ styre kodingen av RRRtr.
 * 
 * ir==1, det har falt mï¿½lbar nedbï¿½r.
 * ir==3, tï¿½rt, det har ikke falt nedbï¿½r, kode ==> 6000tr 
 * ir==4, 
 */

void
Bufr::precip(std::string &kode,  //RRRtr
		    		std::string &verTilleggKode, //555 ... 4RtWdWdWd
		    		std::string &rr24Kode,       //333 ... 7RR24
		    		float totalNedboer, 
		    		float fRR24,
		    		int time,
		    		int &tr,
		    		int ir)
{
  	double dummy;
  	float  nedboerTiDel=0.0;
  	char   stmp[30];

  	kode.erase();
  	rr24Kode.erase();

  	if(time==6){
    	//Skal vi kode 24 (7RR24) timers nedbï¿½r i 333 seksjonen 
    	if(fRR24!=FLT_MAX){
      		if( fRR24 == FLT_MIN ) //FLT_MIN signals trace of precipitation
      			rr24Kode=" 79999";
      		else if(fRR24<0){
				rr24Kode=" 70000";
      		}else{
				fRR24*=10;

				if(fRR24>9999.5){
	  				rr24Kode=" 7////";
				}else{
	  				sprintf(stmp," 7%04.0f",fabs(floor((double)fRR24+0.5)));
	  				rr24Kode=stmp;
				}
      		}
    	}else{
      		rr24Kode=" 7////";
    	}
  	}
	  
  	if(tr<0 || tr>9){
    	if((time==0)||(time==12))
      		tr=1;
    	else if((time==6)||(time==18))      
      		tr=2;
    	else
      		tr=5;
  	}
  
  	if(verTilleggKode.length()!=6)
    	verTilleggKode=" 4////";

  	if(ir==1){
    
    	//Ugyldig verdi
    	if(totalNedboer==FLT_MAX || totalNedboer<=-2.0){
      		kode=" 6////";
      		return;
    	}else if(totalNedboer<0.0){ //Tï¿½rt
      		sprintf(stmp, " 6000%1d", tr);
      		kode=stmp;
      		verTilleggKode[2]='0';
      		return;
    	}else if(totalNedboer<1.0){
      		sprintf(stmp," 699%1.0f%1d",fabs(floor((double)totalNedboer*10+0.5)),tr);
    	}else if((totalNedboer>=1.0)&&(totalNedboer<989.0)){
      		sprintf(stmp," 6%03.0f%1d",fabs(floor((double)totalNedboer+0.5)), tr);
    	}else{ // totalNedboer>=989.0
      		sprintf(stmp," 6989%1d", tr);
    	}

    	kode=stmp;
    	nedboerTiDel = 10*modf((double)totalNedboer, &dummy);
    
    	//verTilleggKode er pï¿½ formen:
    	// ' 4RtWdWdWd', det er Rt vi skal sette. Rt har indeks=2.
    	//  012 3 4 5 , indekser.
    	sprintf(stmp, "%1.0f", nedboerTiDel);
    	verTilleggKode[2]=stmp[0];
  	}else if(ir==3){
    	sprintf(stmp, " 6000%1d", tr);
    	kode=stmp;
    	verTilleggKode[2]='0';
  	}else if(ir==4){
    	kode=" 6////";
  	}else{
    	LOGWARN("Nedboer (6RRRtr): Unknown ir <" << ir << ">!");
    	kode=" 6////";
  	}
} /* Nedboer_Kode */



void
Bufr::doPrecip( StationInfoPtr     info,
                const BufrDataList &bufrData,
                BufrData           &bufr )
{
  	BufrData      sisteTid;
  	ostringstream ost;
  	int           ir;
  	float         nedboerTotal=0.0;
  	float         fRR24=FLT_MAX;
  	int           tr=-1;

  	precipitationParam=NoPrecipitation;
  	StationInfo::TStringList precip=info->precipitation();

  	if( precip.size() > 0 ){
  	   string rr=*precip.begin();

  	   if(rr=="RA"){
  	      precipitationParam=PrecipitationRA;
  	   }else if(rr=="RR_1" || rr=="RR"){
  	      precipitationParam=PrecipitationRR;
  	   }else if(rr=="RR_3" || rr=="RR_6"
  	         || rr=="RR_12" || rr=="RR_24"){
  	      precipitationParam=PrecipitationRR_N;
  	   }else if( rr=="RRRtr"){
  	      precipitationParam=PrecipitationRRR;
  	   }else if( rr=="RR_01"){
  	      //NOT implemented (Not needed) yet
  	   }
  	}

  	if(precipitationParam==NoPrecipitation)
  	   return;


  	sisteTid = *bufrData.begin();

  	ost << "doPrecip: sisteTid: " << sisteTid.time() << endl;;

  	if(precipitationParam==PrecipitationRA){
    	ir = precipFromRA(nedboerTotal, fRR24, tr, bufrData);
    	ost << "doPrecip: EPrecipitationParam: RA    (Automatisk)" << endl
			<< "    Nedbør bereggning Ir: " << ir  << endl
			<< "                   RR_24: " << fRR24          << endl
			<< "                  nedbør: " << nedboerTotal   << endl
			<< "                      tr: " << tr             << endl;
  	}else if(precipitationParam==PrecipitationRR){
    	ir = precipFromRR(nedboerTotal, fRR24, tr, bufrData);
    	ost << "doPrecip: EPrecipitationParam: RR    (Automatisk)" << endl
			<< "  Nedbør bereggning Ir: " << ir
			<< "                nedbør: " << nedboerTotal << endl;
  	}else if(precipitationParam==PrecipitationRR_N){
    	ost << "doPrecip: EPrecipitationParam: RR_N, hvor N=1,3,6,12,24" << endl;
    	ir = precipFromRrN( nedboerTotal, fRR24, tr ,bufrData );
    	//tr=4;
    	ost << "  Nedbør bereggning Ir:" << ir
			<< "                 nedbør: " << nedboerTotal << endl;
  	}else if(precipitationParam==PrecipitationRRR){
    	ost << "doPrecip: PrecipitationParam: RRR  (Manuell)" << endl;
    	ir=precipFromRRRtr(nedboerTotal, fRR24, tr, bufrData);
    	ost << "    Nedbør bereggning Ir: " << ir  << endl
          << "                   RR_24: " << fRR24          << endl
          << "                  nedbør: " << nedboerTotal   << endl
          << "                      tr: " << tr             << endl;
  	}else{
    	ost << "PrecipitationParam: UNKNOWN" << endl;
    	return;
  	}

  	LOGDEBUG( ost.str() );
  	/*
  	precip(nedboerKode, verTilleggKode, rr24Kode, nedboerTotal, fRR24, 
			     sisteTid.time().hour(), tr, ir);
  	 */
}



/**
 * 2003.03.12 Bxrge Moe
 *
 * nedborFromRA, beregner nedbï¿½ren fra RA (Akumulert nedbï¿½r).
 * Nedbï¿½ren beregnes ved ï¿½ ta differansen mellom  RA fra bufrtidspunktet og
 * RA 12 (evt 24) timer tilbake. Dette betyr at nedbï¿½ren bergnes pï¿½ fï¿½lgende
 * mï¿½te for bufrtidspunktene 06, 12, 18 og 24. Bruke notasjonen RA(t) for ï¿½
 * angi bï¿½tteinholdet ved timen t. Eks RA(12) er bï¿½tteinnholdet kl 12.
 * 
 * bufrtidspunkt kl 00 og 12,  6 timers nedbï¿½r:
 *    nedbï¿½r= RA(t)-RA(t-6)
 *
 * bufrtidspunkt kl 6 og 18,  12 timers nedbï¿½r:
 *    nedbï¿½r=RA(t)-RA(t-12)
 *
 * Nedbï¿½ren raporteres bare dersom den er over en gitt grense. For
 * ï¿½yeblikket er den hardjodet til 0.15. Dette kan endres dersom
 * obsdiv finner det nï¿½dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse pï¿½ 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedbï¿½ren for.
 * \return ir
 */
int
Bufr::precipFromRA(float &nedbor, float &fRR24, int &tr, const BufrDataList &sd)
{
  	const float limit=0.2;
  	const float bucketFlush=-10.0;
  	int   nTimes;
  	miutil::miTime t=sd.begin()->time();
  	miutil::miTime t2;
  	BufrData d1;
  	BufrData d2;
  	CIBufrDataList it;

  	int   time=t.hour();

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(time==6 || time==18){
    	tr=2;
    	nTimes=12;
  	}else if(time==0 || time==12){
    	tr=1;
    	nTimes=6;
  	}else{
    	tr=5;
    	nTimes=1;
  	}

  	d1=*sd.begin();

  	if(time==6){
    	//Vi lager en RR_24 verdi til bruk i seksjonen 333 7RR_24
    	t2=t;
    	t2.addHour(-24);
    
    	it=sd.find(t2);

    	if(it!=sd.end() && it->time()==t2){
      		d2=*it;
      
      		if(d1.RA!=FLT_MAX && d2.RA!=FLT_MAX){
				fRR24=d1.RA-d2.RA;
	
				if(fRR24>bucketFlush){
	  				if(fRR24<=limit)
	    				fRR24=0.0;  //Tï¿½rt
					}else{
	  					//Bï¿½tta er tï¿½mt
	  					fRR24=FLT_MAX;
					}
      		}
    	}
  	}

  	t2=t;
  	t2.addHour(-1*nTimes);  

  	it=sd.find(t2);

  	if(it==sd.end())
    	return 4;

  	d2=*it;

  	if(d2.time()!=t2)
    	return 4;


  	if(d1.RA==FLT_MAX || d2.RA==FLT_MAX)
    	return 4;

  	nedbor=d1.RA-d2.RA;

  	LOGDEBUG("bufrTidspunkt:          " << d1.time() << endl
			 << "nedbor=" <<  nedbor << endl
	   		 << " RR_24=" <<  fRR24 << endl
	   		 << "    RA=" <<  d1.RA << endl 
  	   		 << "bufrTidspunkt-" << nTimes << " timer : " << d2.time() << endl
	   		 << "    RA=" <<  d2.RA << endl);

  
  	if(nedbor>bucketFlush){
    	if(nedbor<=limit){
      		nedbor=-1.0;
      		return 3;
    	}
  	}else{ //Bï¿½tta er tï¿½mt
    	nedbor=FLT_MAX;
    	return 4;
  	}

  	return 1;
}

int
Bufr::rr24FromRrN(const BufrDataList &sd, float &fRR24)
{
  	return 4;
}


/*
 * bufrtidspunkt kl 00 og 12,  6 timers nedbï¿½r: RR_6
 * bufrtidspunkt kl 6 og 18,  12 timers nedbï¿½r: RR_12
 * 
 * Sï¿½ker i nedbï¿½rparametrene RR_N, hvor N=1, 3, 6, 12, 24
 * Sï¿½ker nedbï¿½ren fra den fï¿½rste som er gitt, sï¿½ker fra 24, 12, .. ,1
 */
int  
Bufr::precipFromRrN( float &nedbor,
		      	      float &fRR24,
		      		   int &tr,
		      		   const BufrDataList &sd)
{
  	int t=sd.begin()->time().hour();

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(t==6 && sd[0].RR_24!=FLT_MAX)
    	fRR24=sd[0].RR_24;

  	if((t==6 || t==18) && sd[0].RR_12!=FLT_MAX){
    	nedbor=sd[0].RR_12;
    	tr=2;
  	}else if((t==12 || t==0) && sd[0].RR_6!=FLT_MAX){
    	nedbor=sd[0].RR_6;
    	tr=1;
  	}

  	if(nedbor==FLT_MAX){
    	if(sd[0].RR_24!=FLT_MAX){
      		nedbor=sd[0].RR_24;
      		tr=4;
    	}else if(sd[0].RR_18!=FLT_MAX){
      		nedbor=sd[0].RR_18;
      		tr=3;
    	}else if(sd[0].RR_15!=FLT_MAX){
      		nedbor=sd[0].RR_15;
      		tr=9;
    	}else  if(sd[0].RR_12!=FLT_MAX){
      		nedbor=sd[0].RR_12;
      		tr=2;
    	}else if(sd[0].RR_9!=FLT_MAX){
      		nedbor=sd[0].RR_9;
      		tr=8;
    	}else if(sd[0].RR_6!=FLT_MAX){
      		nedbor=sd[0].RR_6;
      		tr=1;
    	}else if(sd[0].RR_3!=FLT_MAX){
      		nedbor=sd[0].RR_3;
      		tr=7; 
    	}else if(sd[0].RR_2!=FLT_MAX){
      		nedbor=sd[0].RR_2;
      		tr=6;
    	}else if(sd[0].RR_1!=FLT_MAX){
      		nedbor=sd[0].RR_1;
      		tr=5; 
    	}
  	}

  	if(nedbor==FLT_MAX)
    	return 4;

  	if(t==6 && fRR24==FLT_MAX && sd.size()>1){
    	CIBufrDataList it=sd.begin();
    	miutil::miTime tt=it->time();
    	
    	tt.addHour(-12);	
    
    	CIBufrDataList it2=sd.find(tt);
    	
    	if(it2!=sd.end() && it2->time()==tt){
    		bool hasPrecip=true;
    		fRR24=0.0;

         //If there is measured no precip in the 12 hour time
         //periode the nedboer12Time=-1.0.
         //We cant just test for for nedboer12Time==-1.0. 

			
			if(it->RR_12!=FLT_MAX){
            if(it->RR_12>=0.0)
	  			   fRR24+=it->RR_12;
            else if(it->RR_12<=-1.5f)
               hasPrecip=false;
         }else if( static_cast<int>(it->IR) != 3)
	  			hasPrecip=false;
	  			
	  		if(it2->RR_12!=FLT_MAX){
            if(it2->RR_12>=0.0)
   	  			fRR24+=it2->RR_12;
            else if(it2->RR_12<=-1.5f)
               hasPrecip=false;
         }else if( static_cast<int>(it2->IR) != 3 )
	  			hasPrecip=false;

			if(!hasPrecip)
				fRR24=FLT_MAX;
    	}
  	}
  
  	if(nedbor<0.1){
    	nedbor=0.0;
    	return 3;
  	}

   	return 1;
}  


/**
 * Nedbï¿½r fra manuelle nedbï¿½rstasjoner. Bruker ITR for ï¿½ sjekke
 * om det er angitt manuell nedbï¿½r. Hvis ITR er satt nedbï¿½ren gitt.
 * Bruker ITR for ï¿½ finne rett nedbï¿½rparameter.
 *
 *  ITR    Nedbï¿½r parameter
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
 * Hvis ITR har en gyldig verdi og nedbï¿½ren er -1 angir dette tï¿½rt.
 */
int  
Bufr::precipFromRRRtr(float &nedbor,
		       float &fRR24, 
		       int   &tr, 
		       const BufrDataList &sd)
{
	nedbor=FLT_MAX;
  	fRR24 =FLT_MAX;

  	if(sd.begin()->time().hour()==6) {
  		CIBufrDataList it=sd.begin();
    	miutil::miTime tt=it->time();
	
    	tt.addHour(-12);	
    
    	CIBufrDataList it2=sd.find(tt);
    	
    	if( it2 != sd.end() && it2->time() == tt ){
    		bool hasPrecip=true;
    		fRR24=-1.0;
			
    		//If there is measured no precip in the 12 hour time
    		//periode the nedboer12Time=-1.0.
    		//We cant just test for for nedboer12Time==-1.0.
         
    		//cerr << "nedboer12Time:  " << it->nedboer12Time << endl
    		//    <<"nedboer12Time2: " << it2->nedboer12Time << endl;
                    
			if( it->RR_12 != FLT_MAX ){
				if( it->RR_12 >= 0.0 ) {
					if( fRR24 < 0  )
						fRR24 = 0.0;

	  			   fRR24+=it->RR_12;
				} else if( it->RR_12 <= -1.5f )
					hasPrecip=false;
			}else if( static_cast<int>( it->IR ) != 3 )
				hasPrecip=false;
	  			
			if( it2->RR_12 != FLT_MAX){
				if( it2->RR_12 >= 0.0) {
					if( fRR24 < 0  )
						fRR24 = 0.0;

					fRR24 += it2->RR_12;
				} else if( it2->RR_12 <= -1.5f)
					hasPrecip=false;
			}else if( static_cast<int>( it2->IR ) != 3 )
				hasPrecip=false;

			if(!hasPrecip)
				fRR24=FLT_MAX;
			else if( FEQ( fRR24, 0.0, 0.001 ) )
				fRR24=FLT_MIN; //Use FLT_MIN to signal trace of precipitation.
    	}
    	
    	if(fRR24==FLT_MAX){
    		//Do we have an RR_24 precip.
    		fRR24=sd.begin()->RR_24;
    		
    		if(fRR24!=FLT_MAX && fRR24<0.0)
    			fRR24=FLT_MAX;
    	}
  	}

   //cerr << "sd[0].ITR: [" << (sd[0].ITR[0]-'0') << "]" << endl;

  	if(sd[0].IR == FLT_MAX){
  		if(sd[0].IR == FLT_MAX || sd[0].IR!=3)
  			return 4;

  		nedbor=0.0;
    	return 3;
  	}

  	switch( static_cast<int>( sd[0].IR ) ){
  	case 1: 
    	if(sd[0].RR_6!=FLT_MAX){
      		nedbor=sd[0].RR_6;
      		tr=1;
     	}
    	break;
  	case 2:
    	if(sd[0].RR_12!=FLT_MAX){
      		nedbor=sd[0].RR_12;
      		tr=2;
     	}
    	break;
  	case 3:
    	if(sd[0].RR_18!=FLT_MAX){
      		nedbor=sd[0].RR_18;
      		tr=3;
   		}
    	break;
  	case 4:
    	if(sd[0].RR_24!=FLT_MAX){
      		nedbor=sd[0].RR_24;
      		tr=4;
     	}
    	break;
  	case 5:
    	if(sd[0].RR_1!=FLT_MAX){
      		nedbor=sd[0].RR_1;
      		tr=5;
     	}
    	break;
  	case 6:
    	if(sd[0].RR_2!=FLT_MAX){
      		nedbor=sd[0].RR_2;
      		tr=6;
    	}
    	break;
  	case 7:
    	if(sd[0].RR_3!=FLT_MAX){
      		nedbor=sd[0].RR_3;
      		tr=7;
     	}
    	break;
  	case 8:
    	if(sd[0].RR_9!=FLT_MAX){
      		nedbor=sd[0].RR_9;
      		tr=8;
    	}
    	break;
  	case 9:
    	if(sd[0].RR_15!=FLT_MAX){
      		nedbor=sd[0].RR_15;
      		tr=9;
    	}
    	break;
  	default:
    	return 4;
  	}

  	if(nedbor==FLT_MAX) 
    	return 4;

  	if(static_cast<int>(round(nedbor))==-1) //tï¿½rt
    	return 3;
 
  	return 1;
}  



/**
 * 2003.03.14 Bxrge Moe
 *
 * nedborFromRR, beregner nedbï¿½ren fra RR (Times nedbï¿½r).
 * Nedbï¿½ren beregnes ved ï¿½ summere RR fra bufrtidspunktet og
 * 12 (evt 6) timer tilbake. 

 * bufrtidspunkt kl 00 og 12,  6 timers nedbï¿½r:
 *    nedbï¿½r= RR(t)+RR(t-1)+ .... +RR(t-6)
 *
 * bufrtidspunkt kl 6 og 18,  12 timers nedbï¿½r:
 *    nedbï¿½r=RR(t)+RR(t-1)+ .... +RR(t-12)
 *
 * Nedbï¿½ren raporteres bare dersom den er over en gitt grense. For
 * ï¿½yeblikket er den hardkodet til 0.15. Dette kan endres dersom
 * obsdiv finner det nï¿½dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse pï¿½ 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedbï¿½ren for.
 * \return ir
 */

int  
Bufr::precipFromRR(float &nedbor, float &fRR24, int &tr, const BufrDataList &sd)
{
  	const float limit=0.2;
  	int   nTimeStr=sd.nContinuesTimes();
  	int   time=sd.begin()->time().hour();
  	int   nTimes;
  	float sum=0;

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(time==6 || time==18){
    	tr=2;
    	nTimes=12;
  	}else if(time==0 || time==12){
    	tr=1;
    	nTimes=6;
  	}else{
    	tr=5;
    	nTimes=1;
  	}
  	
  	if(nTimeStr<nTimes)
    	return 4; 

  	for(int i=0; i<nTimes; i++){
    	if(sd[i].RR_1==FLT_MAX)
      	return 4;
    
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
  
  	LOGDEBUG("bufrTidspunkt:          " << sd[0].time()
			 << "  RR=" <<  sd[0].RR_1 << endl
			 << "bufrTidspunkt-" << nTimes << " timer : "
			 << sd[nTimes-1].time() 
			 << "  RR=" <<  sd[nTimes-1].RR_1 << endl);

  
  	if(nedbor<=limit){
    	nedbor=-1.0;
    	return 3;
  	}

  	return 1;
}


/*
** Funksjonen splittar strengen ved karakter nr. index (ved nï¿½raste space),
** og legg inn linefeed og 6 space'ar ved denne.
*/
void 
Bufr::SplittStreng(std::string &streng, std::string::size_type index)
{
  	std::string tmp;
  	int         i;
  	std::string::size_type t;
  	std::string::size_type p;
  	std::string::size_type n;
  	const char        *seksjon[]={" 222// ",
			                      " 333 ",
			  				      " 444 ",
			  				      " 555 ",
			  				      0};
  
  	i = 0;
  
  	while(seksjon[i]){
    	n=strlen(seksjon[i]);
    	t = streng.find(seksjon[i]);
    
    
    	if(t!=std::string::npos){      
      		tmp=streng.substr(t);
      		p=tmp.find_first_not_of(' ', n);
      
      		if(p!=std::string::npos){
				if(p<7)
	  				tmp.insert(n, 6-n, ' ');
				
				if(p>6)
	  				tmp.erase(7, p-7);
      		}
      
      		streng.erase(t);
      		streng+="\r\r\n     ";
      		streng+=tmp;
    	}
    
   		i++;
  	}
} /* SplittStreng */



Bufr::Bufr(EPrecipitation pre)
  :debug(false), test( false), precipitationParam(pre)
    
{
}

Bufr::Bufr():debug(false), test( false ), precipitationParam(PrecipitationRA)
{
}

Bufr::~Bufr()
{
}

void 
Bufr::replaceCCCXXX(std::string &bufr, int ccx)
{
  	char tmp[10];
  	std::string::size_type i=bufr.find(" CCCXXX");

  	if(i==std::string::npos)
    	return;

  	tmp[0]='\0';

  	if(ccx>0)
    	sprintf(tmp, " CC%c", 'A'+(ccx-1));
  
  	bufr.replace(i, 7, tmp);
}
















