/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: App.h,v 1.13.2.9 2007/09/27 09:02:22 paule Exp $

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

#ifndef __checkfile_h__
#define __checkfile_h__

#include <unistd.h>

/**
 * checkfile checks the existence of the file given with filename and the the permissions given with
 * mode.
 *
 * The mode is the same as for the standard function access in \b unistd.h
 *
 * Excerpt from the man page ACCESS(2).
 * The  mode  specifies  the  accessibility  check(s) to be performed, and is either the value F_OK,
 * or a mask consisting of the bitwise OR of one or more of R_OK, W_OK, and X_OK. F_OK tests for the
 * existence of the file.  R_OK, W_OK, and X_OK test whether the file exists and grants read, write,
 * and execute permissions, respectively.
 *
 * @param filename The name of the file.
 * @param mode What to check.
 * @param error An string holding the error information in case of error.
 * @return true if the file exist and the has the needed access permission and false otherwise.
 */
bool
checkfile( const std::string &filename, int mode, std::string &error );

#endif
