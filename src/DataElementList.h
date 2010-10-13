/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: BufrData.h,v 1.8.6.6 2007/09/27 09:02:23 paule Exp $

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
#ifndef __DataElementList_h__
#define __DataElementList_h__

#include <float.h>
#include <stdexcept>
#include <list>
#include <string>
#include <sstream>
#include <boost/cstdint.hpp>
#include <boost/crc.hpp>
#include <boost/thread/tss.hpp>
#include <puTools/miTime.h>
#include "KvParam.h"



class  DataElement
{
   friend class KvParam;

   class ParamListPtrHelper {
      KvParamList *paramList;
   public:
      ParamListPtrHelper():paramList( 0 ) {}
      ParamListPtrHelper( KvParamList *paramList_ ): paramList( paramList_ ){
         DataElement::pParams.reset( paramList );
      }

      ~ParamListPtrHelper() {
         if( paramList )
            DataElement::pParams.reset( 0 );
      }
   };

protected:
   KvParamList params;

private:
   static boost::thread_specific_ptr<KvParamList> pParams;
   ParamListPtrHelper setParamPointer;

  	miutil::miTime time_;

  	friend class DataElementList;
  	//  friend class BufrDataList::BufrDataProxy
  	
 public:
  	KvParam  TA;       
  	KvParam  TAM;       
  	KvParam  TAN;       
  	KvParam  TAX;
  	KvParam  TD;
  	KvParam  UU;       
  	KvParam  UM;       
  	KvParam  FF;   
  	KvParam  FM;   
  	KvParam  FG;    //Gust since last observation
  	KvParam  FG_1;
  	KvParam  FG_6;
  	KvParam  FG_12;
  	KvParam  FG_010; //Gust 10 last minutes before observation.
  	KvParam  FX;    //Max wind since last observation
  	KvParam  FX_1;   
  	KvParam  FX_3;
  	KvParam  FX_6;
  	KvParam  DD;   
  	KvParam  DM;   
  	KvParam  DG;
  	KvParam  DG_010;
  	KvParam  DG_1;
  	KvParam  DG_6;
  	KvParam  DX;
  	KvParam  DX_3;   
  	KvParam  RA;    
  	KvParam  RR_1; 
  	KvParam  RR_2;
  	KvParam  RR_3;  
  	KvParam  RR_6;
  	KvParam RR_9;
  	KvParam RR_12;
  	KvParam  RR_15;
  	KvParam RR_18;
  	KvParam  RR_24;
  	KvParam  RT_1;     
  	KvParam  PO;  //PO, stasjonstrykk.    
  	KvParam  POM;  //POM, stasjonstrykk.  
  	KvParam PON;  //PON, stasjonstrykk.  
  	KvParam  POX;  //POX, stasjonstrykk.  
  	KvParam  PH;  //PH, trykk redusert til havets nivï¿½, ICAO standard.   
  	KvParam  PR;  //PR, trykk redusert til havets nivï¿½.
  	KvParam  PP;  
  	KvParam  TAN_12;
  	KvParam TAX_12;
  	KvParam TWF;
  	KvParam TW;
  	KvParam  TWM;
  	KvParam  TWN;
  	KvParam TWX;
  	KvParam  TGN;
  	KvParam TGN_12;
  	KvParam WAWA;
  	KvParam  HLN;
  	KvParam  EM;    //Snow state to the gound (Markas tilstand).
  	KvParam SA;    //Snow depth.
  	KvParam Vmor;  //Automatic measured horizontal visibility
  	KvParam VV;    //Human estimated horizontal visibility
  	KvParam HL;
  	KvParam NH;     //NhClCmCh
  	KvParam CL;     //NhClCmCh
  	KvParam CM;     //NhClCmCh
  	KvParam CH;     //NhClCmCh
  	KvParam IR;   //Nedbørindikator.
  	KvParam IX;   //Værindikator
  	KvParam N;    //Skydekke
  	KvParam ww;    //ww,  wwW1W2
  	KvParam W1;    //W1,  wwW1W2
  	KvParam W2;    //W2,  wwW1W2
  	KvParam X1WD;
  	KvParam X2WD;
  	KvParam X3WD;
  	KvParam SG;
  	KvParam AA;
  	KvParam ITZ;
  	KvParam ITR;
  	int nSet;
  	bool onlyTypeid1;
  	std::list<int> typeidList;

  	DataElement();
  	DataElement(const DataElement &p);
  	~DataElement();

  	DataElement& operator=(const DataElement &p);

  	bool setData( int  param,
  	              int  typeid_,
				     const std::string &data_ );

  	/**
  	 * Removes data that only generates groups with slashes.
  	 */

  	void           time(const miutil::miTime &t){time_=t;}
  	miutil::miTime time()const{ return time_;}

  	bool undef()const{ return time_.undef() || nSet == 0;}
  	boost::uint16_t crc() const;

  	void writeTo( std::ostream &header, std::ostream &data, bool withId=true  )const;

  	friend std::ostream& operator<<(std::ostream& ost,
									  const DataElement& sd);
};


typedef std::list<DataElement>                   TDataElementList;
typedef std::list<DataElement>::iterator        IDataElementList;
typedef std::list<DataElement>::const_iterator  CIDataElementList;

class DataElementList{
  	TDataElementList  dataList;

  	friend class DataElementProxy;

  	//setTime is a hack to set the time_ field in BufrData. It is
  	//needed because I cant manage to make BufrDataProxy
  	//a friend of BufrData. I am not sure if this is a defect of
  	//g++ 3.3.x or if the construct 'friend class BufrDataList::BufrDataProxy'
  	//is ilegal c++. I cant see any reason why this shouldnt be allowed.

  	void setTime(std::list<DataElement>::iterator it,
			     const miutil::miTime &t){ it->time_=t;}
public:
  
  	class DataElementProxy{
    	//BufrDataProxy is a helper class that is used
    	//to deceide if the array operator [] is used
    	//as a lvalue or a rvalue.
    
    	DataElementList                  *sdl;
    	miutil::miTime                 timeIndex;
    
  	public:
    	DataElementProxy(DataElementList *sdl_,
					   const miutil::miTime &t)
      		:sdl(sdl_), timeIndex(t){}

    	DataElementProxy& operator=(const DataElement &rhs); //used as lvalue use
    
    	operator DataElement()const; //used as rvalue
 	};

  	DataElementList();
  	DataElementList(const DataElementList &d);
  	~DataElementList();
  
  	//BufrDataList& operator=(const BufrDataList &rhs);

  	void clear(){ dataList.clear();}


  	/**
  	 * @return undef if list is empty.
  	 */
  	miutil::miTime firstTime() const;

  	/**
  	 * If used as a lvalue the BufrData record wil be inserted if it don't
  	 * exist.  The current record at timeIndex will be replaced if it exist.
  	 * if we use the operator as a rvalue it will throw std::out_of_range
  	 * if there is now BufrData record at timeIndex.
  	 *
  	 * \exception std::out_of_range, used as rvalue, if there is now BufrData
  	 *            at timeIndex.
  	 */
  	const DataElementProxy operator[](const miutil::miTime &timeIndex)const;
  	DataElementProxy operator[](const miutil::miTime &timeIndex);
  
  	/**
  	 * \exception std::out_of_range if index is not in [0, size()>
  	 */
  	const DataElement& operator[](const int index)const;
  	DataElement&       operator[](const int index);
  

  	/**
  	 * Returns the number of times, from the start of the list, that
  	 * has one hour diffs between them.
  	 */
  	int nContinuesTimes()const;

  	/**
  	 * Insert BufrData at timeIndex in the list.
  	 * 
  	 * \param timeIndex, the time the BufrData (sd) is for.
  	 * \param bd the BufrData to insert at timeIndex.
  	 * \param replace, shall bd replace the BufrData at timeIndex if
  	 *        it exist.
  	 *
  	 * \return true on success, false otherwise. It may only return false if
  	 *         replace is false and there allready is a BufrData record at
  	 *         timeIndex.
  	 */
 	 bool      insert(const miutil::miTime &timeIndex,
		   			  const DataElement &bd,
		   			  bool replace=false);

  	int       size()const { return dataList.size();}

  	IDataElementList find(const miutil::miTime &from);
  	CIDataElementList find(const miutil::miTime &from)const;
    
  	IDataElementList  begin(){ return dataList.begin();}
  	CIDataElementList begin()const{ return dataList.begin();}
  	IDataElementList  end(){ return dataList.end();}
  	CIDataElementList end()const { return dataList.end();}

  	DataElementList subData( const miutil::miTime &from, const miutil::miTime &to=miutil::miTime() ) const;

  	DataElementList& operator=( const DataElementList &rhs );

  	void writeTo( std::ostream &o, bool withId=true )const;

  	friend std::ostream& operator<<(std::ostream& ost,
				 					  const DataElementList& sd);
};




std::ostream& operator<<(std::ostream& ost,
						  const DataElement& sd);

std::ostream& operator<<(std::ostream& ost,
						  const DataElementList& sd);

#endif
