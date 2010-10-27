/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDbGate.cc,v 1.14.2.4 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <time.h>
#include <kvDbGateProxy.h>
#include <sstream>
//#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi::db;


bool 
kvalobs::
kvDbGateProxy::
insert(const kvalobs::kvDbBase &elem, 
       const std::string &tblName,
       bool replace)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateInsert *command = new KvDbGateInsert( retQue, gate, elem, tblName, replace );
   bool retStatus=false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus = static_cast<KvDbGateInsert*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}

bool 
kvalobs::
kvDbGateProxy::
insert(const kvalobs::kvDbBase &elem, bool replace)
{
   return insert(elem, elem.tableName(), replace);
}

bool 
kvalobs::
kvDbGateProxy::
insert(const kvalobs::kvDbBase &elem, 
	const char *tblName,
	bool replace)
{
  return insert(elem, string(tblName), replace);
}


bool 
kvalobs::
kvDbGateProxy::
update(const kvalobs::kvDbBase &elem)
{
  return update(elem, elem.tableName());
}

bool 
kvalobs::
kvDbGateProxy::
update(const kvalobs::kvDbBase &elem, const char *tblName)
{
  return update(elem, string(tblName));
}

bool 
kvalobs::
kvDbGateProxy::
update(const kvalobs::kvDbBase &elem, const std::string &tblName)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateUpdate *command = new KvDbGateUpdate( retQue, gate, elem, tblName );
   bool retStatus=false;
   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus = static_cast<KvDbGateUpdate*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}

bool 
kvalobs::
kvDbGateProxy::
replace(const kvalobs::kvDbBase &elem)
{
  return replace(elem, elem.tableName());
}

bool 
kvalobs::
kvDbGateProxy::
replace(const kvalobs::kvDbBase &elem, const  char *tblName)
{
  return replace(elem, string(tblName));
}

bool 
kvalobs::
kvDbGateProxy::
replace(const kvalobs::kvDbBase &elem, const std::string &tblName)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateReplace *command = new KvDbGateReplace( retQue, gate, elem, tblName );
   bool retStatus = false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus =  static_cast<KvDbGateReplace*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }
   delete command;
   return retStatus;
}

bool 
kvalobs::
kvDbGateProxy::
remove(const kvalobs::kvDbBase &elem)
{
  return remove(elem, elem.tableName());
}

bool 
kvalobs::
kvDbGateProxy::
remove(const kvalobs::kvDbBase &elem, const char *tblName)
{
  return remove(elem, string(tblName));
}

bool 
kvalobs::
kvDbGateProxy::
remove(const kvalobs::kvDbBase &elem, const std::string &tblName)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateRemove *command = new KvDbGateRemove( retQue, gate, elem, tblName );
   bool retStatus=false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus = static_cast<KvDbGateRemove*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}

bool 
kvalobs::
kvDbGateProxy::
remove(const std::string &query)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateRemove *command = new KvDbGateRemove( retQue, gate, query );
   bool retStatus=false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus = static_cast<KvDbGateRemove*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}

bool 
kvalobs::
kvDbGateProxy::
exec(const std::string &query)
{
   dnmi::thread::CommandQue retQue;
   errorFromExecResult = false;
   KvDbGateExec *command = new KvDbGateExec( retQue, gate, query );
   bool retStatus = false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      retStatus = static_cast<KvDbGateExec*>( ret )->result;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}


bool
kvalobs::
kvDbGateProxy::
exec( KvDbGateResult &result, const std::string &query )
{
   dnmi::thread::CommandQue retQue;
   KvDbGateExecResult *command = new KvDbGateExecResult( retQue, result, query, busytimeout() );
   errorFromExecResult = true;
   bool retStatus = false;

   try {
      dbQue->postAndBrodcast( command );
      dnmi::thread::CommandBase *ret = retQue.get();
      KvDbGateExecResult *result = static_cast<KvDbGateExecResult*>( ret );
      error = result->error;
      errorString = result->errorString;
      retStatus = result->returnStatus;
   }
   catch ( const dnmi::thread::QueSuspended &e) {
   }

   delete command;
   return retStatus;
}
