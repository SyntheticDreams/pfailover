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
 * pfailover.cpp
 *
 * Global functions and main
 *
 * See header for complete information.
 * 
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <boost/thread.hpp>
#include "pfailover.h"
#include "conf.h"
#include "monitor.h"
#include "ipc.h"

int pfailover::reportError(std::string passError, std::string passFile, int passLine) {   
   std::cerr << "Error: " << passError;
   if (passFile != "") {
      std::cerr << " (" << passFile; 
      if (passLine != 0) std::cerr << ":" << passLine;
      std::cerr << ")";      
   }
   std::cerr << "\n";

   return 1;
}

int pfailover::reportUsage() {
   std::cout << "Usage: pfailover [-d] [-c=config directory] [-s=<get:0>|<on|off|on|pri|sec>:<monitor name>]\n\n";
   std::cout << "Options:\n";
   std::cout << "  -d                 Run as daemon\n";
   std::cout << "Command Arguments:\n";
   std::cout << "  get:0              Get current state and status information\n";
   std::cout << "  on:<monitor name>  Start monitor currently off\n";
   std::cout << "  off:<monitor name> Stop monitor currently on\n";
   std::cout << "  pri:<monitor name> Manually activate primary connection\n";
   std::cout << "  sec:<monitor name> Manually activate secondary connection\n\n";

   return 1;
}

int main(int argc,char *argv[]) {
   std::vector<pfailover::Monitor *> monitors_;
   std::stringstream argumentStream;
   std::string commandResult;
   int daemonize;

   // Initialize
   daemonize = 0;
   commandResult = "";

   // Parse arguments
   for (int lcv = 1 ; lcv < argc ; lcv++) {
      argumentStream << argv[lcv];

      // -d = run as a daemon
      if (argumentStream.str() == "-d") {
         daemonize = 1;
      }
      // -s = send message to running daemon
      else if (argumentStream.str().substr(0,3) == "-s=") {
         if (pfailover::IPC::sendCommand(argumentStream.str().substr(3), commandResult)) return 1;
         std::cout << commandResult << "\n";
         return 0;
      }
      // -c = override config directory
      else if (argumentStream.str().substr(0,3) == "-c=") {
         if (argumentStream.str().length() == 3) return pfailover::reportError("No path specified");
         pfailover::GlobalConfig::setPath(argumentStream.str().substr(3));
      }
      // Anything else is invalid
      else {
         pfailover::reportError("Invalid argument: " + argumentStream.str());
         return(pfailover::reportUsage());
      }

      // Clear stream for next iteration
      argumentStream.str("");
   }

   // Populate configuration data, exit if error
   if (pfailover::GlobalConfig::populate()) return 1;
 
   // If running as a daemon, fork and quit parent process
   if (daemonize) {
     if (fork()) return 0; 
   }

   // Create shared memory for IPC
   if (pfailover::IPC::initMemory()) return 1;

   // Create monitors for each config, start monitor running
   for (int lcv = 0 ; lcv < pfailover::GlobalConfig::getConfigCount() ; lcv++) {
      monitors_.push_back(new pfailover::Monitor());
      monitors_.back()->setConfig(pfailover::GlobalConfig::getConfig(lcv));
      monitors_.back()->start();
   }
   
   while (1) {
      if (pfailover::IPC::process(monitors_)) return 1;
      boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(250));
   }       

   return 0;
}
