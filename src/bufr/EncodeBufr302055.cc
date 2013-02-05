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

#include <boost/assign.hpp>
#include "EncodeBufr302055.h"

EncodeBufr302055::
EncodeBufr302055()
{

}
std::string
EncodeBufr302055::
logIdentifier() const
{
   return "302055";
}

std::list<int>
EncodeBufr302055::
encodeIds()const
{
   std::list<int> ids;
   boost::assign::push_back( ids )(302055);

   return ids;
}



void
EncodeBufr302055::
encode( )
{
   bufr->addValue( 20031, data->Es, "Es, ice deposit (thickness)." );
   bufr->addValue( 20032, data->ERs.valAsInt(), "ERs, rate of ice accretion.");
   bufr->addValue( 20033, data->XIS.valAsInt(), "XIs,  cause of ice accretion.");
   bufr->addValue( 20034, data->Ci.valAsInt(), "Ci, sea ice concentration." );
   bufr->addValue( 20035, data->Bi.valAsInt(), "Bi, amount and type of ice." );
   bufr->addValue( 20036, data->Zi.valAsInt(), "Zi, ice situation." );
   bufr->addValue( 20037, data->Si.valAsInt(), "Si, ice development." );
   bufr->addValue( 20038, data->Di, "Di, bearing of ice edge." );
}

