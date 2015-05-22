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
 * monitor.h
 *
 * Handles monitor processing
 *
 * Each monitor processing function ("run" method) runs in its own thread, started by the
 * the "start" method, and terminated by the "stop" method.  "run" makes a call to the ping
 * program, and changes state info/executes the appropriate scripts when triggered.
 *
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include <string>
#include <vector>
#include <boost/thread.hpp>
#include "conf.h"

namespace pfailover {
   class Monitor {
      public:
         Monitor();
         ~Monitor();
         
         void setConfig(Config * passConfig);
         Config * getConfig();
         int getState();  // 0 = Primary active, 1 = Secondary active
         int getStatus(); // 0 = Off, 1 = On, 2 = Going Off
         void start(); // Starts monitor
         void stop();  // Stops monitor
         void activate(int passState); // Activate a new state and run scripts

      private:
         void run();  // Internal function that does actual pinging
         boost::thread * thread_; // Thread for this monitor (run function)
         Config * config_; // Points to monitor's config file
         int status_;
         int state_;
   };
}

#endif /* MONITOR_H_ */
