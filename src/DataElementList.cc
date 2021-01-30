/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: BufrData.cc,v 1.17.2.7 2007/09/27 09:02:23 paule Exp $

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO19

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
#include <math.h>
#include <float.h>
#include <sstream>
#include <iostream>
#include <vector>
#include "decodeutility/decodeutility.h"
#include "miutil/timeconvert.h"
#include "DataElementList.h"

namespace pt=boost::posix_time;

namespace {
   template <class T>
   bool resizeAndCheckData( std::vector<T> &vec, int size, float value, float null_value = FLT_MAX )
   {
      if( value == null_value )
         return false;

      if( vec.size() < size )
         vec.resize( size );

      return true;
   }

   std::string printOut( const std::string &name, float val, float null_value = FLT_MAX  )
   {
      std::ostringstream o;

      o << name <<": ";
      if( val == null_value )
         o << "NA";
      else
         o << val;

      o << " ";
      return o.str();
   }

   void noFreeCleanUp( KvParamList * ) {
      //NOOP
   }
}
using namespace decodeutility;
using namespace std;


boost::thread_specific_ptr<KvParamList> DataElement::pParams( noFreeCleanUp );

DataElement::
DataElement():
    TA( params, "TA", 211 ),
    TAM( params, "TAM", 212 ),
    TAN( params, "TAN", 213),
    TAX( params, "TAX", 215),
    TD( params, "TD", 217),
    UU( params, "UU", 262),
    UM( params, "UM", 263),
    FF( params, "FF", 81 ),
    FM( params, "FM", 85 ),
    FG(params, "FG", 83 ),
    FG_1( params, "FG_1", 90),
    FG_6( params, "FG_6", 91),
    FG_12( params, "FG_12", 92),
    FG_010(params, "FG_010", 84 ),
    FX(params, "FX", 86 ),
    FX_1(params, "FX_1", 87 ),
    FX_3(params, "FX_3", 93 ),
    FX_6(params, "FX_6", 88 ),
    DD( params, "DD", 61),
    DM(params, "DM",  64 ),
    DG(params, "DG", 63 ),
    DG_010( params, "DG_010", 77 ),
    DG_1( params, "DG_1", 78 ),
    DG_6( params, "DG_1", 79 ),
    DX(params, "DX", 67 ),
    DX_3(params, "DX_3", 74 ),
    RA(params, "RA", 104), 
    RR_1(params, "RR_1", 106),
    RR_2(params, "RR_2", 119),
    RR_3(params, "RR_3", 107 ),  
    RR_6(params, "RR_6", 108),
    RR_9(params, "RR_9", 120 ),
    RR_12(params, "RR_12", 109),
    RR_15(params, "RR_15", 125),
    RR_18(params, "RR_18", 126),
    RR_24(params, "RR_24", 110),
    RT_1(params, "RT_1", 123 ),
    PO(params, "PO", 173),
    POM(params, "POM", 174),
    PON(params, "PON", 175),
    POX(params, "POX", 176 ),
    PH(params, "PH", 172 ),
    PR(params, "PR", 178),
    PP(params, "PP", 177), 
    TAN_12(params, "TAN_12", 214),
    TAX_12(params, "TAX_12", 216),
    TWF( params, "TWF", 250 ),
    TW(params, "TW", 242 ),
    TWM(params, "TWM", 243 ),
    TWN(params, "TWN2", 244 ),
    TWX(params, "TWX", 245 ),
    TGN(params, "TGN", 223),
    TGN_12(params, "TGN_12", 224 ),
    WAWA(params, "WAWA", 49),
    HLN(params, "HLN", 56 ),
    EM(params, "EM", 7),
    EE(params, "EE", 129),
    Es(params, "Es", 101),
    ERs(params, "ERs", 139),
    XIS(params,"XIS", 11),
    Ci(params,"Ci", 4),   //Sea ice concentration.
  	Bi(params,"Bi", 2),   //Amount and type of ice.
  	Zi(params,"Zi", 21),   //Ice situation.
  	Si(params,"Si", 20),   //Ice development
  	Di(params,"Di", 6),   //Bearing of ice edge.
    SA(params, "SA", 112 ),
    SD(params, "SD", 18 ),
    SS_24(params, "SS_24", 114 ),
    Vmor(params, "VMOR", 271 ),
    VV(params, "VV", 273),
    HL(params, "HL", 55 ),
    NH( params, "NH",14 ),
    CL( params, "CL", 23 ),
    CM( params, "CM", 24 ),
    CH( params, "CH", 22 ),
    IR( params, "IR", 9 ),
    IX( params, "IX", 10 ),
    N( params, "N", 15 ),
    ww( params, "ww", 41 ),
    W1( params, "W1", 42 ),
    W2( params, "W2", 43 ),
    X1WD( params, "X1WD", 44 ),
    X2WD( params, "X2WD", 45 ),
    X3WD( params, "X3WD", 46 ),
    SG( params, "SG", 19 ),
    AA( params, "AA", 1 ),
    ITZ( params, "ITZ", 13 ),
    ITR( params, "ITR", 12 ),
    OT_1( params, "OT_1", 121),
    OT_24( params, "OT_24", 122 ),
    QLI(params, "QLI", 210),
		QSI(params, "QSI", 2070),
		QD(params, "QD", 196),  
		QS(params, "QS", 208),
    MDIR( params, "MDIR", 403),
    MSPEED( params, "MSPEED", 404),
    MLAT( params, "MLAT", 401 ),
    MLON( params, "MLON", 402 ),
    WDMT( params, "WDMT", 68 ),
    WTZ( params, "WTZ", 160 ),
    Pwa( params, "Pwa", 154 ),
    Pw( params, "Pw", 151 ),
    CD(params, "CD",290),
		CV(params, "CV", 291),   //Havstrøm, fart
		CSW(params, "CSW", 295),  //Elektrisk konduktivitet i sjøvann
    SSW(params, "SSW", 296),  //Saltholdighet i sjøvann 
    WDHF(params,"WDHF",10633), //Bølgeretning høyfrekvente bølger
		WDLF(params,"WDLF", 10637), //Bølgeretning lavfrekvente bølger 
    WDP1(params,"WDP1",161),    //Primærbølgens hovedretning. Tilhører WHM0 og WTP
		WHM0(params,"WHM0", 136), //Signifikant bølgehøyde
		WHM0HF(params,"WHM0HF",10609), //høyfrekvent signifikant bølgehøyde
		WHM0LF(params,"WHM0LF",10610), //lavfrekvent signifikant bølgehøyde
		WHMAX(params,"WHMAX", 135),  //Høyde på den høyeste individuelle bølgen
		WSPRTP(params,"WSPRTP",10621), //Spredning ved spektral peak periode
		WTHHF(params,"WTHHF", 10623),  //midlere høyfrekvente bølgeretning
		WTHMAX(params,"WTHMAX",155), //Perioden til den høyeste bølgen
		WTHTP(params,"WTHTP",10625), //Midlere bølgeretning ved maks i spekteret, tilhører WTP
		WTM01(params,"WTM01",166), //Bølgeperiode tilsvarende midlere frekvens i spekteret
	  WTM02(params,"WTM02",10627), //bølgeperiode
	  WTM02HF(params,"WTM02HF",10632), //Høyfrekvent bølgeperiode
    WTM02LF(params,"WTM02LF",10631), //Lavfrekvent bølgeperiode
    WTP(params,"WTP", 157),    //Perioden som svarer til maksimum i spektret
    Hwa( params, "Hwa", 134 ),
    Hw( params, "Hw", 131 ),
    nSet( 0 ),
    onlyTypeid1( true )

{
}

DataElement::
DataElement( const DataElement &p):
   setParamPointer( &params ),
   time_(p.time_),
   nSet( p.nSet ),
   onlyTypeid1( p.onlyTypeid1 ),
   typeidList( p.typeidList )
{
  if( params.size() != p.params.size() ) {
    cerr << "FATAL BUG CTOR: Something nasty have happend.  DataElement copy CTOR  size differ!" << params.size() << " " << p.params.size() << endl;
    abort();
  }
   
  KvParamList::const_iterator itSource = p.params.begin();
  KvParamList::iterator itDest = params.begin();

  for( ; itSource != p.params.end(); ++itSource, ++itDest ) {
    (*itDest)->copy(**itSource, false);
  }
}

DataElement&
DataElement::
operator=(const DataElement &p)
{

   if(this != &p) {
      if( params.size() != p.params.size() ) {
         cerr << "FATAL BUG: Something nasty have happend.  DataElement Operator= params size differ!" << params.size() << " " << p.params.size() << endl;
         abort();
      }

      time_            = p.time_;
      KvParamList::const_iterator itSource = p.params.begin();
      KvParamList::iterator itDest = params.begin();

      for( ; itSource != p.params.end(); ++itSource, ++itDest ) {
         if( (*itSource)->id() != (*itDest)->id() ) {
            cerr << "FATAL BUG: Something nasty have happend.  DataElement Operator= params id differ! " << (*itDest)->id() << " " << (*itSource)->id() << endl;
            cerr << "FATAL BUG: Check that the default CTOR and the copy CTOR have the same KvParams in the same order." << endl;
            abort();
         }
         (*itDest)->copy(**itSource, false);
      }

      nSet             = p.nSet;
      onlyTypeid1      = p.onlyTypeid1;
      typeidList       = p.typeidList;

   }

   return *this;
}


DataElement::
~DataElement()
{
}



bool
DataElement::
setData( int  param,
         int typeid_,
         int  sensor,
				 int level,
         const std::string &data_)
{
  float       fData;

  if(data_.empty())
    return true;

  if(sscanf(data_.c_str(),"%f", &fData)!=1){
    fData=FLT_MAX;
    return false;
  }

    
  KvParamList::iterator pit = params.begin();

  for( ; pit != params.end(); ++pit ) {
  	if( (*pit)->id() == param ) {
   		//**pit = fData;
      (*pit)->value(fData, sensor, level); 
   		break;
   	}
  }

	if( pit == params.end() ) 
		return false;

    nSet++;

    if( typeid_ != 1 )
       onlyTypeid1 = false;
    
    std::list<int>::iterator it = typeidList.begin();

    for( ; it != typeidList.end(); ++it )
       if( *it == typeid_ )
          break;

    if( it == typeidList.end() )
       typeidList.push_back( typeid_ );

    return true;
}

void
DataElement::
writeTo( std::ostream &header, std::ostream &data, bool withId )const
{
   KvParamList::const_iterator it = params.begin();

   if( it != params.end()  ) {
      header << "obstime";
      data << pt::to_kvalobs_string(time());
      ++it;
   }

   for( ; it != params.end(); ++it ) {
     if( **it == FLT_MAX )
         continue;

      header << "," << (*it)->name();

      if( withId )
         header << ":" << (*it)->id();

      data << "," << **it;
   }
}

void 
DataElement::
crcHelper(std::ostream &o)const
{
  KvParamList::const_iterator it = params.begin();

  if( it != params.end()  ) {
    o << "obstime: " <<   pt::to_kvalobs_string(time()) << endl;
    for( ; it != params.end(); ++it ) {
      for( auto sit = (*it)->sensorsBegin(); sit != (*it)->sensorsEnd(); ++sit ) {
        for( auto lvl : sit->second.levels ) {
          if( lvl.second == FLT_MAX )
            continue;
          o << (*it)->name() << " (" << sit->first <<", " << lvl.first << "): " << lvl.second << std::endl;
        }
      }
    }
  }
}

boost::uint32_t
DataElement::
crc(std::string *theDataUsed) const
{
   boost::crc_32_type crcChecker;
   string msg;
   ostringstream data;

   crcHelper( data );
   msg = data.str();

   if( theDataUsed)
    *theDataUsed=msg;

   crcChecker.process_bytes( msg.c_str(),  msg.length() );
   return crcChecker.checksum();
}

void 
DataElement::clean() {
  for( auto it = params.begin(); it != params.end(); ++it ) {
    (*it)->clean();
  }
}



DataElementList::
DataElementList()
{
} 

DataElementList::
DataElementList(const DataElementList &d)
{
  dataList=d.dataList;
}


DataElementList::
~DataElementList()
{
}  

pt::ptime
DataElementList::
firstTime() const
{
   CIDataElementList it=dataList.begin();

    if( it == dataList.end() )
      return pt::ptime();

    return it->time();

}
/**
* \exception
*/
const DataElementList::DataElementProxy
DataElementList::
operator[](const pt::ptime &t)const
{
  //std::cerr << "const [miTime] operator\n";
  
  return DataElementProxy(const_cast<DataElementList*>(this), t);
}

DataElementList::DataElementProxy
DataElementList::
operator[](const pt::ptime &t)
{
  //std::cerr << "[miTime] operator\n";
  
  return DataElementProxy(this, t);
}

/**
 * \exception  
 */
const DataElement&
DataElementList::
operator[](const int index)const
{
  CIDataElementList it=dataList.begin();

  //std::cerr << "const [int] operator\n";

  for(int i=0; it!=dataList.end() && i<index; i++,it++);

  if(it==dataList.end()){
    std::ostringstream ost;
    ost << "Index <" << index << "> out of range [0," << dataList.size()
	<< ">!";
    throw std::out_of_range(ost.str());
  }
    
  return *it;
}

DataElement&
DataElementList::
operator[](const int index)
{
   IDataElementList it=dataList.begin();

   //std::cerr << "const [int] operator\n";

  for(int i=0; it!=dataList.end() && i<index; i++,it++);

  if(it==dataList.end()){
    std::ostringstream ost;
    ost << "Index <" << index << "> out of range [0," << dataList.size()
	<< ">!";
    throw std::out_of_range(ost.str());
  }
    
  return *it;
}


int 
DataElementList::
nContinuesTimes()const
{
  CIDataElementList it=dataList.begin();
  pt::ptime prevT;
  pt::ptime testT;
  int             n;
  
  if(it==dataList.end())
    return 0;
  
  n=1;
  prevT=it->time();
  it++;

  for(;it!=dataList.end(); it++){
    testT=it->time();
    testT += pt::hours(1);

    if(testT!=prevT){
      break;
    }else{
      prevT=it->time();
      n++;
    }
  }

  return n;
}

  
bool      
DataElementList::
insert(const pt::ptime &timeIndex,
       const DataElement &sd,
       bool replace)
{
  IDataElementList it=dataList.begin();

  for(;it!=dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }
  
  if(it==dataList.end()){
    dataList.push_back(sd);
    it=dataList.end();
    it--;
  }else if(it->time()==timeIndex){
    if(!replace)
      return false;
    *it=sd;
  }else{
    it=dataList.insert(it, sd);
  }
  
  it->time_=timeIndex;

  return true;
}




IDataElementList
DataElementList::
find(const pt::ptime &from)
{
  IDataElementList it=dataList.begin();

  if(from.is_special())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}


CIDataElementList
DataElementList::
find(const pt::ptime &from)const
{
  CIDataElementList it=dataList.begin();

  if(from.is_special())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}

DataElementList
DataElementList::
subData( const pt::ptime &from, const pt::ptime &to ) const
{
   DataElementList retList;

   for( CIDataElementList it = find( from ); it != end() || ( !to.is_special() && to>=it->time()); ++it )
      retList.dataList.push_back( *it );

   return retList;
}

DataElementList&
DataElementList::
operator=( const DataElementList &rhs )
{
   if( this != &rhs ) {
      dataList.clear();
      for( CIDataElementList it = rhs.begin(); it != rhs.end(); ++it )
         dataList.push_back( *it );
   }

   return *this;
}

DataElementList::DataElementProxy&
DataElementList::DataElementProxy::
operator=(const DataElement &rhs) //lvalue use
{
  //std::cerr << "***** DataElementList::DataElementProxy:: lvalue ... rhs: '" << rhs.time() <<"' timeindex: '" << timeIndex << "'." <<  endl ;
  IDataElementList it=sdl->dataList.begin();

  for( ;it!=sdl->dataList.end(); it++ ){
    if( it->time() <= timeIndex )
      break;
  }

  if( it == sdl->dataList.end() ){
    sdl->dataList.push_back( rhs );
    it=sdl->dataList.end();
    it--;
  }else if(it->time()==timeIndex)
    *it=rhs;
  else{
    it=sdl->dataList.insert(it, rhs);
  }

  sdl->setTime(it, timeIndex);
  
  return *this;
}

DataElementList::DataElementProxy::
operator DataElement()const //rvalue use
{

  IDataElementList it=sdl->dataList.begin();

  for(;it!=sdl->dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }

  if(it==sdl->dataList.end() || it->time()!=timeIndex){
    std::ostringstream ost;
    ost << "NO BufrData at <" << timeIndex << ">!";
    throw std::out_of_range(ost.str());
  }
  
  return *it;
}

void
DataElementList::
writeTo( std::ostream &o, bool withId, bool debug )const
{
  if( ! debug ) {
    return;
  }

  ostringstream header, data;
  TDataElementList::const_reverse_iterator it=dataList.rbegin();

  if( it != dataList.rend() ) {
    it->writeTo( header, data, withId );
    o << header.str() << endl;
    o << data.str() << endl;
    ++it;
  }

   for( ; it != dataList.rend(); ++it ) {
      header.str("");
      data.str("");
      it->writeTo( header, data, false );
      o << data.str() << endl;
   }
}

void
DataElementList::clean() {
  for( auto it=dataList.begin(); it != dataList.end(); ++it) {
    it->clean();
  }
}



std::ostream& 
operator<<(std::ostream& o, const DataElement& sd)
{
  using namespace std;
  KvParamList::const_iterator it = sd.params.begin();

  if( it != sd.params.end()  ) {
    o << "obstime: " <<   (sd.time().is_special()?"(undefined)":pt::to_kvalobs_string(sd.time())) << endl;
    for( ; it != sd.params.end(); ++it ) {
      for( auto sit = (*it)->sensorsBegin(); sit != (*it)->sensorsEnd(); ++sit ) {
        for( auto lvl : sit->second.levels ) {
          if( lvl.second == FLT_MAX )
            continue;
          o << (*it)->name()<<":" << (*it)->id() << " (" << sit->first <<", " << lvl.first << "): " << lvl.second << std::endl;
        }
      }
    }
  }

  return o;
}



std::ostream& 
operator<<(std::ostream& ost,
	   const DataElementList& sd)
{
  CIDataElementList it=sd.begin();

  for(auto &it : sd ){
    ost << it << std::endl 
	      << "-----------------------------------" << std::endl;
  }
  
  return ost;
}
