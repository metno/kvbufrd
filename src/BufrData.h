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
#ifndef __BufrData_h__
#define __BufrData_h__

#include <stdexcept>
#include <list>
#include <string>
#include <puTools/miTime.h>


class  BufrData
{          
  	miutil::miTime time_;

  	friend class BufrDataList;
  	//  friend class BufrDataList::BufrDataProxy
 public:
  	float  tempNaa;       
  	float  tempMid;       
  	float  tempMin;       
  	float  tempMax;       
  	float  fuktNaa;       
  	float  fuktMid;       
  	float  vindHastNaa;   
  	float  vindHastMid;   
  	float  vindHastGust;
  	float  vindHastMax;   
  	float  FX_3;
  	float  vindRetnNaa;   
  	float  vindRetnMid;   
  	float  vindRetnGust;  
  	float  vindRetnMax;
  	float  DX_3;   
  	float  nedboerTot;    
  	float  nedboer1Time; 
  	float  nedboer2Time;
  	float  nedboer3Time;  
  	float  nedboer6Time;
  	float  nedboer9Time;
  	float  nedboer12Time;
  	float  nedboer15Time;
  	float  nedboer18Time;
  	float  nedboer24Time; 
  	float  nedboerJa;     
  	float  trykkQFENaa;  //PO, stasjonstrykk.    
  	float  trykkQFEMid;  //POM, stasjonstrykk.  
  	float  trykkQFEMin;  //PON, stasjonstrykk.  
  	float  trykkQFEMax;  //POX, stasjonstrykk.  
  	float  trykkQNHNaa;  //PH, trykk redusert til havets niv�, ICAO standard.   
  	float  trykkQFFNaa;  //PR, trykk redusert til havets niv�.
  	float  trykkTendens;  
  	float  TAN_12;
  	float  TAX_12;
  	float  TW;
  	float  TWM;
  	float  TWN;
  	float  TWX;
  	float  TGN;
  	float  TGN_12;
  	float  FG;    //Gust since last observation
  	float  FX;    //Max wind since last observation
  	float  WAWA;
  	float  HLN;
  	float  EM;    //Snow state to the gound (Markas tilstand).
  	float  SA;    //Snow depth.
  	float  Vmor;  //Automatic measured horizontal visibility
  	float  VV;    //Human estimated horizontal visibility
  	float  HL;
  	std::string nedboerInd_verInd;
  	std::string hoeyde_sikt;
  	std::string skydekke;
  	//  std::string nedboermengde;
  	std::string verGenerelt;
  	std::string skyer;
  	std::string verTillegg;
  	std::string skyerEkstra1;
  	std::string skyerEkstra2;
  	std::string skyerEkstra3;
  	std::string skyerEkstra4;
  	std::string sjoeTemp;
  	std::string sjoegang;
  	std::string snoeMark;
  	std::string AA;
  	std::string ITZ;
  	std::string IIR; //nedb�r indikator
  	std::string ITR;

  	BufrData();
  	BufrData(const BufrData &p);
  	BufrData& operator=(const BufrData &p);
  	~BufrData();

  	bool setData(const int  &param, 
				 const std::string &data_);

  	/**
  	 * Removes data that only generates groups with slashes.
  	 */
  	void cleanUpSlash();

  	void           time(const miutil::miTime &t){time_=t;}
  	miutil::miTime time()const{ return time_;}

  	bool undef()const{ return time_.undef();}

  	friend std::ostream& operator<<(std::ostream& ost,
									  const BufrData& sd);
};

typedef std::list<BufrData>                   TBufrDataList;
typedef std::list<BufrData>::iterator        IBufrDataList;
typedef std::list<BufrData>::const_iterator  CIBufrDataList;

class BufrDataList{
  	TBufrDataList  dataList;

  	friend class BufrDataProxy;

  	//setTime is a hack to set the time_ field in BufrData. It is
  	//needed because I cant manage to make BufrDataProxy
  	//a friend of BufrData. I am not sure if this is a defect of
  	//g++ 3.3.x or if the construct 'friend class BufrDataList::BufrDataProxy'
  	//is ilegal c++. I cant see any reason why this shouldnt be allowed.

  	void setTime(std::list<BufrData>::iterator it,
			     const miutil::miTime &t){ it->time_=t;}
public:
  
  	class BufrDataProxy{
    	//BufrDataProxy is a helper class that is used
    	//to deceide if the array operator [] is used
    	//as a lvalue or a rvalue.
    
    	BufrDataList                  *sdl;
    	miutil::miTime                 timeIndex;
    
  	public:
    	BufrDataProxy(BufrDataList *sdl_,
					   const miutil::miTime &t)
      		:sdl(sdl_), timeIndex(t){}

    	BufrDataProxy& operator=(const BufrData &rhs); //used as lvalue use
    
    	operator BufrData()const; //used as rvalue
 	};

  	BufrDataList();
  	BufrDataList(const BufrDataList &d);
  	~BufrDataList();
  
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
  	const BufrDataProxy operator[](const miutil::miTime &timeIndex)const;
  	BufrDataProxy operator[](const miutil::miTime &timeIndex);
  
  	/**
  	 * \exception std::out_of_range if index is not in [0, size()>
  	 */
  	const BufrData& operator[](const int index)const;
  	BufrData&       operator[](const int index);
  

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
		   			  const BufrData &bd,
		   			  bool replace=false);

  	int       size()const { return dataList.size();}

  	IBufrDataList find(const miutil::miTime &from);
  	CIBufrDataList find(const miutil::miTime &from)const;
    
  	IBufrDataList  begin(){ return dataList.begin();}
  	CIBufrDataList begin()const{ return dataList.begin();}
  	IBufrDataList  end(){ return dataList.end();}
  	CIBufrDataList end()const { return dataList.end();}

  	BufrDataList subData( const miutil::miTime &from, const miutil::miTime &to=miutil::miTime() ) const;

  	friend std::ostream& operator<<(std::ostream& ost,
				 					  const BufrDataList& sd);
};


std::ostream& operator<<(std::ostream& ost,
						  const BufrData& sd);

std::ostream& operator<<(std::ostream& ost,
						  const BufrDataList& sd);

#endif