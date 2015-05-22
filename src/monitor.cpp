// $Id$

/*
 * Copyright (C) 2009 Toni Westbrook <twestbrook@synthdreams.com>
 *
 * This file is part of pfailover.
 *
 * pfailover is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pfailover is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pfailover.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * monitor.cpp
 *
 * Handles monitor processing
 *
 * See header for complete information.
 * 
 */

#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include "pfailover.h"
#include "monitor.h"

pfailover::Monitor::Monitor() {
   state_ = 0;
   status_ = 0;
}

pfailover::Monitor::~Monitor() {

}

void pfailover::Monitor::setConfig(pfailover::Config * passConfig) {
   config_ = passConfig;
}

pfailover::Config * pfailover::Monitor::getConfig() {
   return config_;
}

int pfailover::Monitor::getState() {
   return state_;
}

int pfailover::Monitor::getStatus() {
   return status_;
}

void pfailover::Monitor::start() {
   thread_ = new boost::thread(boost::bind(&pfailover::Monitor::run,this));
   status_ = 1;
}

void pfailover::Monitor::stop() {
   status_ = 2;
}

void pfailover::Monitor::activate(int passState) {
   std::stringstream scriptStream;

   // Build script string
   scriptStream << pfailover::GlobalConfig::getPath() << "/" << getConfig()->getName();

   // Run appropropriate script
   if (passState == 0) {
      scriptStream << ".primary";
   }
   else {
      scriptStream << ".secondary";
   }
   
   // Send ping destination as argument to the script
   scriptStream << " " << getConfig()->getPingDst(); 
   scriptStream << " &> /dev/null";

   system(scriptStream.str().c_str());
   state_ = passState;
}

void pfailover::Monitor::run() {
   std::stringstream execStream;
   int numPings;
   int numFail, numAlert;

   // Initialize data
   execStream << "ping -c 1";
   execStream << " " << getConfig()->getPingDst();
   execStream << " &> /dev/null";
   numAlert = 0;

   // Outer loop - check for group failures
   while (1) {
      // Inner loop - pings every second, checks for exit condition
      numPings = 0;
      numFail = 0;
      while (numPings < getConfig()->getPingCount()) {
         if (status_ == 2) {
            status_ = 0;
            return;
         }
         numFail += (system(execStream.str().c_str()) > 0);     
         numPings++;

         // Sleep for 1 second between pings
         boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(1));      
      }

      // See if we passed ping failure threshold to trigger a group failure
      if (numFail >= getConfig()->getPingFail()) {
         numAlert++;
         if (numAlert > getConfig()->getPingAlert()) numAlert = getConfig()->getPingAlert();
      }
      else {
         numAlert--;
         if (numAlert < 0) numAlert = 0;
      }

      // Check to see if we need to switch modes
      if ((numAlert == getConfig()->getPingAlert()) && state_ == 0) activate(1);
      if ((numAlert == 0) && state_ == 1) activate(0);

      // Sleep until next ping group test
      boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(getConfig()->getPingInterval()));  

   }

}
