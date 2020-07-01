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
#ifndef __THREAD_COMMAND_PRIORITY_QUEUE_H__
#define __THREAD_COMMAND_PRIORITY_QUEUE_H__

#include <iosfwd>
#include <string>
#include <deque>
#include <mutex>
#include <list>
#include <condition_variable>
#include "CommandBase.h"

namespace threadutil {
/**
 * \addtogroup threadutil
 * @{
 */


/**
 * \brief A FIFO que to comunicate between threads.
 */
class CommandPriorityQueue: virtual public CommandQueueBase {

 protected:
  typedef std::unique_lock<std::mutex>  Lock;
  typedef std::list<CommandBase*> Que;
  typedef std::list<CommandBase*>::iterator QueIterator;
  typedef std::list<CommandBase*>::const_iterator QueCIterator;

  std::mutex m;
  std::condition_variable cond;
  Que que;
  
  void add(PriorityCommandBase *e);
 public:
  CommandPriorityQueue();
  CommandPriorityQueue(const CommandPriorityQueue &)=delete;
  CommandPriorityQueue(const CommandPriorityQueue &&)=delete;
  CommandPriorityQueue& operator=(const CommandPriorityQueue &)=delete;
  

  explicit CommandPriorityQueue(bool suspended);
  virtual ~CommandPriorityQueue();

  void postImpl(CommandBase *command) override;
  void postAndBrodcastImpl(CommandBase *command) override;
  CommandBase *getImpl(int timeoutInSeconds = 0) override;

  CommandBase *peek(int timeoutInSeconds = 0) override;

  /**
   * \brief remove, removes the command com from the que.
   */
  CommandBase *remove(CommandBase *com) override;

  /**
   * \brief removeAll, removes all commands currently in
   * the que.
   *
   * The commands is returned in a list so the
   * caller can decide what shall be done with them.
   */
  std::list<CommandBase*> *removeAll() override;

  /**
   * \brief clear, remove all the command in the que.
   */
  void clear() override;

  bool empty() override;
  int size() override;
  void brodcast() override;

  void suspend() override;
  void resume() override;
  bool isSuspended()  override{
    Lock lk(m);
    return suspended;
  }

  /**
   * \brief signal, make the get function return imidetly for all
   * threads that is blocked on  get().
   */
  void signal() override;
};

/** @} */
}

#endif
