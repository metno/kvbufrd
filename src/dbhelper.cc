/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvDbBase.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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

#include <iostream>
#include <boost/lexical_cast.hpp>
#include "dbhelper.h"

using namespace std;
using namespace boost;

namespace dbhelper {

int
toInt( const std::string &dbVal )
{
	try {
		if( dbVal.empty() )
			return kvalobs::kvDbBase::INT_NULL;

		float f = lexical_cast<float>( dbVal );

		return static_cast<int>( f );
//		//Round to nearest int away from 0.
//		if( f < 0 )
//			return static_cast<int>( f - 0.5 );
//		else
//			return static_cast<int>( f + 0.5 );
	}
	catch( const bad_lexical_cast &ex) {
		cerr << "EXCEPTION: dbhelper::toInt: '" << dbVal << "':  "<< ex.what() << endl;
		return kvalobs::kvDbBase::INT_NULL;
	}
}

float
toFloat( const std::string &dbVal )
{
	try {
		if( dbVal.empty() )
			return kvalobs::kvDbBase::FLT_NULL;
		return lexical_cast<float>( dbVal );
	}
	catch( const bad_lexical_cast &ex  ) {
		cerr << "EXCEPTION: dbhelper::toFloat: " << ex.what() << endl;
		return kvalobs::kvDbBase::FLT_NULL;
	}
}

std::string
toString( const std::string &dbVal )
{
	if( dbVal.empty() )
		return kvalobs::kvDbBase::TEXT_NULL;

	return dbVal;
}


}
