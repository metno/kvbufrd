/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: BufrData.cc,v 1.17.2.7 2007/09/27 09:02:23 paule Exp $

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
#include <math.h>
#include <float.h>
#include <sstream>
#include <iostream>
#include <decodeutility/decodeutility.h>
#include "DataElementList.h"


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
}
using namespace decodeutility;
using namespace std;

DataElement::DataElement():
    TA( params, "TA", 211 ),
    TAM( params, "TAM", 212 ),
    TAN( params, "TAN", 213),
    TAX( params, "TAX", 215),
    TD( params, "TD", 217),
    UU( params, "UU", 262),
    UM( params, "UM", 263),
    FF( params, "FF", 81 ),
    FM( params, "FM", 85 ),
    FG_1( params, "FG_1", 90),
    FG_010(params, "FG_010", 84 ),
    FX_1(params, "FX_1", 87 ),
    FX_3(params, "FX_3", 93 ),
    DD( params, "DD", 61),
    DM(params, "DM",  64 ),
    DG(params, "DG", 63 ),
    DX(params, "DX", 67 ),
    DX_3(params, "DX_3", INT_MAX),
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
    TW(params, "TW", 242 ),
    TWM(params, "TWM", 243 ),
    TWN(params, "TWN2", 244 ),
    TWX(params, "TWX", 245 ),
    TGN(params, "TGN", 223),
    TGN_12(params, "TGN_12", 224 ),
    FG(params, "FG", 83 ),
    FX(params, "FX", 86 ),
    WAWA(params, "WAWA", 49),
    HLN(params, "HLN", INT_MAX ),
    EM(params, "EM", 7),
    SA(params, "SA", 112 ),
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
    S( params, "S", 19 ),
    AA( params, "AA", 1 ),
    ITZ( params, "ITZ", 13 ),
    ITR( params, "ITR", 12 ),
    nSet( 0 ),
    onlyTypeid1( true )

{
}

DataElement::
DataElement( const DataElement &p):
    time_(p.time_), 
    TA( params, p.TA),
    TAM( params, p.TAM), 
    TAN( params, p.TAN),
    TAX( params, p.TAX),
    TD(  params, p.TD ),
    UU( params, p.UU),
    UM( params, p.UM),
    FF( params, p.FF),
    FM( params, p.FM),
    FG_1( params, p.FG_1),
    FG_010( params, p.FG_010 ),
    FX_1( params, p.FX_1),
    FX_3( params, p.FX_3),
    DD( params, p.DD),
    DM( params, p.DM),
    DG( params, p.DG),
    DX( params, p.DX),
    DX_3( params, p.DX_3),
    RA( params, p.RA), 
    RR_1( params, p.RR_1),  
    RR_2( params, p.RR_2),
    RR_3( params, p.RR_3),
    RR_6( params, p.RR_6),
    RR_9( params, p.RR_9),
    RR_12( params, p.RR_12),
    RR_15( params, p.RR_15),
    RR_18( params, p.RR_18),
    RR_24( params, p.RR_24),
    RT_1( params, p.RT_1),
    PO( params, p.PO),   
    POM( params, p.POM),
    PON( params, p.PON),
    POX( params, p.POX),
    PH( params, p.PH),
    PR( params, p.PR),
    PP( params, p.PP),
    TAN_12( params, p.TAN_12),
    TAX_12( params, p.TAX_12),
    TW( params, p.TW),
    TWM( params, p.TWM),
    TWN( params, p.TWN),
    TWX( params, p.TWX),
    TGN( params, p.TGN),
    TGN_12( params, p.TGN_12),
    FG( params, p.FG),
    FX( params, p.FX),
    WAWA( params, p.WAWA),
    HLN( params, p.HLN),
    EM( params, p.EM),
    SA( params, p.SA),
    Vmor( params, p.Vmor),
    VV( params, p.VV),
    HL( params, p.HL),
    NH( params,  p.NH ),
    CL( params, p.CL ),
    CM( params, p.CM ),
    CH( params, p.CH ),
    IR( params, p.IR ),
    IX( params, p.IX ),
    N( params, p.N ),
    ww( params, p.ww ),
    W1( params, p.W1 ),
    W2( params, p.W2 ),
    X1WD( params, p.X1WD ),
    X2WD( params, p.X2WD ),
    X3WD( params, p.X3WD ),
    S( params, p.S ),
    AA( params, p.AA),
    ITZ( params, p.ITZ),
    ITR( params, p.ITR),
    nSet( p.nSet ),
    onlyTypeid1( p.onlyTypeid1 ),
    typeidList( p.typeidList )
{
}

DataElement&
DataElement::operator=(const DataElement &p)
{

   if(this != &p) {
      if( params.size() != p.params.size() ) {
         cerr << "FATAL BUG: Something nasty have happend.  DataElement Operator= params size differ!" << params.size() << " " << p.params.size() << endl;
         cerr << "FATAL BUG: Check that the default CTOR and the copy CTOR have the same KvParams in the same order." << endl;
         abort();
      }

      time_            = p.time_;
      KvParamList::const_iterator itSource = p.params.begin();
      KvParamList::iterator itDest = params.begin();

      for( ; itSource != p.params.end(); ++itSource, ++itDest ) {
         if( (*itSource)->id() != (*itDest)->id() ) {
            cerr << "FATAL BUG: Something nasty have happend.  DataElement Operator= params id differ!" << (*itDest)->id() << " " << (*itSource)->id() << endl;
            cerr << "FATAL BUG: Check that the default CTOR and the copy CTOR have the same KvParams in the same order." << endl;
            abort();
         }
         **itDest = **itSource;
      }

      nSet             = p.nSet;
      onlyTypeid1      = p.onlyTypeid1;
      typeidList       = p.typeidList;

   }

   return *this;
}


DataElement::~DataElement()
{
}



bool
DataElement::setData( int  param,
                      int typeid_,
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
    		**pit = fData;
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


DataElementList::DataElementList()
{
} 

DataElementList::DataElementList(const DataElementList &d)
{
  dataList=d.dataList;
}


DataElementList::~DataElementList()
{
}  
  
//BufrDataList&
//BufrDataList::operator=(const BufrDataList &rhs)
//{
//  if(this!=&rhs){
//    dataList=rhs.dataList;
//  }
//}



miutil::miTime
DataElementList::
firstTime() const
{
   CIDataElementList it=dataList.begin();

    if( it == dataList.end() )
      return miutil::miTime();

    return it->time();

}
  /**
   * \exception 
   */
const DataElementList::DataElementProxy
DataElementList::operator[](const miutil::miTime &t)const
{
  //std::cerr << "const [miTime] operator\n";
  
  return DataElementProxy(const_cast<DataElementList*>(this), t);
}

DataElementList::DataElementProxy
DataElementList::operator[](const miutil::miTime &t)
{
  //std::cerr << "[miTime] operator\n";
  
  return DataElementProxy(this, t);
}

/**
 * \exception  
 */
const DataElement&
DataElementList::operator[](const int index)const
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
DataElementList::operator[](const int index)
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
DataElementList::nContinuesTimes()const
{
  CIDataElementList it=dataList.begin();
  miutil::miTime  prevT;
  miutil::miTime  testT;
  int             n;
  
  if(it==dataList.end())
    return 0;
  
  n=1;
  prevT=it->time();
  it++;

  for(;it!=dataList.end(); it++){
    testT=it->time();
    testT.addHour(1);

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
DataElementList::insert(const miutil::miTime &timeIndex,
		      const DataElement            &sd,
		      bool                 replace)
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
DataElementList::find(const miutil::miTime &from)
{
  IDataElementList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}


CIDataElementList
DataElementList::find(const miutil::miTime &from)const
{
  CIDataElementList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}

DataElementList
DataElementList::subData( const miutil::miTime &from, const miutil::miTime &to ) const
{
   DataElementList retList;

   for( CIDataElementList it = find( from ); it != end() || ( !to.undef() && to>=it->time()); ++it )
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
DataElementList::DataElementProxy::operator=(const DataElement &rhs) //lvalue use
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

DataElementList::DataElementProxy::operator DataElement()const //rvalue use
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


std::ostream& 
operator<<(std::ostream& ost, const DataElement& sd)
{
  using namespace std;
  if(sd.time_.undef())
    ost << "obsTime                    : " << "(UNDEFINED)" <<  endl;
  else
    ost << "obsTime                    : " << sd.time_ <<  endl;
  
  ost << "tempNaa                (TA): " << sd.TA           << endl 
      << "tempMid               (TAM): " << sd.TAM           << endl
      << "tempMin               (TAN): " << sd.TAN           << endl
      << "tempMax               (TAX): " << sd.TAX           << endl
      << "tempMin       (12t)(TAN_12): " << sd.TAN_12            << endl
      << "tempMax       (12t)(TAX_12): " << sd.TAX_12            << endl
      << "TD (devpoint temperature)  : " << sd.TD << endl
      << "fuktNaa                (UU): " << sd.UU           << endl
      << "fuktMid                (UM): " << sd.UM           << endl
      << "vindHastNaa            (FF): " << sd.FF       << endl
      << "vindHastMid            (FM): " << sd.FM       << endl
      << "vindHastGust         (FG_1): " << sd.FG_1      << endl
      << "FG_010                     : " << sd.FG_010 << endl
      << "FG (Since last obs.)       : " << sd.FG                << endl
      << "vindHastMax          (FX_1): " << sd.FX_1       << endl
      << "FX_3                       : " << sd.FX_3              << endl
      << "FX  (Siden forige obs.)    : " << sd.FX                << endl
      << "vindRetnNaa            (DD): " << sd.DD       << endl
      << "vindRetnMid            (DM): " << sd.DM       << endl
      << "vindRetnGust           (DG): " << sd.DG      << endl
      << "vindRetnMax            (DX): " << sd.DX       << endl
      << "DX_3                       : " << sd.DX_3              << endl
      << "nedboerTot             (RA): " << sd.RA        << endl
      << "nedboer1Time           (RR): " << sd.RR_1      << endl
      << "nedboer2Time         (RR_2): " << sd.RR_2      << endl
      << "nedboer3Time         (RR_3): " << sd.RR_3      << endl
      << "nedboer6Time         (RR_6): " << sd.RR_6      << endl
      << "bedboer9Time         (RR_9): " << sd.RR_9      << endl
      << "nedboer12Time       (RR_12): " << sd.RR_12     << endl
      << "nedboer15Time       (RR_15): " << sd.RR_15     << endl
      << "nedboer18Time       (RR_18): " << sd.RR_18     << endl
      << "nedboer24Time       (RR_24): " << sd.RR_24     << endl
      << "Nedbør periode         (Ir): " << sd.IR                << endl
      << "Verindikator           (Ix): " << sd.IX                << endl
      << "sjøtemperatur          (TW): " << sd.TW                << endl
      << "TWN                   (TWN): " << sd.TWN               << endl
      << "TWM                   (TWM): " << sd.TWM               << endl 
      << "TWX                   (TWX): " << sd.TWX               << endl
      << "nedboerJa (min)      (RT_1): " << sd.RT_1         << endl
      << "trykkQFENaa            (PO): " << sd.PO       << endl
      << "trykkQFEMid           (POM): " << sd.POM       << endl
      << "trykkQFEMin           (PON): " << sd.PON       << endl
      << "trykkQFEMax           (POX): " << sd.POX       << endl
      << "trykkQNHNaa            (PH): " << sd.PH       << endl
      << "trykkQFFNaa            (PR): " << sd.PR       << endl
      << "trykkTendens           (PP): " << sd.PP      << endl
      << "trykkKarakter          (AA): " << sd.AA                << endl
      << "Vmor (automatic VV)  (_hVV): " << sd.Vmor              << endl
      << "VV (estimated VV)    (_hVV): " << sd.VV                << endl
      << "HLN                        : " << sd.HLN               << endl
      << "skydekke               (_N): " << sd.N                 << endl
      << "verGenerelt       (_wwW1W2): " << printOut( "ww", sd.ww )
      <<                                    printOut( "W1", sd.W1 )
      <<                                    printOut( "W2", sd.W2 ) << endl
      << "WAWA    (ww automatisk måt): " << sd.WAWA             << endl
      << "skyer           (_NhClCmCh): " << printOut( "Nh", sd.NH )
      <<                                    printOut( "Cl", sd.CL )
      <<                                    printOut( "Cm", sd.CM )
      <<                                    printOut( "Ch", sd.CH ) << endl
      << "verTillegg      (_RtWdWdWd): " << printOut( "Wd1", sd.X1WD )
      <<                                    printOut( "Wd2", sd.X2WD )
      <<                                    printOut( "Wd3", sd.X3WD ) << endl
//      << "skyerEkstra1    (_1NsChshs): " << (sd.cloudExtra.size()>0?sd.cloudExtra[0]:DataElement::CloudDataExtra()) << endl
//      << "skyerEkstra2    (_2NsChshs): " << (sd.cloudExtra.size()>1?sd.cloudExtra[1]:DataElement::CloudDataExtra()) << endl
//      << "skyerEkstra3    (_3NsChshs): " << (sd.cloudExtra.size()>2?sd.cloudExtra[2]:DataElement::CloudDataExtra()) << endl
//      << "skyerEkstra4    (_4NsChshs): " << (sd.cloudExtra.size()>3?sd.cloudExtra[3]:DataElement::CloudDataExtra()) << endl
      << "gressTemp             (TGN): " << sd.TGN               << endl
      << "gressTemp_12       (TGN_12): " << sd.TGN_12            << endl
      << "sjoegang               (_S): " << sd.S                 << endl
      << "Naar intraff FX       (ITZ): " << sd.ITZ               << endl;
  
  return ost;
}


std::ostream& 
operator<<(std::ostream& ost,
	   const DataElementList& sd)
{
  CIDataElementList it=sd.begin();

  for(;it!=sd.end(); it++){
    ost << *it << std::endl 
	      << "-----------------------------------" << std::endl;
  }
  
  return ost;
}
