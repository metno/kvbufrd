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

#ifndef __BUFREXCEPTION_H__
#define __BUFREXCEPTION_H__

#include <exception>
#include <string>
#include <sstream>

class BufrException : public std::exception
{
   std::string message;

public:
   BufrException( const std::string &msg )
      : message( msg ) {}
   ~BufrException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};

class NotImplementedException : public std::exception
{
   std::string message;

public:
   NotImplementedException( const std::string &msg )
      : message( msg ) {}
   ~NotImplementedException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};


class EncodeException : public std::exception
{
   std::string message;

public:
   EncodeException( const std::string &msg )
      : message( msg ) {}
   ~EncodeException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};


class TypeException : public std::exception
{
   std::string message;

public:
   TypeException(  int id, const std::string &msg  ){
      std::ostringstream o;

      o << "Wrong type: " << id << ".";
      if( ! msg.empty() )
         o << " " << msg;

      message = o.str();
   }

   ~TypeException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};


class CodeException : public std::exception
{
   std::string message;

public:
   CodeException(  int id, const std::string &msg  ){
      std::ostringstream o;

      o << "Invalid code: " << id << ".";
      if( ! msg.empty() )
         o << " " << msg;

      message = o.str();
   }

   ~CodeException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};


class IdException : public std::exception
{
   std::string message;

public:
   IdException( int id, const std::string &msg="" ) {
      std::ostringstream o;

      o << "Unknown id: " << id;
      if( ! msg.empty() )
         o << " : " << msg;
      else
         o << ".";

      message = o.str();
   }

   ~IdException() throw (){}

   const char* what()const throw() { return message.c_str(); }
};




#endif
