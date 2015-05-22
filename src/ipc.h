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
 * ipc.h
 *
 * Handles operations related to interprocess communications.  
 *
 * initMemory sets up shared memory segment "pfailover" with 3 objects:
 *
 * "incoming" - monitored by daemon for incoming requests (see below)
 * "outgoing" - text response from daemon after sending request (client should clear this beforehand)
 * "raw" - current status of all monitors maintained by daemon (read only - see below)
 *
 * Request Format: 
 *   "get:0" Get current status and state information of all monitors
 *   "off:<monitor name>" Stop monitor
 *   "on:<monitor name>" Start monitor
 *   "pri:<monitor name>" Activate primary connection for monitor
 *   "sec:<monitor name>" Activate secondary connection for monitor
 *
 * Raw Format:
 *    "<monitor name><status><state>" (each monitor is separated by "~")
 *    Status: 0=Off, 1=On, 2=Shutting Down
 *    State: 0=Primary, 1=Secondary
 *    Note - These integers are stored as ASCII characters, not as the integer value itself
 */

#ifndef IPC_H_
#define IPC_H_
#define SHAREDNAME "pfailover"

#include <string>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "monitor.h"

namespace pfailover {

// Normal strings cannot be shared, only Boost's basic_string
typedef boost::interprocess::managed_shared_memory::segment_manager segment_manager_t;
typedef boost::interprocess::allocator<char, segment_manager_t> char_allocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>,char_allocator> shared_string;

   class IPC {
      public:
         static int initMemory();  // Create memory segment and objects
         static int sendCommand(std::string passCommand, std::string & returnResult);  // Send command to daemon
         static int process(std::vector<pfailover::Monitor *> passMonitors); // Daemon to process incoming requests
      private:
         static boost::interprocess::managed_shared_memory * sharedMem_;  // Shared memory segment
         static shared_string * incoming_;  // Incoming shared object
         static shared_string * outgoing_;  // Outgoing shared object
         static shared_string * raw_;  // Raw shared object
   };
}

#endif /* IPC_H_ */
