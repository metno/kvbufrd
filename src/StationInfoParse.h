/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: StationInfoParse.h,v 1.5.6.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __StationInfoParse_h__
#define __StationInfoParse_h__

#include "StationInfo.h"
#include <climits>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <milog/milog.h>
#include <string>

namespace miconf = miutil::conf;

// class StationInfo;
// StationInfo::TLongList;

class ConfMaker;

extern std::string DefaultName;

class StationInfoParse
{
  friend class ConfMaker;

  struct DefaultVal
  {
    std::string copyto;
    bool copy;
    bool copyIsSet;
    std::string owner;
    std::list<std::string> precipitation;
    std::string list;
    milog::LogLevel loglevel;
    bool defined;
    StationInfo::TDelayList delay;
    std::string delayConf;
    int code;

    DefaultVal()
      : copy(false)
      , copyIsSet(false)
      , loglevel(milog::INFO)
      , defined(false)
      , code(-1)
    {
    }

    DefaultVal(const DefaultVal& dv)
      : copyto(dv.copyto)
      , copy(dv.copy)
      , copyIsSet(dv.copyIsSet)
      , owner(dv.owner)
      , precipitation(dv.precipitation)
      , list(dv.list)
      , loglevel(dv.loglevel)
      , defined(dv.defined)
      , delayConf(dv.delayConf)
      , code(dv.code)
    {
    }

    DefaultVal& operator=(const DefaultVal& dv)
    {
      if (&dv != this) {
        copyto = dv.copyto;
        copy = dv.copy;
        copyIsSet=dv.copyIsSet;
        owner = dv.owner;
        precipitation = dv.precipitation;
        list = dv.list;
        delay = dv.delay;
        delayConf = dv.delayConf;
        loglevel = dv.loglevel;
        defined = dv.defined;
        code = dv.code;
      }
      return *this;
    }

    void print(std::ostream &o ) {
      o << "  defined: " << (defined?"true":"false") << "\n";
      o << "           code: " << code << "\n";
      o << "           copy: " << (copy?"true":"false") << "\n";
      o << "      copyIsSet: " << (copyIsSet?"true":"false") << "\n";
      o << "         copyto: '" << copyto << "'\n";
      o << "          owner: " << owner << "\n";
      o << "  precipitation: \n";
      for ( auto p : precipitation) {
        o << "                " << p << "\n";
      }
      o << "           list: " << list << "\n";
      o << "       loglevel: " << loglevel << "\n";
      o << "          delay: \n";
      for ( auto d : delay ) {
        o << "                " << d << "\n";
      }
      o << "      dealyConf: " << delayConf << "\n";
    }

    bool valid() const;
  };

  typedef boost::shared_ptr<DefaultVal> DefaultValPtr;
  bool doDefault(DefaultValPtr def, miconf::ConfSection* stationConf);

  std::string doDefList(miconf::ValElementList& vl);

  int doDefCode(miconf::ValElementList& vl);

  std::string doDefOwner(miconf::ValElementList& vl);

  std::list<std::string> doDefPrecip(miconf::ValElementList& vl);

  milog::LogLevel doDefLogLevel(miconf::ValElementList& vl);

  bool doDefCopy(miconf::ValElementList& vl, bool* copyIsSet = 0);

  std::string doDefCopyto(miconf::ValElementList& vl);

  StationInfo::TDelayList doDefDelay(const miconf::ValElementList& vl,
                                     std::string& confDelay,
                                     miconf::ConfSection* stationConf);

  bool doStationid(const std::string& key,
                   miconf::ValElementList& vl,
                   StationInfo& st);

  bool doDelay(const std::string& key,
               miconf::ValElementList& vl,
               StationInfo& st,
               miconf::ConfSection* conf,
               DefaultValPtr defVal, 
               bool mayUseDefaultValues = true);
  bool doDelayConfMaker(const std::string& key,
               miconf::ValElementList& vl,
               StationInfo& st,
               miconf::ConfSection* conf);


  bool doPrecip(const std::string& key,
                miconf::ValElementList& vl,
                StationInfo& st);

  bool doTypePri(const std::string& key,
                 miconf::ValElementList& vl,
                 StationInfo& st);

  void doIntList(std::list<int>& iList, const miconf::ValElementList& val);

  void doInt(int& i,
             const miconf::ValElementList& val,
             const int defaultVal = INT_MAX);
  void doFloat(float& f, const miconf::ValElementList& val);

  bool parseDefaultSections(miconf::ConfSection* conf, bool useDefaultValues);

  StationInfo* parseSection(miconf::ConfSection* stationConf,
                            const std::string& id,
                            bool useDefaultValues);
  void parseStationDefSections(miconf::ConfSection* conf,
                               std::list<StationInfoPtr>& stationList,
                               bool useDefaultValues);

  DefaultValPtr getDefaultValue(const std::string &type);
  std::map<std::string, DefaultValPtr> defVals;
  bool ignoreMissingValues;

  // The following values is used
  // for error reporting while parsing a section.
  std::stringstream curErr;
  std::string curSectionName;

  std::stringstream errors;

public:
  StationInfoParse(bool ignoreMissingValue_ = false)
    : ignoreMissingValues(ignoreMissingValue_)
  {
    // Must be in sync with the method parse and the idType in
    // teh method parseStationDefSections.
    defVals[DefaultName] = DefaultValPtr(new DefaultVal());
    defVals["WMO"] = DefaultValPtr(new DefaultVal());
    defVals["ID"] = DefaultValPtr(new DefaultVal());
    defVals["CALLSIGN"] = DefaultValPtr(new DefaultVal());
    defVals["WSI"] = DefaultValPtr(new DefaultVal());
    //  defVal = defVals[DefaultName];
  }
  ~StationInfoParse() {}

  std::map<std::string, StationInfoPtr> getDefaultVals();
  StationInfoPtr defaultVal(const std::string& defaultSectionName);
  bool parse(miconf::ConfSection* stationConf,
             std::list<StationInfoPtr>& stationList,
             bool useDefaultValues = true);

  std::string getErrors() const { return errors.str(); }
};

#endif
