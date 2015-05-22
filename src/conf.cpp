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
 * conf.cpp
 *
 * Handles reading and processing configuration information
 *
 * See header for complete information.
 * 
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/regex.hpp>
#include <sys/stat.h>
#include "pfailover.h"
#include "conf.h"

pfailover::Config::Config() {
   // Default values
   setName("");
   setPingSrc("");
   setPingDst("");
   setPingCount(5);
   setPingInterval(20);
   setPingFail(3);
   setPingAlert(2);
}

pfailover::Config::~Config() {
}

void pfailover::Config::setName(std::string passName) {
   name_ = passName;  
}

std::string pfailover::Config::getName() {
   return name_;
}

void pfailover::Config::setPingSrc(std::string passPingSrc) {
   pingSrc_ = passPingSrc;

}

std::string pfailover::Config::getPingSrc() {
   return pingSrc_;
}

void pfailover::Config::setPingDst(std::string passPingDst) {
   pingDst_ = passPingDst;
}

std::string pfailover::Config::getPingDst() {
   return pingDst_;
}

void pfailover::Config::setPingCount(int passPingCount) {
   pingCount_ = passPingCount;
}

int pfailover::Config::getPingCount() {
   return pingCount_;
}

void pfailover::Config::setPingInterval(int passPingInterval) {
   pingInterval_ = passPingInterval;
}

int pfailover::Config::getPingInterval() {
   return pingInterval_;
}

void pfailover::Config::setPingFail(int passPingFail) {
   pingFail_ = passPingFail;
}

int pfailover::Config::getPingFail() {
   return pingFail_;
}

void pfailover::Config::setPingAlert(int passPingAlert) {
   pingAlert_ = passPingAlert;
}

int pfailover::Config::getPingAlert() {
   return pingAlert_;
}

// Declare static fields
std::vector<pfailover::Config *> pfailover::GlobalConfig::configs_;
std::string pfailover::GlobalConfig::path_ = "/usr/local/etc/pfailover";

void pfailover::GlobalConfig::setPath(std::string passPath) {
   // Drop trailing slash
   while (passPath.substr(passPath.length()-1, 1) == "/" && passPath.length() > 0) {
      passPath = passPath.erase(passPath.length()-1, 1);
   }

   path_ = passPath;   
}

std::string pfailover::GlobalConfig::getPath() {
   return path_;
}

pfailover::Config * pfailover::GlobalConfig::getConfig(int passConfigIndex) {
   return configs_.at(passConfigIndex);
}

int pfailover::GlobalConfig::getConfigCount() {
   return configs_.size();
}

int pfailover::GlobalConfig::populate() {
   std::ifstream confFile;
   std::string lineBuffer;
   std::stringstream lineStream;
   boost::regex patternConfig;
   boost::regex patternValue;
   boost::regex patternComments;
   boost::smatch regexMatch;
   struct stat scratchInfo;
   int numericValue;
   int currentLine;

   
   // Item type regular expressions
   patternConfig = "^\\s*\\[([^]]+)\\]\\s*($|[#;].*$)";
   patternValue  = "^\\s*(((src|dst)\\s*=\\s*([^\\s]*))|((count|interval|fail|alert)\\s*=\\s*([0-9]+)))\\s*($|[#;].*$)";
   patternComments = "^\\s*($|[#;].*$)"; 

   confFile.open((path_ + "/pfailover.conf").c_str(), std::ios::in);

   // Check to see if we could open the file
   if (confFile.fail()) return pfailover::reportError("Could not read configuration file", path_ + "/pfailover.conf");

   // Read each line
   currentLine = 1;
   while (std::getline(confFile, lineBuffer)) {
    
      // Check for new monitor configs
      if (boost::regex_match(lineBuffer, regexMatch, patternConfig)) {
         configs_.push_back(new pfailover::Config());
         configs_.back()->setName(regexMatch[1]);
      }
      // Check for comments
      else if (boost::regex_match(lineBuffer, patternComments)) {
      }
      // Check for values
      else if (boost::regex_match(lineBuffer, regexMatch, patternValue)) {
         // Error cases first
         if (getConfigCount() == 0) return (pfailover::reportError("No monitor defined for configuration value", path_ +"/pfailover.conf", currentLine));
         if (regexMatch.size() != 9) return (pfailover::reportError("No configuration value defined", path_ +"/pfailover.conf", currentLine));

         // Convert numeric value if necessary
         if (regexMatch[6] != "") {
            lineStream << regexMatch[7];
            lineStream >> numericValue;
            lineStream.clear();
         }
         
         // Good cases
         if (regexMatch[3] == "src") configs_.back()->setPingSrc(regexMatch[4]);
         if (regexMatch[3] == "dst") configs_.back()->setPingDst(regexMatch[4]);
         if (regexMatch[6] == "count") configs_.back()->setPingCount(numericValue);
         if (regexMatch[6] == "interval") configs_.back()->setPingInterval(numericValue);
         if (regexMatch[6] == "fail") configs_.back()->setPingFail(numericValue);
         if (regexMatch[6] == "alert") configs_.back()->setPingAlert(numericValue);

      }
      // Otherwise our input is unrecognized
      else {
         return (pfailover::reportError("Unknown configuration option or value", path_ +"/pfailover.conf", currentLine));
      }
      
      currentLine++;     
   }
   
   confFile.close();

   // Check for error conditions
   if (getConfigCount() == 0) return (pfailover::reportError("No monitors defined", path_ + "/pfailover.conf"));

   for (int lcv = 0 ; lcv < getConfigCount() ; lcv++) {
      // For now we're not using source IP/Interface - origination is done via routing.  We may use this if we manually write a ICMP
      // function in the future.
      /*if (getConfig(lcv)->getPingSrc() == "") {
         return (pfailover::reportError("No source IP specified for monitor '" + getConfig(lcv)->getName() + "'", path_ + "/pfailover.conf"));
      }*/
      if (getConfig(lcv)->getPingDst() == "") {
         return (pfailover::reportError("No ping target specified for monitor '" + getConfig(lcv)->getName() + "'", path_ + "/pfailover.conf"));
      }
      lineStream.str(path_ + "/" + getConfig(lcv)->getName() + ".primary");
      if (stat(lineStream.str().c_str(), &scratchInfo)) {
         return (pfailover::reportError("No primary script found for monitor '" + getConfig(lcv)->getName() + "'", path_ + "/pfailover.conf"));
      }
      lineStream.str(path_ + "/" + getConfig(lcv)->getName() + ".secondary");
      if (stat(lineStream.str().c_str(), &scratchInfo)) {
         return (pfailover::reportError("No secondary script found for monitor '" + getConfig(lcv)->getName() + "'", path_ + "/pfailover.conf"));
      }
   }
  
   return 0;
}
