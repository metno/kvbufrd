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

#ifndef __ENCODEBUFRMANAGER_H__
#define __ENCODEBUFRMANAGER_H__

#include <boost/shared_ptr.hpp>
#include <map>
#include <list>
#include "BufrHelper.h"

class EncodeBufrBase;



class EncodeBufrManager
{
   class CountHelper
   {
      EncodeBufrManager &mgr;
   public:
      CountHelper( EncodeBufrManager *mgr_ ) : mgr( *mgr_ )
      {
         mgr.recursionCount++;
      }
      ~CountHelper() {
         mgr.recursionCount--;
      }
   };

   typedef std::map< int, boost::shared_ptr<EncodeBufrBase> > EncoderList;
   EncoderList encoders;
   int recursionCount;

   /*
    * @throw IdException
    */
   void encode( int templateid, BufrHelper &bufrHelper, int replicator=-1 );
   friend class EncodeBufrBase;

public:
   static int masterBufrTable;
   static BufrParamValidaterPtr paramValidater;
   

   EncodeBufrManager();

   bool addEncoder( EncodeBufrBase *encoder );



   /*
    * @throw IdException, NotImplementedException, logic_error
    */
   void encode( const BufrTemplateList &templateList,
                BufrHelper &bufrHelper );

   /*
    * @throw IdException, NotImplementedException, logic_error
    */
   void encode( BufrHelper &bufrHelper );
};

#endif
