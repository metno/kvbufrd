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
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "ConfApp.h"
#include "ConfMaker.h"
#include "InitLogger.h"
#include "kvbufrconfOptions.h"
#include <kvalobs/kvPath.h>
#include <miconfparser/miconfparser.h>
#include <miutil/trimstr.h>

using namespace std;

int
main(int argn, char** argv)
{
  Options opt;
  std::string confFile;
  miutil::conf::ConfSection* conf;
  miutil::conf::ConfSection* templateConf = 0;
  miutil::conf::ConfParser parser(false);

  // If we have different prefix than the kvalobs libs.
  auto prefix = kvalobs::kvPath(kvalobs::prefix);
  std::string myprefix(PREFIX);
  miutil::trimstr(prefix, miutil::TRIMBACK, "/ \t\r\n");
  miutil::trimstr(myprefix, miutil::TRIMBACK, "/ \t\r\n");
  if ( myprefix != prefix) {
    cerr << "Setting prefix '" << myprefix << "'\n";
    setKvPathPrefix(PREFIX);
  }

  InitLogger(argn, argv, "kvbufrconf");
  getOptions(argn, argv, opt);

  if ( myprefix != prefix ) {
    cerr << "Resetting prefix '" << prefix << "'\n";
    setKvPathPrefix(prefix);
  }

  

  if (!opt.verbose) {
    milog::Logger::logger().logLevel(milog::ERROR);
  }

  if (opt.debug > 0)
    parser.debugLevel(opt.debug);

  try {
    conf = miutil::conf::ConfParser::parse(opt.confile);
  } catch (const logic_error& ex) {
    LOGFATAL(ex.what());
    return 1;
  }

  if (!opt.templatefile.empty()) {
    try {
      ifstream fin(opt.templatefile.c_str());

      if (!fin.is_open()) {
        LOGFATAL("Cant open file '" << opt.confile << "'\n\n");
        return 1;
      }
      templateConf = parser.parse(fin);

      // templateConf = miutil::conf::ConfParser::parse( opt.templatefile );
    } catch (const logic_error& ex) {
      LOGFATAL("Cant parse templatefile '" << opt.templatefile
                                           << "'. Reason: " << ex.what());
      return 1;
    }
  }

  ConfApp app(opt, conf);
  ConfMaker confMaker(app);
  bool ret;

  switch (opt.type) {
    case Options::SVV:
      ret = confMaker.doSVVConf(opt.outconf, templateConf);
      break;
    case Options::PRECIP:
      ret = confMaker.doPrecipConf(opt.outconf, templateConf);
      break;
    case Options::SHIP:
      ret = confMaker.doShipConf(opt.outconf, templateConf);
      break;
    case Options::SYNOP:
      ret = confMaker.doSynopConf(opt.outconf, templateConf);
      break;
    case Options::SYNOP_WIGOS:
      ret = confMaker.doWigosConf(opt.outconf, templateConf);
      break;
    case Options::BSTATIONS:
      ret = confMaker.doBStationsConf(opt.outconf, templateConf);
      break;
    case Options::MBOUY:
      cerr << "\n\n --- Moored buoys is not implemented --- \n\n";
      ret = true;
      break;
    default:
      ret = false;
      break;
  }

  if (!ret) {
    cerr << " -- Failed to generate kvbufrd configuration! --" << endl << endl;
  }

  return ret ? 0 : 1;
}
