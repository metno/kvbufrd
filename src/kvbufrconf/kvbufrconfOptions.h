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
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <string>

struct Options {
    typedef enum { NON, PRECIP, SVV, SHIP, SYNOP, SYNOP_WIGOS, BSTATIONS, MBOUY } What;
   std::string confile;              //--conf
   std::string templatefile;         //--template
   std::string outconf;              //--out
   int nIsTypes;
   int debug;
   What  type;

   std::string getType()const;
   Options():nIsTypes(0), debug(0), type( NON ) {}
};

void
getOptions(int argn, char **argv, Options &opt);

void
use(int exitstatus);


#endif
