/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: CommandQue.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $

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
#ifndef __THREAD_COMMANDBASE_H__
#define __THREAD_COMMANDBASE_H__

#include <deque>
#include <exception>
#include <iosfwd>
#include <iostream>
#include <list>
#include <ostream>
#include <string>
#include <tuple>

namespace threadutil {
/**
 * \addtogroup threadutil
 * @{
 */

/**
 * \brief Exception calss for the que.
 *
 * The que is suspened an dont accepts new messages.
 */
class QueSuspended : public std::exception
{
  const char* w = "QueSuspended";

public:
  const char* what() const noexcept override { return w; }
};

/**
 * \brief A base class to be used for all messages to be posted
 * on a CommandQue.
 */
class CommandBase
{
protected:
  std::string comment;

public:
  CommandBase() {}
  CommandBase(const std::string& comment_)
    : comment(comment_)
  {}
  virtual ~CommandBase() {}

  virtual void onGet();
  virtual void onPost();

  /**
   * \brief This command calls executeImpl and can be used as an
   * interceptor.
   *
   * It is nice to have if we need to do pre/post prossesing before
   * executeImpl is called.
   *
   * If overidden it must call executeImpl.
   */
  virtual bool execute();

  /**
   * \brief This function implements what the command has to do.
   */
  virtual bool executeImpl() = 0;
  virtual void debugInfo(std::ostream& info) const;

  const std::string& getComment() const { return comment; }

  void setComment(const std::string& comment_) { comment = comment_; }

  friend std::ostream& operator<<(std::ostream& os, const CommandBase& c)
  {
    c.debugInfo(os);
    return os;
  }
};

class PriorityCommandBase : virtual public CommandBase
{
public:
  PriorityCommandBase()
    : CommandBase()
  {}

  PriorityCommandBase(const std::string& comment_)
    : CommandBase(comment_)
  {}

  virtual ~PriorityCommandBase() {}

  virtual bool lessThan(const PriorityCommandBase* rhs) const = 0;

  friend std::ostream& operator<<(std::ostream& os,
                                  const PriorityCommandBase& c)
  {
    c.debugInfo(os);
    return os;
  }

  bool operator<(const PriorityCommandBase& rhs) const
  {
    return lessThan(&rhs);
  }

  bool operator>(const PriorityCommandBase& rhs) const
  {
    return rhs.lessThan(this);
  }

  bool operator==(const PriorityCommandBase& rhs) const
  {
    return !rhs.lessThan(this) && !lessThan(&rhs);
  }

  bool operator<=(const PriorityCommandBase& rhs) const
  {
    return *this < rhs || *this == rhs;
  }

  bool operator>=(const PriorityCommandBase& rhs) const
  {
    return *this > rhs || *this == rhs;
  }
};

class Priority2CommandBase : virtual public CommandBase
{
public:
  Priority2CommandBase()
    : CommandBase()
  {}

  Priority2CommandBase(const std::string& comment_)
    : CommandBase(comment_)
  {}

  virtual ~Priority2CommandBase() {}

  friend std::ostream& operator<<(std::ostream& os,
                                  const Priority2CommandBase& c)
  {
    c.debugInfo(os);
    return os;
  }

  typedef enum { INSERT, INSERT_BACK, REPLACE, CONTINUE } Action;

  virtual Action add(const Priority2CommandBase& e) = 0;
  virtual bool get() = 0;
};

class CommandQueueBase
{

protected:
  bool suspended;
  std::string name;

public:
  CommandQueueBase()
    : suspended(false){};
  CommandQueueBase(const CommandQueueBase&) = delete;
  CommandQueueBase(const CommandQueueBase&&) = delete;
  CommandQueueBase& operator=(const CommandQueueBase&) = delete;

  explicit CommandQueueBase(bool suspended_)
    : suspended(suspended_){};
  virtual ~CommandQueueBase(){};

  void setName(const std::string& n) { name = n; }
  std::string getName() const { return name; }
  virtual void postImpl(CommandBase* command) = 0;
  virtual void post(CommandBase* command);
  virtual void postAndBrodcastImpl(CommandBase* command) = 0;
  virtual void postAndBrodcast(CommandBase* command);
  virtual CommandBase* getImpl(int timeoutInSeconds = 0) = 0;
  virtual CommandBase* get(int timeoutInSeconds = 0);

  virtual CommandBase* peek(int timeoutInSeconds = 0) = 0;

  /**
   * \brief remove, removes the command com from the que.
   */
  virtual CommandBase* remove(CommandBase* com) = 0;

  /**
   * \brief removeAll, removes all commands currently in
   * the que.
   *
   * The commands is returned in a list so the
   * caller can decide what shall be done with them.
   */
  virtual std::list<CommandBase*>* removeAll() = 0;

  /**
   * \brief clear, remove all the command in the que.
   */
  virtual void clear() = 0;

  virtual bool empty() = 0;
  virtual int size() = 0;
  virtual void brodcast() = 0;

  virtual void suspend() = 0;
  virtual void resume() = 0;
  virtual bool isSuspended() = 0;

  /**
   * \brief signal, make the get function return imidetly for all
   * threads that is blocked on  get().
   */
  virtual void signal() = 0;
};

/** @} */
}

#endif
