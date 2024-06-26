/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: DataList.cc,v 1.5.2.5 2007/09/27 09:02:22 paule Exp $

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

#include <milog/milog.h>
#include <LoadBufrData.h>

using namespace std;

void
loadBufrData( const DataEntryList   &dl,
              DataElementList       &sd,
              StationInfoPtr        info,
              kvdatacheck::Validate &validate )
{
   StationInfo::TLongList          types;
   StationInfo::RITLongList        itt;
   DataEntryList::CITDataEntryList it;
   DataListEntry::ITDataList       itd;
   DataListEntry::TDataList        dataList;
   stringstream log;
   bool  useCorrected;

   validate.clearLogger();
   sd.clear();

   types=info->typepriority();
   it=dl.begin();

   for(; it!=dl.end(); it++){
      DataElement bufrData;

      itt=types.rbegin();

      for(;itt!=types.rend(); itt++){
         dataList=it->getTypeId(*itt);

         if(dataList.empty())
            continue;

         itd=dataList.begin();
         bufrData.time( itd->obstime() );

         for(;itd!=dataList.end(); itd++){
            //COMMENT:
            //We use only the default sensor for every parameter, ie sensor=0.
            //This may be to restrective. Shuold this be a configuration option for
            //the parameters we wish to overide this behavior for.


            if( validate( *itd, &useCorrected ) ) {
               itd->useCorrected(useCorrected);
              //cerr << "LoadBufrData: '" << itd->obstime() << "' " << itd->paramID() << ", " << itd->typeID() << ", " << itd->original() << endl;
              bufrData.setData( itd->paramID(), itd->typeID(), 
              itd->sensor(), itd->level(), itd->original(), itd->useCorrected() );
            }else{
               log << "NOT USED. sid: " << itd->stationID()
                       << " tid: " << itd->typeID()
                       << " pid: " << itd->paramID()
                       << " sen: " << itd->sensor()
                       << " lvl: " << itd->level()
                       << " orig: " << itd->original()
                       << endl;
            }
         }
      }

      if(!bufrData.undef() ){

         try{
            sd[bufrData.time()]=bufrData;
         }
         catch(out_of_range &ex){
            LOGDEBUG("EXCEPTION(out_of_range): Should not happend!!!"<< endl);
         }
         catch(...){
            LOGDEBUG("EXCEPTION(Unknown): Should never happend!" << endl);
         }
      }
   }

   string notUsed=log.str();
   string rejected=validate.getLog();

   if( ! rejected.empty() || ! notUsed.empty() ) {
      log.str("");
      if( !rejected.empty() ) {
         log << "REJECTED";
         if( !notUsed.empty() )
            log << " and NOT USED";
      } else {
         log << "NOT USED";
      }
      log << " paramteres.";

      LOGDEBUG2( log.str() << endl << rejected << notUsed );
   }
}


