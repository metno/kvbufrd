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
#include <sstream>
#include <kvalobs/milog/milog.h>
#include "CommandPriority2Queue.h"

namespace threadutil{

CommandPriority2Queue::CommandPriority2Queue()
    : CommandQueueBase(false) {
}

CommandPriority2Queue::CommandPriority2Queue(bool suspended_)
    : CommandQueueBase(suspended_) {
}

CommandPriority2Queue::~CommandPriority2Queue() {
  clear();
}

void CommandPriority2Queue::add(Priority2CommandBase *e) {
  auto it=que.begin();
  Priority2CommandBase::Action action;

  while( it!=que.end() ) {
    action=e->add(*dynamic_cast<Priority2CommandBase*>(*it));
    if(action!=Priority2CommandBase::CONTINUE){
      break;
    }
    ++it;
  }
    
  
  if( it==que.end()) {
    que.push_back(e);
  } else {
    if( action == Priority2CommandBase::REPLACE) {
      delete *it;
      *it = e;
    } else {
      que.insert(it, e);
    }
  }

  if( que.size() > 1 ) {
    std::ostringstream o;
    o << "Add: '" << name << "' size: " << que.size() << "\n"
      << "   add: ";
    e->debugInfo(o);
    o << "--------------------\n";
    for( auto c : que ) {
      if(dynamic_cast<Priority2CommandBase*>(c) == e ) {
        o << "--> ";
      }
      c->debugInfo(o);
    }
    IDLOGDEBUG("priqueue", o.str());
  }
}

CommandBase *CommandPriority2Queue::select(bool peek)
{
  bool printDebugHeader=true;
  std::ostringstream o;
  auto it=que.begin();
 
  if( it == que.end()) {
    return nullptr;
  }

  Priority2CommandBase *e;

  for(;it!=que.end(); ++it) {
    e=dynamic_cast<Priority2CommandBase*>(*it);
 
    if( !e ) { //Should never happend. As it is checked at insert.
      LOGFATAL("CommandPriority2Queue::select: assert failed! '" << name << "'");
      IDLOGFATAL("priqueue", "select: assert failed! '" << name << "'");
      abort();
    }
 
    if( e->get() ) {
      if( !printDebugHeader ) {
        o << "Found: ";
        e->debugInfo(o);
      }
      break;
    }
    if(printDebugHeader) {
      o << "select: '" << name << "' que size: " << que.size() << "\n";
    }

    (*it)->debugInfo(o);
  }

  int qSize=que.size();
  CommandBase *cmd = nullptr;
  if(it == que.end() ) {
    if( !printDebugHeader ) {
      o <<"NO element found!\n";
    }
    if(!suspended ) {
      IDLOGDEBUG("priqueue", o.str());
      return nullptr;
    }
    o <<"select: SUSPENDED!\n";
    cmd = que.front();
    if( ! peek ) {
      que.pop_front();
    }
  } else {
    cmd = *it;
    if( !peek ) {
      que.erase(it);
    }
  }
  
  if( printDebugHeader ) {
    if( cmd ) {
      o <<"select: '" << name << "' remaining que size: " << qSize-1 << "\nFound: ";
      cmd->debugInfo(o);
    }
  }

  if( qSize > 0 ){
    std::string s=o.str();
    if( ! s.empty() ) {
      IDLOGDEBUG("priqueue",o.str());
    }
  }
  return cmd;
}

void CommandPriority2Queue::postImpl(CommandBase *command) {
  Lock lock(m);

  if (suspended) {
    LOGINFO("CommandPriorityQueue::post: suspended '" << name << "'");
    IDLOGINFO("priqueue","post: suspended '" << name << "'");
    throw QueSuspended();
  }
  
  auto p = dynamic_cast<Priority2CommandBase*>(command);

  if( !p ) {
    //std::cerr << "FATAL: CommandPriorityQueue: postAndBroacast, input command must be derrived from PriorityCommandBase. aborting.\n\n";
    abort();
  }
  
  add(p);
}

void CommandPriority2Queue::postAndBrodcastImpl(CommandBase *command) {
  Lock lock(m);

  if (suspended) {
    LOGINFO("CommandPriorityQueue::postAndBrodcast: suspended '" << name << "'");
    IDLOGINFO("priqueue","postAndBrodcast: suspended '" << name << "'");
    throw QueSuspended();
  }

  auto p = dynamic_cast<Priority2CommandBase*>(command);

  if( !p ) {
    //std::cerr << "FATAL: CommandPriorityQueue: postAndBroacast, input command must be derrived from PriorityCommandBase. aborting.\n\n";
    abort();
  }

  add(p);
    
  cond.notify_all();
}

CommandBase*
CommandPriority2Queue::peek(int timeout) {
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
  CommandBase *tmp = select(true);
  //std::cerr << "CommandPriorityQueue::peek: ret '" << name <<"' #: " << que.size() << "\n";  
  return tmp;
}

CommandBase*
CommandPriority2Queue::getImpl(int timeout) {
  Lock lk(m);
  std::ostringstream o;
  CommandBase *cmd=select(false);

  milog::LogLevel ll = milog::Logger::logger("priqueue").logLevel();
  
  if ( !cmd) {
    if (suspended && que.empty()) {
      LOGINFO("CommandPriorityQueue::getImpl: '" << name << "' suspended!");
      IDLOGINFO("priqueue","getImpl: '" << name << "' suspended!");
      throw QueSuspended();
    }

    if (timeout == 0) {
      while (!cmd ) {
        if (suspended && que.empty()) {
          LOGINFO("CommandPriority2Queue::getImpl: '" << name << "' suspended!");
          IDLOGINFO("priqueue","getImpl: '" << name << "' suspended! ");
          throw QueSuspended();
        }
        cond.wait(lk);
        cmd=select(false);
      }
    } else {
      cond.wait_for(lk, std::chrono::seconds(timeout));
      cmd=select(false);
      if (!cmd ) {
        if (suspended && que.empty()) {
          LOGINFO("CommandPriorityQueue::getImpl: '" << name << "' suspended!");
          IDLOGINFO("priqueue","getImpl: '" << name << "' suspended!");
          throw QueSuspended();
        }
        return nullptr;
      }
    }
  }

  if( ! cmd ) {
    cmd = select(false);
  }

  if(! que.empty()) {
    IDLOGINFO("priqueue", "getImpl '" << name << "' : que size: " << que.size());
  }
  std::string s=o.str();
  
  if( !s.empty() ) {
    IDLOGDEBUG("priqueue", o.str());
  }
  
  return cmd;
}

CommandBase*
CommandPriority2Queue::remove(CommandBase *com) {
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

void CommandPriority2Queue::clear() {
  Lock lck(m);
  for (QueIterator it = que.begin(); it != que.end(); it++)
    delete *it;
  que.clear();
}

std::list<CommandBase*>*
CommandPriority2Queue::removeAll() {
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
bool CommandPriority2Queue::empty() {
  Lock lck(m);

  return que.empty();
}

int CommandPriority2Queue::size() {
  Lock lck(m);

  return que.size();
}

void CommandPriority2Queue::brodcast() {
  Lock lck(m);

  if (!que.empty())
    cond.notify_all();
}

void CommandPriority2Queue::suspend() {
  Lock lk(m);

  if (suspended)
    return;

  suspended = true;
  cond.notify_all();
}

void CommandPriority2Queue::resume() {
  Lock lk(m);

  if (!suspended)
    return;

  suspended = false;
  cond.notify_all();
}

void CommandPriority2Queue::signal() {
  Lock lk(m);
  cond.notify_all();
}

void CommandPriority2Queue::printQueue(std::ostream &o) {
  Lock lk(m);
  for(auto c : que ) {
    c->debugInfo(o);
  }
}


}