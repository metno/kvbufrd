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

#include <iosfwd>
#include <string>
#include <deque>
#include <ostream>

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
class QueSuspended {
};

/**
 * \brief A base class to be used for all messages to be posted
 * on a CommandQue.
 */
class CommandBase {
 protected:
  std::string comment;

 public:

  CommandBase() {
  }
  CommandBase(const std::string &comment_)
      : comment(comment_) {
  }
  virtual ~CommandBase() {
  }

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
  virtual bool executeImpl()=0;
  virtual void debugInfo(std::ostream &info) const;

  const std::string& getComment() const {
    return comment;
  }

  void setComment(const std::string &comment_) {
    comment = comment_;
  }

  friend std::ostream& operator<<(std::ostream& os, const CommandBase &c) {
    c.debugInfo(os);
    return os;
  }

};
}
#endif
