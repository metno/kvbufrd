/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: CommandQue.cc,v 1.7.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include "CommandPriorityQueue.h"

namespace threadutil{

CommandPriorityQueue::CommandPriorityQueue()
    : CommandQueueBase(false) {
}

CommandPriorityQueue::CommandPriorityQueue(bool suspended_)
    : CommandQueueBase(suspended_) {
}

CommandPriorityQueue::~CommandPriorityQueue() {
  clear();
}

void CommandPriorityQueue::add(PriorityCommandBase *e) {
  auto it=que.begin();
  
  while( it!=que.end() && *dynamic_cast<PriorityCommandBase*>(*it) < *e ) 
    ++it;

  if( it==que.end())
    que.push_back(e);
  else
    que.insert(it, e);
}

void CommandPriorityQueue::postImpl(CommandBase *command) {
  Lock lock(m);

  if (suspended) {
    //std::cerr << "CommandPriorityQueue::post: suspended '" << name << "' #: " << que.size()<<"\n";  
    throw QueSuspended();
  }

  auto p = dynamic_cast<PriorityCommandBase*>(command);

  if( !p ) {
    //std::cerr << "FATAL: CommandPriorityQueue: postAndBroacast, input command must be derrived from PriorityCommandBase. aborting.\n\n";
    abort();
  }
  //std::cerr << "CommandPriorityQueue::post: '" << name <<"' #: " << que.size()<<"\n";  
  add(p);
}

void CommandPriorityQueue::postAndBrodcastImpl(CommandBase *command) {
  Lock lock(m);

  if (suspended) {
    //std::cerr << "CommandPriorityQueue::postAndBrodcast: suspended '" << name <<"' #: " << que.size() << "\n";  
    throw QueSuspended();
  }

  auto p = dynamic_cast<PriorityCommandBase*>(command);

  if( !p ) {
    //std::cerr << "FATAL: CommandPriorityQueue: postAndBroacast, input command must be derrived from PriorityCommandBase. aborting.\n\n";
    abort();
  }

  //std::cerr << "CommandPriorityQueue::postAndBrodcast: '" << name <<"' #: " << que.size() << "\n";  
  add(p);
  
  cond.notify_all();
}

CommandBase*
CommandPriorityQueue::peek(int timeout) {
  Lock lk(m);

  if (que.empty()) {
    if (suspended) {
      //std::cerr << "CommandPriorityQueue::peek: suspended '" << name <<"' #: " << que.size() << "\n";  
      throw QueSuspended();
    }

    if (timeout == 0) {
      while (que.empty()) {
        cond.wait(lk);
        if (suspended) {
          //std::cerr << "CommandPriorityQueue::peek: suspended '" << name <<"' #: " << que.size() << "\n";  
          throw QueSuspended();
        }
      }
    } else {
      cond.wait_for(lk, std::chrono::seconds(timeout));

      if (suspended && que.empty()) {
        //std::cerr << "CommandPriorityQueue::peek: suspended '" << name <<"' #: " << que.size() << "\n";  
        throw QueSuspended();
      }

      if (que.empty()) {
        //std::cerr << "CommandPriorityQueue::peek: ret nullptr '" << name <<"' #: " << que.size() << "\n";  
        return nullptr;
      }
    }
  }
  CommandBase *tmp = que.front();
  //std::cerr << "CommandPriorityQueue::peek: ret '" << name <<"' #: " << que.size() << "\n";  
  return tmp;
}

CommandBase*
CommandPriorityQueue::getImpl(int timeout) {
  Lock lk(m);

  if (que.empty()) {
    if (suspended) {
      //std::cerr << "CommandPriorityQueue::get: suspended '" << name <<"' #: " << que.size() << "\n";  
      throw QueSuspended();
    }

    if (timeout == 0) {
      while (que.empty()) {
        if (suspended) {
          //std::cerr << "CommandPriorityQueue::get: suspended '" << name <<"' #: " << que.size() << "\n";  
          throw QueSuspended();
        }
        cond.wait(lk);
      }
    } else {
      cond.wait_for(lk, std::chrono::seconds(timeout));

      if (que.empty()) {
        if (suspended) {
          //std::cerr << "CommandPriorityQueue::get: suspended '" << name <<"' #: " << que.size() << "\n";  
          throw QueSuspended();
        }
        //std::cerr << "CommandPriorityQueue::get: ret nullptr '" << name <<"' #: " << que.size() << "\n";  
        return nullptr;
      }
    }
  }

  CommandBase *tmp = que.front();
  que.pop_front();
  //std::cerr << "CommandPriorityQueue::get: ret '" << name <<"' #: " << que.size() << "\n";  
  return tmp;
}

CommandBase*
CommandPriorityQueue::remove(CommandBase *com) {
  Lock lck(m);

  if (que.empty())
    return 0;

  QueIterator it = que.begin();

  for (; it != que.end(); it++) {
    if (*it == com) {
      que.erase(it);
      return com;
    }
  }

  return 0;
}

void CommandPriorityQueue::clear() {
  Lock lck(m);
  for (QueIterator it = que.begin(); it != que.end(); it++)
    delete *it;
  que.clear();
}

std::list<CommandBase*>*
CommandPriorityQueue::removeAll() {
  std::list<CommandBase*> *toReturn;
  Lock lck(m);

  if (que.empty())
    return 0;

  QueIterator it = que.begin();

  try {
    toReturn = new std::list<CommandBase*>();
  } catch (...) {
    return 0;
  }

  for (; it != que.end(); it++) {
    toReturn->push_back(*it);
  }

  que.clear();
  return toReturn;
}
bool CommandPriorityQueue::empty() {
  Lock lck(m);

  return que.empty();
}

int CommandPriorityQueue::size() {
  Lock lck(m);

  return que.size();
}

void CommandPriorityQueue::brodcast() {
  Lock lck(m);

  if (!que.empty())
    cond.notify_all();
}

void CommandPriorityQueue::suspend() {
  Lock lk(m);

  if (suspended)
    return;

  suspended = true;
  cond.notify_all();
}

void CommandPriorityQueue::resume() {
  Lock lk(m);

  if (!suspended)
    return;

  suspended = false;
  cond.notify_all();
}

void CommandPriorityQueue::signal() {
  Lock lk(m);
  cond.notify_all();
}

}