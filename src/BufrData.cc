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
#include <decodeutility/decodeutility.h>
#include "BufrData.h"


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

BufrData::BufrData():
    TA(FLT_MAX),
    TAM(FLT_MAX), 
    TAN(FLT_MAX),
    TAX(FLT_MAX),
    TD( FLT_MAX ),
    UU(FLT_MAX),
    UM(FLT_MAX),
    FF(FLT_MAX),
    FM(FLT_MAX),
    FG_1(FLT_MAX),
    FG_010(FLT_MAX),
    FX_1(FLT_MAX),
    FX_3(FLT_MAX),
    DD(FLT_MAX),
    DM(FLT_MAX),
    DG(FLT_MAX),
    DX(FLT_MAX),
    DX_3(FLT_MAX),
    RA(FLT_MAX), 
    RR_1(FLT_MAX),
    RR_2(FLT_MAX),
    RR_3(FLT_MAX),  
    RR_6(FLT_MAX),
    RR_9(FLT_MAX),
    RR_12(FLT_MAX),
    RR_15(FLT_MAX),
    RR_18(FLT_MAX),
    RR_24(FLT_MAX),
    RT_1(FLT_MAX),
    PO(FLT_MAX),
    POM(FLT_MAX),
    PON(FLT_MAX),
    POX(FLT_MAX),
    PH(FLT_MAX),
    PR(FLT_MAX),
    PP(FLT_MAX), 
    TAN_12(FLT_MAX),
    TAX_12(FLT_MAX),
    TW(FLT_MAX),
    TWM(FLT_MAX),
    TWN(FLT_MAX),
    TWX(FLT_MAX),
    TGN(FLT_MAX),
    TGN_12(FLT_MAX),
    FG(FLT_MAX),
    FX(FLT_MAX),
    WAWA(FLT_MAX),
    HLN(FLT_MAX),
    EM(FLT_MAX),
    SA(FLT_MAX),
    Vmor(FLT_MAX),
    VV(FLT_MAX),
    HL(FLT_MAX),
    NH( FLT_MAX ),
    CL( FLT_MAX ),
    CM( FLT_MAX ),
    CH( FLT_MAX ),
    IR( FLT_MAX ),
    IX( FLT_MAX ),
    N( FLT_MAX ),
    ww( FLT_MAX ),
    W1( FLT_MAX ),
    W2( FLT_MAX ),
    X1WD( FLT_MAX ),
    X2WD( FLT_MAX ),
    X3WD( FLT_MAX ),
    AA( FLT_MAX ),
    ITZ( FLT_MAX ),
    ITR( FLT_MAX )
{
}



BufrData::BufrData(const BufrData &p):
    time_(p.time_), 
    TA(p.TA),
    TAM(p.TAM), 
    TAN(p.TAN),
    TAX(p.TAX),
    TD( p.TD ),
    UU(p.UU),
    UM(p.UM),
    FF(p.FF),
    FM(p.FM),
    FG_1(p.FG_1),
    FG_010( p.FG_010 ),
    FX_1(p.FX_1),
    FX_3(p.FX_3),
    DD(p.DD),
    DM(p.DM),
    DG(p.DG),
    DX(p.DX),
    DX_3(p.DX_3),
    RA(p.RA), 
    RR_1(p.RR_1),  
    RR_2(p.RR_2),
    RR_3(p.RR_3),
    RR_6(p.RR_6),
    RR_9(p.RR_9),
    RR_12(p.RR_12),
    RR_15(p.RR_15),
    RR_18(p.RR_18),
    RR_24(p.RR_24),
    RT_1(p.RT_1),
    PO(p.PO),   
    POM(p.POM),
    PON(p.PON),
    POX(p.POX),
    PH(p.PH),
    PR(p.PR),
    PP(p.PP),
    TAN_12(p.TAN_12),
    TAX_12(p.TAX_12),
    TW(p.TW),
    TWM(p.TWM),
    TWN(p.TWN),
    TWX(p.TWX),
    TGN(p.TGN),
    TGN_12(p.TGN_12),
    FG(p.FG),
    FX(p.FX),
    WAWA(p.WAWA),
    HLN(p.HLN),
    EM(p.EM),
    SA(p.SA),
    Vmor(p.Vmor),
    VV(p.VV),
    HL(p.HL),
    cloudExtra( p.cloudExtra ),
    NH( p.NH ),
    CL( p.CL ),
    CM( p.CM ),
    CH( p.CH ),
    IR( p.IR ),
    IX( p.IX ),
    N( p.N ),
    ww( p.ww ),
    W1( p.W1 ),
    W2( p.W2 ),
    X1WD( p.X1WD ),
    X2WD( p.X2WD ),
    X3WD( p.X3WD ),
    S( p.S ),
    AA(p.AA),
    ITZ(p.ITZ),
    ITR(p.ITR)
  
{
}

BufrData&
BufrData::operator=(const BufrData &p)
{
    if(this==&p)
	return *this;

    time_            = p.time_;
    TA          = p.TA;
    TAM          = p.TAM;
    TAN          = p.TAN;
    TAX          = p.TAX;
    TD         = p.TD;
    UU          = p.UU;
    UM          = p.UM;
    FF      = p.FF;
    FM      = p.FM;
    FG_1     = p.FG_1;
    FG_010   = p.FG_010;
    FX_1      = p.FX_1;
    FX_3             = p.FX_3;
    DD      = p.DD;
    DM      = p.DM;
    DG     = p.DG;
    DX      = p.DX;
    DX_3             = p.DX_3;
    RA       = p.RA;
    RR_1     = p.RR_1;
    RR_2     = p.RR_2;
    RR_3     = p.RR_3;
    RR_6     = p.RR_6;
    RR_9     = p.RR_9;
    RR_12    = p.RR_12;
    RR_15    = p.RR_15;
    RR_18    = p.RR_18;
    RR_24    = p.RR_24;
    RT_1        = p.RT_1;
    PO      = p.PO;
    POM      = p.POM;
    PON      = p.PON;
    POX      = p.POX;
    PH      = p.PH;
    PR      = p.PR;
    PP     = p.PP;
    TAN_12           = p.TAN_12;
    TAX_12           = p.TAX_12;
    TW               = p.TW;
    TWM              = p.TWM;
    TWN              = p.TWN;
    TWX              = p.TWX;
    TGN              = p.TGN;
    TGN_12           = p.TGN_12;
    FG               = p.FG;
    FX               = p.FX;
    WAWA             = p.WAWA;
    HLN              = p.HLN;
    EM               = p.EM;
    SA               = p.SA;
    Vmor             = p.Vmor;
    VV               = p.VV;
    HL               = p.HL;
    cloudExtra       = p.cloudExtra;
    NH          = p.NH;
    CL          = p.CL;
    CM          = p.CM;
    CH          = p.CH;
    IR               = p.IR;
    IX               = p.IX;
    N                = p.N;
    ww               = p.ww;
    W1               = p.W1;
    W2               = p.W2;
    X1WD              = p.X1WD;
    X2WD              = p.X2WD;
    X3WD              = p.X3WD;
    S                = p.S;
    AA               = p.AA;
    ITZ              = p.ITZ;
    ITR              = p.ITR;
    return *this;
}

BufrData::~BufrData()
{
}

void 
BufrData::cleanUpSlash()
{

}


bool
BufrData::setData(const int  &param,
		   const std::string &data_)
{
    float       fData;
    int         im;
    char        ch;
    std::string s;
    char        buf[256];

    if(data_.empty())
      return true;

    if(sscanf(data_.c_str(),"%f", &fData)!=1){
      fData=FLT_MAX;
      return false;
    }

    im=static_cast<int>(round(fData));
    sprintf(buf, "%d", im);

    switch(param){
    case 211: TA=fData;     break; //TA
    case 212: TAM=fData;    break; //TAM
    case 213: TAN=fData;    break; //TAN
    case 215: TAX=fData;    break; //TAX
    case 262: UU=fData;     break; //UU
    case 263: UM=fData;     break; //UM
    case  81: FF=fData;     break; //FF
    case  85: FM=fData;     break; //FM
    case  90: FG_1=fData;   break; //FG_1
    case  84: FG_010=fData; break; //FG_010
    case  93: FX_3=fData;   break; //FX_3
    case  87: FX_1=fData;   break; //FX_1
    case  61: DD=fData;     break; //DD
    case  64: DM=fData;     break; //DM
    case  63: DG=fData;     break; //DG
    case  67: DX=fData;     break; //DX
    case 104: RA=fData;     break; //RA
    case 106: RR_1=fData;   break; //RR_1
    case 107: RR_3=fData;   break; //RR_3
    case 108: RR_6=fData;   break; //RR_6
    case 109: RR_12=fData;  break; //RR_12
    case 110: RR_24=fData;  break; //RR_24
    case 119: RR_2=fData;   break; //RR_2
    case 120: RR_9=fData;   break; //RR_9
    case 123: RT_1=fData;   break; //RT_1
    case 125: RR_15=fData;  break; //RR_15
    case 126: RR_18=fData;  break; //RR_18
    case 173: PO=fData;     break; //PO
    case 174: POM=fData;    break; //POM
    case 175: PON=fData;    break; //PON
    case 176: POX=fData;    break; //POX
    case 172: PH=fData;     break; //PH
    case 178: PR=fData;     break; //PR
    case 177: PP=fData;     break; //PP
    case 214: TAN_12=fData; break; //TAN_12
    case 216: TAX_12=fData; break; //TAX_12
    case 242: TW=fData;     break; //TW
    case 244: TWN=fData;    break; //TWN
    case 243: TWM=fData;    break; //TWM
    case 245: TWX=fData;    break; //TWX
    case 223: TGN=fData;    break; //TGN
    case 224: TGN_12=fData; break; //TGN_12
    case  83: FG=fData;     break; //FG
    case  86: FX=fData;     break; //FX
    case  56: HLN=fData;    break; //HLN
    case  49: WAWA=fData;   break; //WaWa
    case   7: EM=fData;     break; //E (snow state to the ground) in E'sss
    case 112: SA=fData;     break; //sss (snow depth) in E'sss
    case   9: IR = fData;   break; //IR, _irix
    case  10: IX = fData;   break; //IX, _irix
    case  55: HL=fData;     break; //HL, _hVV
    case 271: Vmor = fData; break; //Vmor, _hVV
    case 273: VV = fData;   break; //VV, _hVV
    case  15: N = fData;    break; //NN, _N
    case  12: ITR = fData;  break; //ITR, _RRRtr
    case  41: ww = fData;   break; //WW, _wwW1W2
    case  42: W1 = fData;   break; //W1, _wwW1W2
    case  43: W2 = fData;   break; //W2, _wwW1W2
    case  14: NH = fData;   break; //NH, _NhClCmCh")D
    case  23: CL = fData;   break; //CL, _NhClCmCh")
    case  24: CM = fData;   break; //CM, _NhClCmCh")
    case  22: CH=fData;     break; //CH, _NhClCmCh")
    case  44: X1WD = fData; break; //X1WD"_RtWdWdWd")
    case  45: X2WD = fData; break; //X2WD"_RtWdWdWd")
    case  46: X3WD = fData; break; //X3WD"_RtWdWdWd")
    case 25:                       //NS1, "_1NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 1, fData ) )
             break;

          cloudExtra[0].Ns = fData;
          break;
    case 26:                       //NS2, "_2NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 2, fData ) )
             break;

          cloudExtra[1].Ns = fData;
          break;
    case 27:                       //NS3, "_3NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 3, fData ) )
             break;

          cloudExtra[2].Ns = fData;
          break;
    case 28:                       //NS4, "_4NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 4, fData ) )
             break;

          cloudExtra[3].Ns = fData;
          break;
    case 305:                      //CC1, "_1NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 1, fData ) )
             break;

          cloudExtra[0].C = fData;
          break;
    case 306:                      //CC2, "_2NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 2, fData ) )
             break;

          cloudExtra[1].C = fData;
          break;
    case 307:                      //CC3, "_3NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 3, fData ) )
             break;

          cloudExtra[2].C = fData;
          break;
    case 308:                      //CC4, "_4NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 4, fData ) )
             break;

          cloudExtra[3].C = fData;
          break;
    case 301:                      //HS1, "_1NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 1, fData ) )
             break;

          cloudExtra[0].hshs = fData;
          break;
    case 302:                      //HS2, "_2NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 2, fData ) )
             break;

          cloudExtra[1].hshs = fData;
          break;
    case 303:                      //HS3, "_3NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 3, fData ) )
             break;

          cloudExtra[2].hshs = fData;
          break;
    case 304:                      //HS4, "_4NsChshs")
          if( ! resizeAndCheckData( cloudExtra, 4, fData ) )
             break;

          cloudExtra[3].hshs = fData;
          break;
    case  19: S = fData;  break;   //_S
    case  13: ITZ=fData;  break;   //ITZ, "_tz")
    case   1: AA = fData; break;   //AA, _aa
    default:
      return false;
    }
    
    return true;
}


BufrDataList::BufrDataList()
{
} 

BufrDataList::BufrDataList(const BufrDataList &d)
{
  dataList=d.dataList;
}


BufrDataList::~BufrDataList()
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
BufrDataList::
firstTime() const
{
   CIBufrDataList it=dataList.begin();

    if( it == dataList.end() )
      return miutil::miTime();

    return it->time();

}
  /**
   * \exception 
   */
const BufrDataList::BufrDataProxy
BufrDataList::operator[](const miutil::miTime &t)const
{
  //std::cerr << "const [miTime] operator\n";
  
  return BufrDataProxy(const_cast<BufrDataList*>(this), t);
}

BufrDataList::BufrDataProxy
BufrDataList::operator[](const miutil::miTime &t)
{
  //std::cerr << "[miTime] operator\n";
  
  return BufrDataProxy(this, t);
}

/**
 * \exception  
 */
const BufrData&
BufrDataList::operator[](const int index)const
{
  CIBufrDataList it=dataList.begin();

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

BufrData&
BufrDataList::operator[](const int index)
{
   IBufrDataList it=dataList.begin();

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
BufrDataList::nContinuesTimes()const
{
  CIBufrDataList it=dataList.begin();
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
BufrDataList::insert(const miutil::miTime &timeIndex,
		      const BufrData            &sd,
		      bool                 replace)
{
  IBufrDataList it=dataList.begin();

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




IBufrDataList
BufrDataList::find(const miutil::miTime &from)
{
  IBufrDataList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}


CIBufrDataList
BufrDataList::find(const miutil::miTime &from)const
{
  CIBufrDataList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}

BufrDataList
BufrDataList::subData( const miutil::miTime &from, const miutil::miTime &to ) const
{
   BufrDataList retList;

   for( CIBufrDataList it = find( from ); it != end() || ( !to.undef() && to>=it->time()); ++it )
      retList.dataList.push_back( *it );

   return retList;
}

BufrDataList::BufrDataProxy&
BufrDataList::BufrDataProxy::operator=(const BufrData &rhs) //lvalue use
{
  //std::cout << "BufrData: lvalue ...\n";
  IBufrDataList it=sdl->dataList.begin();

  for(;it!=sdl->dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }

  if(it==sdl->dataList.end()){
    sdl->dataList.push_back(rhs);
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

BufrDataList::BufrDataProxy::operator BufrData()const //rvalue use
{
  //std::cerr << "BufrData: rvalue ...\n";
  IBufrDataList it=sdl->dataList.begin();

  for(;it!=sdl->dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }
  
  if(it->time()!=timeIndex){
    std::ostringstream ost;
    ost << "NO BufrData at <" << timeIndex << ">!";
    throw std::out_of_range(ost.str());
  }
  
  return *it;
}

std::ostream
&operator<<(std::ostream &o, const BufrData::CloudDataExtra &cd )
{
   o << "vsci: ";

   if( cd.vsci == FLT_MAX )
      o << "NA";
   else
      o << cd.vsci;

   o << " Ns: ";
   if( cd.Ns == FLT_MAX )
      o << "NA";
   else
      o << cd.Ns;


   o << " C: ";
   if( cd.C == FLT_MAX )
      o << "NA";
   else
      o << cd.C;

   o << " hshs: ";
   if( cd.hshs == FLT_MAX )
      o << "NA";
   else
      o << cd.hshs;

   return o;
}

std::ostream&
operator<<(std::ostream &o, const BufrData::Wind &wind )
{
   o << "ff: ";
   if( wind.ff == FLT_MAX )
      o << "NA";
   else
      o << wind.ff;

   o << " dd: ";

   if( wind.dd == FLT_MAX )
      o << "NA";
   else
      o << wind.dd;

   o << " i: ";
   if( wind.i == FLT_MAX )
      o << "NA";
   else
      o << wind.i;

   return o;
}

std::ostream& 
operator<<(std::ostream& ost, const BufrData& sd)
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
      << "maxWind            tzFxFxFx: " << sd.FxMax             << endl
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
      << "skyerEkstra1    (_1NsChshs): " << (sd.cloudExtra.size()>0?sd.cloudExtra[0]:BufrData::CloudDataExtra()) << endl
      << "skyerEkstra2    (_2NsChshs): " << (sd.cloudExtra.size()>1?sd.cloudExtra[1]:BufrData::CloudDataExtra()) << endl
      << "skyerEkstra3    (_3NsChshs): " << (sd.cloudExtra.size()>2?sd.cloudExtra[2]:BufrData::CloudDataExtra()) << endl
      << "skyerEkstra4    (_4NsChshs): " << (sd.cloudExtra.size()>3?sd.cloudExtra[3]:BufrData::CloudDataExtra()) << endl
      << "gressTemp             (TGN): " << sd.TGN               << endl
      << "gressTemp_12       (TGN_12): " << sd.TGN_12            << endl
      << "sjoegang               (_S): " << sd.S                 << endl
      << "Naar intraff FX       (ITZ): " << sd.ITZ               << endl;
  
  return ost;
}


std::ostream& 
operator<<(std::ostream& ost,
	   const BufrDataList& sd)
{
  CIBufrDataList it=sd.begin();

  for(;it!=sd.end(); it++){
    ost << *it << std::endl 
	      << "-----------------------------------" << std::endl;
  }
  
  return ost;
}
