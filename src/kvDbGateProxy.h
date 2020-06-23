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
#ifndef __kvDbGateProxy_h__
#define __kvDbGateProxy_h__

#include <string>
#include <list>
#include <memory>
#include "CommandQueue.h"
#include "kvalobs/kvDbGate.h"
#include "kvDbGateProxyThread.h"

namespace kvalobs {

  /**
   * \addtogroup dbutility
   *
   * @{
   */

  /**
   * \brief implements a simplified interface on top of 
   *        \ref dbabstraction "database abstraction".
   *
   * All classes that use this class must be inherited from kvalobs::kvDbBase.
   */
  class kvDbGateProxy  {
  protected:
     kvDbGate gate;
     std::shared_ptr<threadutil::CommandQueue> dbQue;
     kvDbGate::TError error;
     std::string errorString;

     bool errorFromExecResult;
  public:
    
    kvDbGateProxy(std::shared_ptr<threadutil::CommandQueue> dbQue_ ):dbQue( dbQue_ ) {}
    ~kvDbGateProxy(){}
    
    /**
     * \brief get the error type.
     */
    kvDbGate::TError getError()const{ return errorFromExecResult?error:gate.getError(); }
    
    /**
     * \brief Get a description of the error.
     */
    std::string getErrorStr()const{ return errorFromExecResult?errorString:gate.getErrorStr(); }
    
    void busytimeout(int btInSecond){ gate.busytimeout( btInSecond ); }
    int  busytimeout()const{ return gate.busytimeout(); }

    void set(dnmi::db::Connection *con_){ gate.set( con_ );}

    bool valid()const{ return gate.valid(); }

    /**
     * \brief insert, insert an element into a table. 
     * 
     * If 'replace' is false
     * the element is inserted if the database allows it. There
     * may be duplicated rows or the request may be rejected because
     * of duplicates. If 'replace' is true it will delete the row before
     * inserting in case of rejected duplicates. It can still be duplicated
     * rows if the table allows it.
     *
     * The table to insert the element is elem.tableName().
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem the element to insert into the database
     * \param replace If an element exist shoul it be replaced.
     * \return true if the element is inserted into the database, false
     *        if not.
     */
    bool insert(const kvalobs::kvDbBase &elem, bool replace=false);


    /**
     * \brief insert an element into a the table \em tblName
     *
     * instead of the default table kvDbBase::tableName. 
     * If 'replace' is false
     * the element is inserted if the database allows it. There
     * may be duplicated rows or the request may be rejected because
     * of duplicates. If 'replace' is true it will delete the row before
     * inserting in case of rejected duplicates. It can still be duplicated
     * rows if the table allows it.
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem The element to insert into the database.
     * \param tblName The table to insert the element into, ie dont
     *                use the default table speciefied with elem.tableName().
     * \param replace If an element exist shoul it be replaced.
     * \return true if the element is inserted into the database, false
     *        if not.
     */
    bool insert(const kvalobs::kvDbBase &elem, 
		const std::string &tblName,
		bool replace=false);

    
    /**
     * \brief insert an element into a the table \em tblName
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \see insert(const kvalobs::kvDbBase &elem,const std::string &tblName,bool replace)

     */
    bool insert(const kvalobs::kvDbBase &elem, 
		const char *tblName,
		bool replace=false);


    template<class T>
    bool insert( const std::list<T>& li , bool replace=false,
                 const std::string &tblName="")
    {
       threadutil::CommandQueue retQue;
       errorFromExecResult = false;
       bool retStatus=false;

       KvDbGateInsertList<T> *command= new KvDbGateInsertList<T>( &retQue, &gate, li, replace, tblName );

       try {
          dbQue->postAndBrodcast( command );
          threadutil::CommandBase *ret = retQue.get();
          retStatus = static_cast<KvDbGateInsertList<T> *>( ret )->result;
       }
       catch ( const threadutil::QueSuspended &e) {
       }

       delete command;

       return retStatus;
    }
      
    


    /**
     * \brief replace an element in the database if it exist,
     *
     * otherwise  the element is inserted.
     * The table to insert the element is elem.tableName().
     *
     * \param elem The element to be replaced/inserted into the database.
     * \return true on success, false otherwise.
     *
     */
    bool replace(const kvalobs::kvDbBase &elem);


    /**
     * \brief replace an element in the table \em tblName 
     * 
     * in the database if it exist, otherwise the element is inserted.
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem the data to replace/insert.
     * \param tblName the table to insert the data into.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     */
    bool replace(const kvalobs::kvDbBase &elem, const std::string &tblName);


    /**
     * \brief replace an element in the table \em tblName 
     * 
     * in the database if it exist, otherwise the element is inserted.
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem the data to replace/insert.
     * \param tblName the table to insert the data into.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     */
    bool replace(const kvalobs::kvDbBase &elem, const  char *tblName);


    /**
     * \brief Update an element in the database.
     *
     * The element must exist for this funcion to succeed.
     *
     * \param elem the data to replace/insert.
     * \param tblName the table to insert the data into.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     */
    bool update(const kvalobs::kvDbBase &elem);

    /**
     * \brief update an element in the table \em tblName.
     * 
     * The element must exist.
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem the data to replace/insert.
     * \param tblName the table to insert the data into.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     */
    bool update(const kvalobs::kvDbBase &elem, const std::string &tblName);

    /**
     * \brief update an element in the table \em tblName.
     * 
     * The element must exist.
     *
     * \note the table specified with \em tblName must have the same 
     * layout as the table specified with kvDbBase::tableName()
     *
     * \param elem the data to replace/insert.
     * \param tblName the table to insert the data into.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     * \see update(const kvalobs::kvDbBase &elem,const std::string &tblName)
     */
    bool update(const kvalobs::kvDbBase &elem, const char *tblName);
    

    /**
     * \brief remove a element from the database.
     *
     * \param elem The element to be removed.
     * \return True on success and false otherwise.
     *
     * \note The function returns true independent of the element existed 
     *       before or not. It returns false if the element cant'be removed
     *       or there was an database error.
     */
    bool remove(const kvalobs::kvDbBase &elem);


    /**
     * \brief remove a element from the table \em tblName in the database.
     *
     * \param elem The element to be removed.
     * \param tblName the table to remove the element from.
     * \return True on success and false otherwise.
     *
     * \note The function returns true independent of the element existed 
     *       before or not. It returns false if the element cant'be removed
     *       or the there was an databse error.
     */
    bool remove(const kvalobs::kvDbBase &elem, const std::string &tblName);
    
    /**
     * \brief remove a element from the table \em tblName in the database.
     *
     * \param elem The element to be removed.
     * \param tblName the table to remove the element from.
     * \return True on success and false otherwise.
     *
     * \note The function returns true independent of the element existed 
     *       before or not. It returns false if the element cant'be removed
     *       or the there was an databse error.
     *
     * \see remove(const kvalobs::kvDbBase &elem,const std::string &tblName)
     */
    bool remove(const kvalobs::kvDbBase &elem, const char *tblName);

    
    /**
     * \brief remove elements from the database based on a \a query.
     *
     * The query executed is:
     * \verbatim
       DELETE FROM table WHERE condition
                   ^^^^^^^^^^^^^^^^^^^^^
                             (*)
       \endverbatim
     * You specify in the query the part marked with (*)
     *
     * \param query The query part (*).
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     * \note The function returns true independent of the query selected
     *       any record or not. 
     *       It returns false if the elements cant'be removed
     *       or the there was an database error.
     */
    bool remove(const std::string &query);

    /**
     *  \brief exec executes a query. The query must NOT return a result. 
     *
     *  You cant use this function to SELECT data.
     *
     * \param query the SQL query to execute.
     * \return true if no error occured and false otherwise. Use getErrString()
     *         or getError() to get information on the error.  
     */
    bool exec(const std::string &query);

    bool exec( KvDbGateResult &result, const std::string &query );


    bool doExec(KvDbGateDoExecCommand *cmd);

    /**
     * \brief selects rows from a table or view in the database.
     *
     * The query executed is on the form:
     * \verbatim
       SELECT * FROM tblName WHERE query
                             ^^^^^^^^^^^
			         (*)
       \endverbatim
     * Where it is the (*) part you must specify. You may use one of the
     * function in \ref kvqueries "common kvalobs query" as the query.
     *
     * \code 
       Example 
       
       Select all data at Oct 23 09:00:00 2004 for
       stationid 18700 and typeid 3.
       
       kvDbGate gate( a pointer to dnmi::db::Connection )
       std::list<kvalobs::kvData> dataList;
       miutil::miTime obsTime(2004, 10, 23, 9, 0, 0); 
       
       
       if(!gate.select(dataList, kvQueries::selectDataFromType(18700,3,obsTime)){
          cerr << "ERROR: " << gate.getErrorStr();
       }else{
          //Do stuff
       }
     * \endcode
     * \note if you specify tblName it must have the same layout or be a 
     *       subset of the table specified in kvDbBase::tableName()
     * 
     *
     * \param li a list to hold the result.
     * \param q the SQL query to execute.
     * \param tblName the table or view to query if we it is not 
     *                kvDbBase::tblName().
     * \return true if no error occured and false otherwise. Use getErrorStr()
     *         or getError() to get information on the error.  
     */
    template<class T>
    bool select( std::list<T>& li , const std::string &q="",
                 const std::string &tblName="")
    {
       threadutil::CommandQueue retQue;
       errorFromExecResult = false;
       KvDbGateSelect<T> *command = new KvDbGateSelect<T>( &retQue, &gate, q, tblName );
       bool retStatus=false;

       try {
          dbQue->postAndBrodcast( command );
          threadutil::CommandBase *ret = retQue.get();
          KvDbGateSelect<T> *retCommand = static_cast<KvDbGateSelect<T> *>( ret );
          li = retCommand->data;
          retStatus = retCommand->result;
       }
       catch ( const threadutil::QueSuspended &e) {
       }

       delete command;
       return retStatus;
    }
    
  
};

  /** @} */

}

#endif
