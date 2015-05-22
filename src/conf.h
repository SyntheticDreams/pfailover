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
 * conf.h
 *
 * Handles reading and processing configuration information
 *
 * GlobalConfig is responsible for reading in the actual configuration file, while
 * each Config object is responsible for each monitor's specific configuration data.
 *
 */

#ifndef CONF_H_
#define CONF_H_

#include <string>
#include <vector>

namespace pfailover {
   class Config {
      public:
         Config();
         ~Config();

         // User friendly name for this config
         void setName(std::string passName);
         std::string getName();

         // Interface's IP from which to ping
         void setPingSrc(std::string passPingSrc);
         std::string getPingSrc();

         // Server to ping
         void setPingDst(std::string passPingDst);
         std::string getPingDst();

         // Number of pings in each ping group
         void setPingCount(int passPingCount);
         int getPingCount();

         // Seconds between each ping group
         void setPingInterval(int passPingInterval);
         int getPingInterval();

         // Number of dropped pings to trigger group fail
         void setPingFail(int passPingFail);
         int getPingFail();

         // Number of failed ping groups to trigger failover alert
         void setPingAlert(int passPingAlert);
         int getPingAlert();

      private:
         std::string name_;
         std::string pingSrc_;
         std::string pingDst_;
         int pingCount_;
         int pingInterval_;
         int pingFail_;
         int pingAlert_;
   };

   class GlobalConfig {
         public:
            static void setPath(std::string passPath); // Set conf directory pathname
            static std::string getPath(); // Get conf directory pathname
            static Config * getConfig(int passConfigIndex);
            static int getConfigCount();

            static int populate(); // Populate object from directory

         private:
            static std::vector<Config *> configs_;
            static std::string path_;
      };
   }

#endif /* CONF_H_ */
