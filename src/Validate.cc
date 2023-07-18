/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ValidData.cc,v 1.5.2.4 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <initializer_list>
#include <milog/milog.h>
#include "Validate.h"

using namespace kvalobs;
using namespace kvQCFlagTypes;
using namespace std;

//Bxrge Moe
//2005.02.28
//Endring av grenseverdi filter for temeperatur. 
//Vi slipper ikke gjennom "sv�rt mistenkelige" temperaturer.
//Dvs.observert verdi h�yere enn h�yeste testverdi eller lavere enn 
//laveste testverdi.


namespace {

//Check if a value is in the set given by validValues 
template<class T> bool valueIn(T value, initializer_list<T> validValues){
   for( const T &v : validValues ) {
      if (value == v ) {
         return true;
      }
   }
   return false;
} 

}


kvdatacheck::
Validate::
Validate()
{
   validate = &Validate::validDataNoCheck;
}

kvdatacheck::
Validate::
Validate( HowToValidate howToValidate )
{
   switch( howToValidate ) {
      case UseOnlyControlInfo:
         validate = &Validate::validDataUseOnlyControlInfo;
         break;
      case UseOnlyUseInfo:
         validate = &Validate::validDataUseOnlyUseInfo;
         break;
      case CombineControlAndUseInfo:
         validate = &Validate::validDataCombineControlAndUseInfo;
         break;
      case NoCheck:
         validate = &Validate::validDataNoCheck;
         break;
      default:
         validate = &Validate::validDataNoCheck;
         break;
   }
}

bool
kvdatacheck::
Validate::
operator()( const Data &data, bool *useCorrected )
{
   if( useCorrected != nullptr ) {
      *useCorrected = false;
   }

   if( data.paramID() == 0 ) {
      //Ignore and reject paramid==0.
      return false;
   }

   return (this->*validate)( data, useCorrected );
}


/**
 * We use the controlinfo and useinfo in data to deceide if we can use
 * this parameter. We use the parameter:
 *  1) It is not  checked
 *  2) It has passed the check
 *  3) It has not passed the check, but it is impossible to deciede if there
 *     is a problem with this parameter or another parameter it is
 *     checked againt.
 * 
 * For useinfo.
 * We dont use the data if:
 * 1) useingo(1) Have a observation period that differ to mutch from normert.
 */
bool
kvdatacheck::
Validate::
validDataUseOnlyControlInfo( const Data &data, bool *useCorrected )
{
   kvUseInfo     uinfo=data.useinfo();
   kvControlInfo info=data.controlinfo();
   
   if(!check_useinfo1(uinfo, data.paramID())){
      log <<"REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " useinfo(1)" << endl;
      return false;
   }


   if(!check_fmis(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " (fmis)" << endl;
      return false;
   }


   if(!check_fr(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                        << " typeid: " << data.typeID()
                        << " obstime: "  << data.obstime()
                        << " paramid: " << data.paramID()
                        << " original: " << data.original() << " (fr)" << endl;
      return false;
   }

   if(!check_fcc(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                        << " typeid: " << data.typeID()
                        << " obstime: " << data.obstime() << " paramid: " << data.paramID()
                        << " original: " << data.original() << " fcc" << endl;
      return false;
   }

   if(!check_fcp(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " (fcp)" << endl;
      return false;
   }

   if(!check_fs(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " (fs)" << endl;
      return false;
   }

   if(!check_fnum(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " (fnum)" << endl;
      return false;
   }

   if(!check_fpos(info, data.paramID())){
      log << "REJECTED: stationid: " << data.stationID()
                   << " typeid: " << data.typeID()
                   << " obstime: " << data.obstime()
                   << " paramid: " << data.paramID()
                   << " original: " << data.original() << " (fpos)" << endl;
      return false;
   }


   return true;
}




bool
kvdatacheck::
Validate::
validDataUseOnlyUseInfo( const Data &data, bool *useCorrected )
{
   kvUseInfo uinfo=data.useinfo();
   kvControlInfo cinfo=data.controlinfo();
   const int A = 10;
   int uf_0 = uinfo.flag( 0 );
   int uf_1 = uinfo.flag( 1 );
   int uf_2 = uinfo.flag( 2 ); // QA level for original data value.
   int uf_3 = uinfo.flag( 3 ); //Treatment of original data value.
   int fmis    = cinfo.flag( f_fmis );
   int fr      = cinfo.flag( f_fr );
   int fpre    = cinfo.flag( f_fpre );
   int fhqc    = cinfo.flag( f_fhqc );

   /*
   LOGDEBUG2("Validate: stationid: " << data.stationID() << " typeid: " << data.typeID()
             << " obstime: " << data.obstime()
             << " paramid/sensor/level: " << data.paramID() << "/" << data.sensor() << "/" << data.level()
             << " original: " << data.original()
             << " cflags: " << data.controlinfo()
             << " uflags: " << data.useinfo() << " u0: " << uinfo_0 << " u2: " << uinfo_2 << " u3: " << uinfo_3 << " cmis: "<< cinfo_mis );
    */


   if (atoi( data.original().c_str() ) == -32767 ) {
      log << "REJECTED: stationid: " << data.stationID() << " typeid: " << data.typeID()
                          << " obstime: " << data.obstime()
                          << " paramid: " << data.paramID()
                          << " original: " << data.original() << " (-32767 (missing))"
                          << " sensor/level: " << data.sensor() << "/" << data.level()
                          << endl;

      return false;
   }

   

   //Flagg kriterier for å godkjenne en observasjon fra Pål.
   // original != -32767  && 
   // useinfo(0) != 9 && useinfo(1)=0,1,9 && 
   //   ( (fmis=0 && useinfo(2)=0,1,2,9 && useinfo(3)=0,9) || 
   //       (fmis=4 && useinfo(2)=3  && useinfo(3)=1 && (fr=A || fpre=4) && fhqc != 7)  )
   //
   //cf (fmis,fr,fpre,fhqc): (0,1,0,0) uf (uf_0,uf_1,uf_2,uf_3): (7,0,0,0)
   if( uf_0 != 9 && valueIn( uf_1, {0,1,9} ) && 
      ( ( fmis == 0 && valueIn( uf_2, {0,1,2,9} ) && valueIn(uf_3, {0,9})) || 
        ( fmis == 4 && uf_2 == 3  &&  uf_3 == 1 && ( fr == A || fpre == 4) && fhqc != 7)  ) ) 
   {
      if( fmis == 4 && useCorrected != nullptr) {
         *useCorrected = true;
      }
      return true;
   }

   //As a hack until the checks for level and sensor > 0 is working
   //we accept all values for level and sensors > 0.
   if ( (data.sensor() > 0 || data.level() > 0) && atoi(data.corrected().c_str()) != -32766 && uf_0==9 ) {
      if( useCorrected != nullptr) {
         *useCorrected=false;
      } 
      return true;
   }

   log << "REJECTED: stationid: " << data.stationID() << " typeid: " << data.typeID()
       << " obstime: " << data.obstime()
       << " paramid: " << data.paramID()
       << " original: " << data.original() 
       << " sensor/level: " << data.sensor() << "/" << data.level()
       << " cf (fmis,fr,fpre,fhqc): (" << fmis<< "," << fr << "," << fpre << "," << fhqc << ")" 
       << " uf (uf_0,uf_1,uf_2,uf_3): (" << uf_0 << "," << uf_1 << "," << uf_2 << "," << uf_3 << ")" 
       << endl;
   return false;
}




#if 0
bool
kvdatacheck::
Validate::
validDataUseOnlyUseInfo( const Data &data )
{
   kvUseInfo uinfo=data.useinfo();
   kvControlInfo cinfo=data.controlinfo();

   int uinfo_0    = uinfo.flag( 0 );
   int uinfo_1    = uinfo.flag(1);
   int uinfo_2    = uinfo.flag( 2 ); // QA level for original data value.
   int uinfo_3    = uinfo.flag( 3 ); //Treatment of original data value.
   int cinfo_mis  = cinfo.flag( f_fmis );
   int cinfo_fr   = cinfo.flag( f_fr );
   int cinfo_fpre = cinfo.flag( f_fpre );
   int cinfo_fhqc = cinfo.flag( f_fhqc );

   /*
   LOGDEBUG2("Validate: stationid: " << data.stationID() << " typeid: " << data.typeID()
             << " obstime: " << data.obstime()
             << " paramid/sensor/level: " << data.paramID() << "/" << data.sensor() << "/" << data.level()
             << " original: " << data.original()
             << " cflags: " << data.controlinfo()
             << " uflags: " << data.useinfo() << " u0: " << uinfo_0 << " u2: " << uinfo_2 << " u3: " << uinfo_3 << " cmis: "<< cinfo_mis );
    */

#if 0
   if( uinfo_0 == 9 ){ //Parameter not defined in obs_pgm.
      IDLOGINFO("uinfo0","REJECTED useinfo(0)=" << uinfo_0 << " :  stationid: " << data.definedStationID()  << " typeid: " << data.typeID()
                << " obstime: " << data.obstime()
                << " paramid: " << data.paramID()
                << " original: " << data.original());
      return false;
   }
#endif

   if( atoi( data.original().c_str() ) == -32767 ) {
      log << "REJECTED: stationid: " << data.stationID() << " typeid: " << data.typeID()
                          << " obstime: " << data.obstime()
                          << " paramid: " << data.paramID()
                          << " original: " << data.original() << " (-32767 (missing))"
                          << " sensor/level: " << data.sensor() << "/" << data.level()
                          << endl;

      return false;
   }


   if( ! check_useinfo1( uinfo, data.paramID() ) ) {
      log << "REJECTED:"
            << " stationid: " << data.stationID()  << " typeid: " << data.typeID()
            << " obstime: " << data.obstime()
            << " paramid/sensor/level: " << data.paramID() << "/"  << data.sensor() << "/" << data.level()
            << " original: " << data.original()
            
            << " useinfo(1)=" << uinfo.flag( 1 )
            << endl;
      return false;
   }

   if( ! check_fmis( cinfo, data.paramID() ) ) {
      log << "REJECTED:"
            << " stationid: " << data.stationID()
            << " typeid: " << data.typeID()
            << " obstime: " << data.obstime()
            << " paramid/sensor/level: " << data.paramID() << "/" << data.sensor() << "/" << data.level()
            << " original: " << data.original()
            << " fmis=" << cinfo.flag(f_fmis)
            << endl;
      return false;
   }

   if( ! (uinfo_3 == 0 || uinfo_3 == 9)  ) {
      log << "REJECTED:"
            << " stationid: " << data.stationID()
            << " typeid: " << data.typeID()
            << " obstime: " << data.obstime()
            << " paramid/sensor/level: " << data.paramID() << "/" << data.sensor() << "/" << data.level()
            << " original: " << data.original()
            << " useinfo(3)=" << uinfo_3 << endl;

      return false;
   }
   if( ! (uinfo_2 <= 2 || uinfo_2 == 9) ) {
      log << "REJECTED:"
            << " stationid: " << data.stationID()
            << " typeid: " << data.typeID()
            << " obstime: " << data.obstime()
            << " paramid/sensor/level: " << data.paramID() << "/" << data.sensor() << "/" << data.level()
            << " original: " << data.original()
            << " useinfo(2)="<< uinfo_2
            << endl;

      return false;
   }

   return true;
}
#endif



bool
kvdatacheck::
Validate::
validDataCombineControlAndUseInfo( const Data &data, bool *useCorrected)
{
   kvUseInfo     uinfo=data.useinfo();
   kvControlInfo info=data.controlinfo();

   return true;
}


bool
kvdatacheck::
Validate::
validDataNoCheck( const Data &data, bool *useCorrected )
{
   *useCorrected = data.useCorrected();
   return true;
}


bool 
kvdatacheck::
Validate::
check_fr(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag;

   flag=flag2int(f.cflag(f_fr));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fr) << ">!" << endl;
      return true;
   }

   if(flag>5){
      return false;
   }

   //Bxrge Moe
   //2005.02.28
   //
   //Vi slipper ikke gjennom "sv�rt mistenkelige" temperaturer.
   //Dvs.observert verdi h�yere enn h�yeste testverdi eller lavere enn
   //laveste testverdi.
#if 0
   if(paramid==211 ||   //TA
         paramid==213 ||   //TAN
         paramid==215 ||   //TAX
         paramid==216 ||   //TAX_12
         paramid==214){    //TAN_12
      if(flag>3)
         return false;
   }
#endif

   return true;
}

bool 
kvdatacheck::
Validate::
check_fcc(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fcc));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fcc) << ">!" << endl;
      return true;
   }

   if(flag>7){
      return false;
   }

   return true;
}

bool 
kvdatacheck::
Validate::
check_fcp(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fcp));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fcp) << ">!" << endl;
      return true;
   }

   if(flag>7){
      return false;
   }

   return true;
}

bool 
kvdatacheck::
Validate::
check_fs(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fs));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fs) << ">!" << endl;
      return true;
   }

   if(flag>5){
      return false;
   }

   return true;
}

bool 
kvdatacheck::
Validate::
check_fnum(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fnum));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fnum) << ">!" << endl;
      return true;
   }

   if(flag>5){
      return false;
   }

   return true;
}

bool 
kvdatacheck::
Validate::
check_fpos(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fpos));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fpos) << ">!" << endl;
      return true;
   }

   if(flag>3){
      return false;
   }

   return true;
}

bool 
kvdatacheck::
Validate::
check_fmis(kvalobs::kvControlInfo &f, int paramid)
{
   int  flag=flag2int(f.cflag(f_fmis));

   if(flag<0){
      log << "Invalid flag value <" << f.cflag(f_fmis) << ">!" << endl;
      return true;
   }

   if(flag==0){
      return true;
   }

   return false;
}


int
kvdatacheck::
Validate::
flag2int(char c)
{
   switch(c){
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      case 'a':
      case 'A': return 10;
      case 'b':
      case 'B': return 11;
      case 'c':
      case 'C': return 12;
      case 'd':
      case 'D': return 13;
      case 'e':
      case 'E': return 14;
      case 'f':
      case 'F': return 15;
      default:
         return -1;
   }
}




bool 
kvdatacheck::
Validate::
check_useinfo1(kvalobs::kvUseInfo  &f,int paramid)
{
   int ui=flag2int(f.cflag(1));

   if(ui<0){
      log << "Invalid flag value (useinfo(1)) <" << f.cflag(1) << ">!" << endl;
      return true;
   }

   if(ui!=9 && ui>1)
      return false;

   return true;
}
