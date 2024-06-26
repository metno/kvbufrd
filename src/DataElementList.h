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
#include "boost/cstdint.hpp"
#include "boost/crc.hpp"
#include "boost/thread/tss.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
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

  	boost::posix_time::ptime time_;

  	friend class DataElementList;
  	//  friend class BufrDataList::BufrDataProxy
  	
 public:
  	KvParam TA;       
  	KvParam TAM;       
  	KvParam TAN;       
  	KvParam TAX;
  	KvParam TD;
  	KvParam UU;       
  	KvParam UM;       
  	KvParam FF;   
  	KvParam FM;   
  	KvParam FG;    //Gust since last observation
  	KvParam FG_1;
  	KvParam FG_6;
  	KvParam FG_12;
  	KvParam FG_010; //Gust 10 last minutes before observation.
  	KvParam FX;    //Max wind since last observation
  	KvParam FX_1;   
  	KvParam FX_3;
  	KvParam FX_6;
  	KvParam DD;   
  	KvParam DM;   
  	KvParam DG;
  	KvParam DG_010;
  	KvParam DG_1;
  	KvParam DG_6;
  	KvParam DX;
  	KvParam DX_3;   
  	KvParam RA;    
  	KvParam RR_1; 
  	KvParam RR_2;
  	KvParam RR_3;  
  	KvParam RR_6;
  	KvParam RR_9;
  	KvParam RR_12;
  	KvParam RR_15;
  	KvParam RR_18;
  	KvParam RR_24;
  	KvParam RT_1;     
  	KvParam PO;  //PO, stasjonstrykk.    
  	KvParam POM;  //POM, stasjonstrykk.  
  	KvParam PON;  //PON, stasjonstrykk.  
  	KvParam POX;  //POX, stasjonstrykk.  
  	KvParam PH;  //PH, trykk redusert til havets niv�, ICAO standard.   
  	KvParam PR;  //PR, trykk redusert til havets niv�.
  	KvParam PP;  
  	KvParam TAN_12;
  	KvParam TAX_12;
  	KvParam TWF;
  	KvParam TW;
  	KvParam TWM;
  	KvParam TWN;
  	KvParam TWX;
  	KvParam TGN;
  	KvParam TGN_12;
  	KvParam WAWA;
  	KvParam HLN;
  	KvParam EM;    //Snow state to the gound (Markas tilstand).
  	KvParam EE;   //BUFR parameter E or E' (0 20 062).
  	KvParam Es;  //Ice deposit (thickness) at observation time.
  	KvParam ERs; //Rate of ice accretion.
  	KvParam XIS;  //Cause of ice accreation.
  	KvParam Ci;   //Sea ice concentration.
  	KvParam Bi;   //Amount and type of ice.
  	KvParam Zi;   //Ice situation.
  	KvParam Si;   //Ice development
  	KvParam Di;   //Bearing of ice edge.
  	KvParam SA;    //Snow depth.
  	KvParam SD;    //Met.no code for snow cover.
  	KvParam SS_24; //Snow accumulation past 24 hour.
  	KvParam Vmor;  //Automatic measured horizontal visibility
  	KvParam VV;    //Human estimated horizontal visibility
  	KvParam HL;
  	KvParam NH;     //NhClCmCh
  	KvParam CL;     //NhClCmCh
  	KvParam CM;     //NhClCmCh
  	KvParam CH;     //NhClCmCh
  	KvParam IR;   //Nedb�rindikator.
  	KvParam IX;   //V�rindikator
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
  	KvParam OT_1;
  	KvParam OT_24;
		KvParam QLI; //Long-wave radiation, integrated over period specified
		KvParam QSI; //Global solar radiation (high accuracy), integrated over period specified
		KvParam QD;  //Diffuse solar radiation (high accuracy), integrated over period specified
		KvParam QS;  //Direct solar radiation (high accuracy), integrated over period specified
  	KvParam MDIR; //Direction of motion of moving observing platform/ship.
  	KvParam MSPEED; //Speed of motion of moving observing platform/ship.
  	KvParam MLAT; //Latitude of ship/platform.
  	KvParam MLON; //Longitude of ship/platform.
  	KvParam WDMT; //Mean wave direction.
  	KvParam WTZ; //Mean wave period.
  	KvParam PWA;  //Period of waves.
  	KvParam Pw;   //Period of wind waves.
		KvParam CD;   //Havstrøm, retning
		KvParam CV;   //Havstrøm, fart
		KvParam CSW;  //Elektrisk konduktivitet i sjøvann
		KvParam SSW;  //Saltholdighet i sjøvann
		KvParam WDHF; //Bølgeretning høyfrekvente bølger
		KvParam WDLF; //Bølgeretning lavfrekvente bølger 
		KvParam WDP1; //Primærbølgens hovedretning. Tilhører WHM0 og WTP
		KvParam WHM0; //Signifikant bølgehøyde
		KvParam WHM0HF; //høyfrekvent signifikant bølgehøyde
		KvParam WHM0LF; //lavfrekvent signifikant bølgehøyde
		KvParam WHMAX;  //Høyde på den høyeste individuelle bølgen
		KvParam WSPRTP; //Spredning ved spektral peak periode
		KvParam WTHHF;  //midlere høyfrekvente bølgeretning
		KvParam WTHMAX; //Perioden til den høyeste bølgen
		KvParam WTHTP; //Midlere bølgeretning ved maks i spekteret, tilhører WTP
		KvParam WTM01; //Bølgeperiode tilsvarende midlere frekvens i spekteret
	  KvParam WTM02; //bølgeperiode
	  KvParam WTM02HF; //Høyfrekvent bølgeperiode
    KvParam WTM02LF; //Lavfrekvent bølgeperiode
    KvParam WTP;    //Perioden som svarer til maksimum i spektret
  	KvParam Hwa;  //Height of waves.
  	KvParam Hw;   //Height of wind waves.
  	int nSet;
  	bool onlyTypeid1;
  	std::list<int> typeidList;

  	DataElement();
  	DataElement(const DataElement &p);
  	~DataElement();

  	DataElement& operator=(const DataElement &p);

		//If isCorrected == true, we use the corrected value from kvalobs and
		//not the original value.
  	bool setData( int  param,
  	              int  typeid_,
									int  sensor,
									int level,
				     			const std::string &data_, bool isCorrected );

  	/**
  	 * Removes data that only generates groups with slashes.
  	 */

  	void           time(const boost::posix_time::ptime &t){time_=t;}
  	boost::posix_time::ptime time()const{ return time_;}

  	bool undef()const{ return time_.is_special() || nSet == 0;}
  	boost::uint32_t crc(std::string *theDataUsed) const;

  	void writeTo( std::ostream &header, std::ostream &data, bool withId=true  )const;
		virtual void crcHelper(std::ostream &o)const;
  	int numberOfValidParams() const{ return params.numberOfValidParams(); }
  	
		//In the code there is a lot code that sets param values to FLT_MAX. 
   	//This makes a lot of trouble after we introduced sensor and levels.
   	//All functions returns now FLT_MAX or INT_MAX if there is no value for
   	//the parameter for sensor level. The clean method removes all values of 
   	//FLT_MAX.
  
	 	void clean();

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
			     const boost::posix_time::ptime &t){ it->time_=t;}
public:
  
  	class DataElementProxy{
    	//BufrDataProxy is a helper class that is used
    	//to deceide if the array operator [] is used
    	//as a lvalue or a rvalue.
    
    	DataElementList          *sdl;
    	boost::posix_time::ptime timeIndex;
    
  	public:
    	DataElementProxy(DataElementList *sdl_,
					   const boost::posix_time::ptime &t)
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
  	 * @return is_special if list is empty.
  	 */
  	boost::posix_time::ptime firstTime() const;

  	/**
  	 * If used as a lvalue the BufrData record will be inserted if it don't
  	 * exist.  The current record at timeIndex will be replaced if it exist.
  	 * if we use the operator as a rvalue it will throw std::out_of_range
  	 * if there is now BufrData record at timeIndex.
  	 *
  	 * \exception std::out_of_range, used as rvalue, if there is now BufrData
  	 *            at timeIndex.
  	 */
  	const DataElementProxy operator[](const boost::posix_time::ptime &timeIndex)const;
  	DataElementProxy operator[](const boost::posix_time::ptime &timeIndex);
  
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
 	 bool      insert(const boost::posix_time::ptime &timeIndex,
		   			  const DataElement &bd,
		   			  bool replace=false);

  	int       size()const { return dataList.size();}

  	IDataElementList find(const boost::posix_time::ptime &from);
  	CIDataElementList find(const boost::posix_time::ptime &from)const;
    
  	IDataElementList  begin(){ return dataList.begin();}
  	CIDataElementList begin()const{ return dataList.begin();}
  	IDataElementList  end(){ return dataList.end();}
  	CIDataElementList end()const { return dataList.end();}

  	DataElementList subData( const boost::posix_time::ptime &from, const boost::posix_time::ptime &to=boost::posix_time::ptime() ) const;

  	DataElementList& operator=( const DataElementList &rhs );

  	void writeTo( std::ostream &o, bool withId=true, bool debug=true )const;

		//Remove all FLT_MAX and missing values (-32767)
		void clean();

  	friend std::ostream& operator<<(std::ostream& ost,
				 					  const DataElementList& sd);
};




std::ostream& operator<<(std::ostream& ost,
						  const DataElement& sd);

std::ostream& operator<<(std::ostream& ost,
						  const DataElementList& sd);

#endif
