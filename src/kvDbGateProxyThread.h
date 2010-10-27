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

#ifndef __kvDbGateProxyTread_H__
#define __kvDbGateProxyTread_H__

#include <stdexcept>
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <kvalobs/kvDbGate.h>

namespace kvalobs {


class KvDbGateResultColumnValue
{
   friend class KvDbGateResultRow;
   const std::string *val;

   KvDbGateResultColumnValue( const std::string *value );
   KvDbGateResultColumnValue operator=( const KvDbGateResultColumnValue &v );
   KvDbGateResultColumnValue();
   KvDbGateResultColumnValue( const KvDbGateResultColumnValue &v );

public:

   std::string value()const;
   float asFloat()const;
   double asDouble()const;
   int    asInt()const;
};

class KvDbGateResultRow
{
   KvDbGateResultRow();
   std::list< std::string > *colNames;
   std::vector<std::string> row;

public:

   KvDbGateResultRow( int cols );
   KvDbGateResultRow( const KvDbGateResultRow &row );

   KvDbGateResultRow operator=(const KvDbGateResultRow &rhs );
   void setColNames( std::list< std::string > *colNames );
   void setColValue( int i, const std::string &val );


   int size()const { return row.size(); }

   /**
    * Returns the element at the index.
    * @param index the index [0, size>.
    * @return The value.
    * @exception std::range_error.
    */
   KvDbGateResultColumnValue operator[](int index )const;

   /**
    * Returns the element from the column
    * with name colName.
    * @param colName The name of the column.
    * @return The value.
    * @exception std::range_error if the named column do not exist.
    */
   KvDbGateResultColumnValue operator[]( const std::string &colName )const ;

   /**
    * Returns the element at the index.
    * @param index the index [0, size>.
    * @return The value.
    * @exception std::range_error.
    */
   KvDbGateResultColumnValue at( int index )const{ return (*this)[index] ; }

   /**
    * Returns the element from the column
    * with name colName.
    * @param colName The name of the column.
    * @return The value.
    * @exception std::range_error if the named column do not exist.
    */
   KvDbGateResultColumnValue at( const std::string &colName )const { return (*this)[colName] ; }

};

class KvDbGateResult
{
   std::list< std::string > colNames;
   std::list< KvDbGateResultRow > resultSet;

public:
   typedef std::list< KvDbGateResultRow >::iterator iterator;
   typedef std::list< KvDbGateResultRow >::const_iterator const_iterator;

   KvDbGateResult();
   void setColNames( const std::list< std::string > &colNames );
   void addRow( KvDbGateResultRow &row );

   int size()const{ return resultSet.size(); }
   const_iterator begin()const { return resultSet.begin(); }
   const_iterator end()const { return resultSet.end(); }
   iterator begin() { return resultSet.begin(); }
   iterator end() { return resultSet.end(); }
};


class KvDbGateCommand : public dnmi::thread::CommandBase
{
protected:
   kvDbGate *gate;
   dnmi::thread::CommandQue &retQue;
   dnmi::db::Connection *con;

public:
   KvDbGateCommand( dnmi::thread::CommandQue &retQue_, kvDbGate &gate )
      : gate( &gate ), retQue( retQue_ ), con( 0 )
   {}
   KvDbGateCommand( dnmi::thread::CommandQue &retQue_ )
         : gate( 0 ), retQue( retQue_ ), con( 0 )
      {}
   virtual ~KvDbGateCommand() {}


   void setConnection( dnmi::db::Connection *con );
   dnmi::db::Connection *getConnection(){ return con; }

};


class KvDbGateExecResult : public KvDbGateCommand
{
   const std::string query;
   int timeout;

public:
   kvDbGate::TError error;
   std::string errorString;
   KvDbGateResult &result;
   bool returnStatus;

   KvDbGateExecResult( dnmi::thread::CommandQue &retQue_,
                       KvDbGateResult &res, const std::string &query, int timeout   )
      : KvDbGateCommand( retQue_ ), query( query), timeout( timeout ), result( res )
   {
   }
   virtual bool   executeImpl();
};

class KvDbGateInsert : public KvDbGateCommand
{
   const  kvalobs::kvDbBase &elem;
   bool replace;
   miutil::miString tableName;

public:
   bool result;
   KvDbGateInsert( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const kvalobs::kvDbBase &elem,
                   const std::string &tblName,
                   bool replace)
      : KvDbGateCommand( retQue_, gate ), elem( elem ), replace( replace ), tableName( tblName )
   {
   }
   virtual bool   executeImpl() {
      result = gate->insert( elem, tableName, replace );
      retQue.postAndBrodcast( this );
      return true;
   }
};

class KvDbGateUpdate : public KvDbGateCommand
{
   const  kvalobs::kvDbBase &elem;
   miutil::miString tableName;

public:
   bool result;
   KvDbGateUpdate( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const kvalobs::kvDbBase &elem,
                   const std::string &tblName )
      : KvDbGateCommand( retQue_, gate ), elem( elem ),  tableName( tblName )
   {
   }
   virtual bool   executeImpl() {
      result = gate->update( elem, tableName );
      retQue.postAndBrodcast( this );
      return true;
   }
};

class KvDbGateReplace : public KvDbGateCommand
{
   const  kvalobs::kvDbBase &elem;
   miutil::miString tableName;

public:
   bool result;
   KvDbGateReplace( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const kvalobs::kvDbBase &elem,
                   const std::string &tblName )
      : KvDbGateCommand( retQue_, gate ), elem( elem ),  tableName( tblName )
   {
   }

   virtual bool   executeImpl() {
      result = gate->replace( elem, tableName );
      retQue.postAndBrodcast( this );
      return true;
   }
};

class KvDbGateRemove : public KvDbGateCommand
{
   const  kvalobs::kvDbBase *elem;
   miutil::miString tableName;
   std::string query;

public:
   bool result;
   KvDbGateRemove( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const kvalobs::kvDbBase &elem,
                   const std::string &tblName )
      : KvDbGateCommand( retQue_, gate ), elem( &elem ),  tableName( tblName )
   {
   }

   KvDbGateRemove( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const std::string &query )
      : KvDbGateCommand( retQue_, gate ), elem( 0 ), query( query )
   {
   }

   virtual bool   executeImpl() {
      if( elem )
         result = gate->remove( *elem, tableName );
      else
         result = gate->remove( query );

      retQue.postAndBrodcast( this );
      return true;
   }
};


class KvDbGateExec : public KvDbGateCommand
{
   std::string query;

public:
   bool result;
   KvDbGateExec( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const std::string &query)
      : KvDbGateCommand( retQue_, gate ), query( query )
   {
   }

   virtual bool   executeImpl() {
      result = gate->exec( query );
      retQue.postAndBrodcast( this );
      return true;
   }
};

template <class T>
class KvDbGateInsertList : public KvDbGateCommand
{
   typename std::list<T> data;
   bool replace;
   miutil::miString tableName;

public:
   bool result;
   KvDbGateInsertList( dnmi::thread::CommandQue &retQue_, kvDbGate &gate, const std::list<T>& li , bool replace=false,
                       const miutil::miString &tblName="")
      : KvDbGateCommand( retQue_, gate ), data( li ), replace( replace ), tableName( tblName )
   {
   }
   virtual bool   executeImpl() {
      result = gate->insert( data, replace, tableName );
      retQue.postAndBrodcast( this );
      return true;
   }
};

template <class T>
class KvDbGateSelect : public KvDbGateCommand
{
   miutil::miString query;
   miutil::miString tableName;

public:
   typename std::list<T> data;
   bool result;
   KvDbGateSelect( dnmi::thread::CommandQue &retQue_, kvDbGate &gate,
                   const miutil::miString &q="",
                   const miutil::miString &tblName="" )
      : KvDbGateCommand( retQue_, gate ), query( q ), tableName( tblName )
   {
   }
   virtual bool   executeImpl() {
      result = gate->select( data, query, tableName );
      retQue.postAndBrodcast( this );
      return true;
   }
};

class ConnectionFactory
{
public:
   virtual ~ConnectionFactory(){}
   virtual dnmi::db::Connection* newConnection()=0;
   virtual void releaseConnection( dnmi::db::Connection *con )=0;
};


class KvDbGateProxyThread {
   //KvDbGateProxyThread( const KvDbGateProxyThread &);
   KvDbGateProxyThread& operator=( const KvDbGateProxyThread &);

   boost::shared_ptr<bool>  joinable_;
   boost::shared_ptr<boost::thread> thread;
   boost::shared_ptr<ConnectionFactory> connectionFactory;

public:
   boost::shared_ptr<dnmi::thread::CommandQue> dbQue;

   KvDbGateProxyThread( boost::shared_ptr<ConnectionFactory> conFactory );
   KvDbGateProxyThread( const KvDbGateProxyThread &cp )
      : dbQue( cp.dbQue ), joinable_( cp.joinable_ ),
        thread( cp.thread ), connectionFactory( cp.connectionFactory )
   {}

   void operator()();
   bool joinable(){ return *joinable_; }
   void join(){ if(thread) thread->join(); }
   void start();
};

}
#endif
