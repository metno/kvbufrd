/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: bufr.h,v 1.8.2.3 2007/09/27 09:02:23 paule Exp $

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

#ifndef __CONFMAKER_H__
#define __CONFMAKER_H__

#include "ConfApp.h"
#include "Indent.h"
#include "StationInfoParse.h"

class ConfMaker
{
  ConfApp& app;
  StInfoSysParamList params;
  std::list<StationInfoPtr> stationList;
  std::list<StationInfoPtr> templateStationList;
  std::map<std::string,StationInfoPtr> defaultVals;

  StationInfoPtr getDefaultVal(int bufrCode );

  StationInfoPtr newStationFrom(int searchForBufrCode,
                                StationInfoPtr st,
                                int newBufrCode,
                                const std::string& wigosid,
                                int wmono,
                                int stationid,
                                const std::string& callsign);

  // StationInfoPtr findStation( int wmono, bool &newStation );
  StationInfoPtr findStation(int wmono,
                             int stationid,
                             const std::string& callsign,
                             int bufrCode,
                             bool& newStation);

  // The first bufrCode in the bufrCodeList must be the code
  // we are generating configuration to.
  StationInfoPtr findStation(const std::string& wigosid,
                             int wmono,
                             int stationid,
                             const std::string& callsign,
                             const std::list<int>& bufrCodeList,
                             bool& newStation);
  void removeStation(StationInfoPtr station);

  bool findSensor(const StInfoSysSensorInfoList& sensors,
                  TblStInfoSysSensorInfo& sensor,
                  int paramid) const;
  bool parseTemplate(miutil::conf::ConfSection* templateConf);

  bool decodeProductCoupling(const std::string& val, StationInfoPtr station);
  bool decodeCouplingDelay(const std::string& val, StationInfoPtr station);
  bool decodePrecipPriority(const std::string& val, StationInfoPtr station);
  bool decodeWindheight(const StInfoSysSensorInfoList& sensors,
                        StationInfoPtr station);
  bool decodePrecipHeight(const StInfoSysSensorInfoList& sensors,
                          StationInfoPtr station);
  bool decodeTemperatureHeight(const StInfoSysSensorInfoList& sensors,
                               StationInfoPtr station);
  bool decodePressureHeight(const StInfoSysSensorInfoList& sensors,
                            const TblStInfoSysStation& tblStation,
                            StationInfoPtr station);
  bool decodePressureHeight(const StInfoSysSensorInfoList& sensors,
                            const TblStInfoSysWigosStation& wigosStation,
                            StationInfoPtr station);

  bool precipConfFromParams(StationInfoPtr station,
                            const std::list<int>& precipParams,
                            int stationid,
                            int typeid_);

  std::string stationIdToConfString(StationInfoPtr station) const;
  std::string typepriorityToConfString(StationInfoPtr station) const;
  std::string precipPriorityToConfString(StationInfoPtr station) const;
  void setShipOwner(StationInfoPtr station,
                    const TblStInfoSysStation& infoSysStation,
                    StInfoSysNetworkStationList::iterator itNetworkStation);
  bool setShipProductCouplingAndDelay(StationInfoPtr station);

  bool doPrintStationConf(const std::string& file);
  std::string doStationConf(StationInfoPtr station) const;

public:
  typedef enum
  {
    RA = 104,
    RR_1 = 106,
    RR_3 = 107,
    RR_6 = 108,
    RR_12 = 109,
    RR_24 = 110,
    RR_X = 117,
    RR_2 = 119,
    RR_9 = 120,
    RR_15 = 125,
    RR_18 = 126
  } PrecipParams;

  ConfMaker(ConfApp& app);

  void setParams(const StInfoSysParamList& params);
  bool doSynopConf(const std::string& outfile,
              miutil::conf::ConfSection* templateConf);
  bool doWigosConf(const std::string& outfile,
                   miutil::conf::ConfSection* templateConf);
  bool doSVVConf(const std::string& outfile,
                 miutil::conf::ConfSection* templateConf);
  bool doPrecipConf(const std::string& outfile,
                    miutil::conf::ConfSection* templateConf);
  bool doShipConf(const std::string& outfile,
                  miutil::conf::ConfSection* templateConf);
  bool doBStationsConf(const std::string& outfile,
                       miutil::conf::ConfSection* templateConf);
};

#endif
