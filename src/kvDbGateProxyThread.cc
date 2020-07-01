/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvDbGate.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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

#include <float.h>
#include <stdio.h>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "kvDbGateProxyThread.h"

namespace {
void
createGlobalLogger(const std::string &id)
{
   using namespace milog;
   using namespace std;

   try{
     /*FIXME: Remove the comments when the needed functionality is
      * in effect on an operational machin.
      *
      *if( LogManager::hasLogger(id) )
      *  return true;
      */
     FLogStream *logs=new FLogStream(2, 204800); //200k
     ostringstream ost;

     ost << kvPath("logdir") << "/kvbufrd/" << id << ".log";

     if(logs->open(ost.str())){
        if(!LogManager::createLogger(id, logs)){
           delete logs;
           return ;
        }

        return;
     }else{
        LOGERROR("Cant open the logfile <" << ost.str() << ">!");
        delete logs;
        return;
     }
  }
  catch(...){
     LOGERROR("Cant create a logstream for LOGID " << id);
     return;
  }
}
}


namespace kvalobs {

using namespace std;

KvDbGateResultColumnValue::
KvDbGateResultColumnValue( const std::string *value )
: val( value )
{
}

KvDbGateResultColumnValue::
KvDbGateResultColumnValue()
:val(0)
{
}

KvDbGateResultColumnValue::
KvDbGateResultColumnValue( const KvDbGateResultColumnValue &v )
: val( v.val )
{
}


KvDbGateResultColumnValue
KvDbGateResultColumnValue::
operator=( const KvDbGateResultColumnValue &v )
{
   if( this != &v )
      val = v.val;
   return *this;
}

std::string
KvDbGateResultColumnValue::
value()const
{
   if( !val )
      return "";

   return *val;
}

float
KvDbGateResultColumnValue::
asFloat()const
{
   float f=FLT_MAX;
   if( val && val->length()>0  )
      sscanf( val->c_str(),"%f", &f);

   return f;

}

double
KvDbGateResultColumnValue::
asDouble()const
{
   double f=DBL_MAX;
   if( val && val->length()>0  )
      sscanf( val->c_str(),"%lf", &f);

   return f;
}

int
KvDbGateResultColumnValue::
asInt()const
{
   int i = INT_MAX;

   if( val && val->length()>0 )
      sscanf( val->c_str(),"%d", &i);

   return i;
}


KvDbGateResultRow::
KvDbGateResultRow( int cols )
   : row( cols ), colNames( 0  )
{
}

KvDbGateResultRow::
KvDbGateResultRow( const KvDbGateResultRow &row )
   : row( row.row ), colNames( row.colNames )
{
}

KvDbGateResultRow
KvDbGateResultRow::
operator=(const KvDbGateResultRow &rhs )
{
   if( this != &rhs ) {
      row = rhs.row;
      colNames = rhs.colNames;
   }
   return *this;
}

void
KvDbGateResultRow::
setColNames( std::list< std::string > *colNames_ )
{
   colNames = colNames_;
}

void
KvDbGateResultRow::
setColValue( int i, const std::string &val )
{
   if( i>= row.size() )
      throw std::range_error("EXCEPTION: KvDbGateResultRow::setColValue: range_error.");

   row[i]=val;
}


/**
* Returns the element at the index.
* @param index the index [0, size>.
* @return The value.
* @exception std::range_error.
*/
KvDbGateResultColumnValue
KvDbGateResultRow::
operator[](int index )const
{
   if( index >= row.size() )
      throw std::range_error("EXCEPTION: KvDbGateResultRow: range_error.");

   return KvDbGateResultColumnValue( &row[index] );
}

/**
* Returns the element from the coloumn
* with name colName.
* @param index the index [0, size>.
* @return The value.
* @exception std::range_error.
*/
KvDbGateResultColumnValue
KvDbGateResultRow::
operator[]( const std::string &colName_ )const
{
   int index=0;
   for( list<string>::const_iterator it=colNames->begin(); it != colNames->end(); ++it, ++index ) {
      if( *it == colName_ )
         return KvDbGateResultColumnValue( &row[index] );
   }

   throw std::range_error("EXCEPTION: KvDbGateResultRow: range_error.");
}

//   std::list< std::string > colNames;
//   std::list< KvDbGateResultRow > resultSet;

KvDbGateResult::
KvDbGateResult()
{
}

void
KvDbGateResult::
setColNames( const std::list< std::string > &colNames_ )
{
   colNames = colNames_;
}

void
KvDbGateResult::
addRow( KvDbGateResultRow &row )
{
   row.setColNames( &colNames );
   resultSet.push_back( row );
}

void
KvDbGateCommand::
setConnection( dnmi::db::Connection *con_ )
{
   con = con_;

   if( gate )
      gate->set( con_ );
}

std::string KvDbGateCommand::name()const{
   return "";
}



KvDbGateDoExecCommand::KvDbGateDoExecCommand(){

}

KvDbGateDoExecCommand::~KvDbGateDoExecCommand(){
}

bool KvDbGateDoExecCommand::executeImpl() {
   try {
      ret = doExec(getConnection());
   } catch(const std::exception &e) {
      err << "KvDbGateDoExecCommand::executeImpl: Exception: " << e.what();  
   }

   retQue->postAndBrodcast( this );
   return true;
}

bool
KvDbGateExecResult::
executeImpl()
{
   returnStatus = false;

   if( ! con ) {
      error = kvDbGate::NotConnected;
      errorString = "No connection.";
      retQue->postAndBrodcast( this );
      return false;
   }

   bool retry=true;
   time_t     now;
   time_t     myTimeout;
   dnmi::db::Result *rs=0;

   time(&now);
   myTimeout=now+timeout;

   while( retry && now<=myTimeout ){
      retry=false;

      try{
         rs=0;
         rs=con->execQuery( query );
      }
      catch(dnmi::db::SQLBusy &ex){
         time(&now);
         error = kvDbGate::Busy;
         errorString = ex.what();
         retry=true;
      }
      catch(dnmi::db::SQLNotConnected &ex){
         delete rs;
         error=kvDbGate::NotConnected;
         errorString = ex.what();
         retQue->postAndBrodcast( this );
         return false;
      }
      catch(dnmi::db::SQLException & ex) {
         delete rs;
         error = kvDbGate::Error;
         errorString = ex.what();
         retQue->postAndBrodcast( this );
         return false;
      }
      catch(...) {
         delete rs;
         error =kvDbGate::UnknownError;
         errorString = "Unknown error! (UNEXPECTED EXCEPTION)";
         retQue->postAndBrodcast( this );
         return false;
      }
   }

   if(!rs) {
      error = kvDbGate::NoError;
      errorString = "The result from the DB query is NULL.";
      retQue->postAndBrodcast( this );
      return false;
   }

   //result.setColNames( rs->getFieldNames() );

   time(&now);
   myTimeout=now+timeout;
   retry=true;
   bool fieldNameSet=false;

   while( myTimeout>=now && retry){
      retry=false;

      try{
         while( rs->hasNext() ) {
            time(&now);
            myTimeout=now+timeout;

            dnmi::db::DRow & row = rs->next();

            KvDbGateResultRow resRow( row.fields() );

            if( ! fieldNameSet ) {
               result.setColNames( row.getFieldNames() );
               fieldNameSet=true;
            }

            int i=0;
            for(dnmi::db::CIDRow it=row.begin(); it != row.end(); ++it, i++){
               try{
                  resRow.setColValue( i, *it );
               }
               catch( const dnmi::db::SQLException &ex) {
                  error = kvDbGate::Error;
                  errorString = ex.what();
                  return false;
               }
               catch( const std::exception &ex ) {
                  error = kvDbGate::UnknownError;
                  errorString = ex.what();
                  return false;
               }
               catch(...){
                  error = kvDbGate::Error;
                  errorString = "An error occured while fetching data from a db row.";
                  delete rs;
                  return false;
               }
            }

            result.addRow( resRow );
         }

         delete rs;
      }
      catch(dnmi::db::SQLNotConnected &ex){
         delete rs;
         error = kvDbGate::NotConnected;
         errorString=ex.what();
         returnStatus = false;
         retQue->postAndBrodcast( this );
         return false;
      }
      catch(dnmi::db::SQLBusy &ex){
         delete rs;
         error = kvDbGate::Busy;
         errorString = ex.what();
         retry = true;
         time(&now);
      }
      catch(dnmi::db::SQLException & ex) {
         delete rs;
         error = kvDbGate::Error;
         errorString = ex.what();
         retQue->postAndBrodcast( this );
         return false;
      }
      catch(...) {
         delete rs;
         error = kvDbGate::UnknownError;
         errorString ="Unknown error! (UNEXPECTED EXCEPTION)";
         retQue->postAndBrodcast( this );
         return false;
      }
   }

   returnStatus = true;
   error = kvDbGate::NoError;
   errorString = "";
   retQue->postAndBrodcast( this );
   return true;
}




KvDbGateProxyThread::
KvDbGateProxyThread( std::shared_ptr< ConnectionFactory> conFactory )
   : joinable_( new bool(false) ), connectionFactory( conFactory ),
     dbQue( std::shared_ptr<threadutil::CommandQueue>( new threadutil::CommandQueue(false) ) )
{
   dbQue->setName("dbThread");
}

void
KvDbGateProxyThread::
operator()()
{
   using namespace threadutil;
   const int QUESIZE=0;
   const int PROBETIME=1;
   bool quit=false;
   CommandBase *command;
   string logid("DbThread");
   time_t printQueSize; //Print the quesize every 15'th minute and quesize is greater than 10.
   time_t now;
   int queSize;
   dnmi::db::Connection *con;

   createGlobalLogger( logid );

   time( &printQueSize );
   printQueSize += PROBETIME;

   LOGDEBUG("DbThread started.");
   IDLOGDEBUG( logid, "DbThread started.");

   while( !quit ) {
      time( &now );

      if( now > printQueSize ) {
         printQueSize = now + PROBETIME;
         queSize = dbQue->size();

         if( queSize > QUESIZE ) {
            IDLOGINFO( logid, "Que size: " << queSize );
         }
      }

      try {
         command = dbQue->get( 1 );

         if( ! command )
            continue;

         KvDbGateCommand *gateCommand = dynamic_cast<KvDbGateCommand*>( command );

         if( ! gateCommand ) {
            LOGFATAL( "KvDbGateProxyThread: Not a valid gate command." );
            continue;
         }


         con = connectionFactory->newConnection();

         if( ! con ) {
            LOGFATAL( "KvDbGateProxyThread: No connection." );
            exit(128);
         }

         gateCommand->setConnection( con );
         gateCommand->execute();

      }
      catch ( const QueSuspended &ex ) {
         cerr << "KvDbGateProxyThread: Exception: QueSuspended (quit)\n";

         quit = true;
      }
      catch( ... ) {
         //NOOP
      }

      if( con ) {
         connectionFactory->releaseConnection( con );
         con = 0;
      }
   }

   LOGDEBUG("DbThread stopped.");
   IDLOGDEBUG( logid, "DbThread stopped.");
}

void
KvDbGateProxyThread::
start()
{
   thread.reset( new std::thread( *this ) );
}

}
