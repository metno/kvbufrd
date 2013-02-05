/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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

#ifndef __ENCODEBUFR302053_H__
#define __ENCODEBUFR302053_H__

#include "EncodeBufrBase.h"


/**
 * Encode BUFR template 302053.
 * Visibility data (SHIP/Platforms)
 */
class EncodeBufr302053 :
public EncodeBufrBase {

protected:

   //override
   virtual void encode( );

public:
   EncodeBufr302053();

   virtual std::string logIdentifier() const;
   virtual std::list<int> encodeIds()const;
};

#endif
