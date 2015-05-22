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
 * ipc.cpp
 *
 * Handles operations related to interprocess communications.  
 *
 * See header for complete information.
 * 
 */

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include "pfailover.h"
#include "ipc.h"

// Declare static fields
boost::interprocess::managed_shared_memory * pfailover::IPC::sharedMem_;
pfailover::shared_string * pfailover::IPC::incoming_;
pfailover::shared_string * pfailover::IPC::outgoing_;
pfailover::shared_string * pfailover::IPC::raw_;

int pfailover::IPC::initMemory() {
   // Remove old shared memory if exists
   boost::interprocess::shared_memory_object::remove(SHAREDNAME);

   // Create shared memory segment
   try {
      sharedMem_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, SHAREDNAME, 65535*2);
    
      // Assign the shared objects
      incoming_ = sharedMem_->construct<shared_string>("incoming")("",sharedMem_->get_segment_manager());
      outgoing_ = sharedMem_->construct<shared_string>("outgoing")("",sharedMem_->get_segment_manager());
      raw_ = sharedMem_->construct<shared_string>("raw")("",sharedMem_->get_segment_manager());
   }
   catch(...) {
      return pfailover::reportError("Unabled to created shared memory segment");
   }

   return 0;
}

int pfailover::IPC::sendCommand(std::string passCommand, std::string & returnResult) {
   std::stringstream returnStream;   
   boost::regex patternCommand;
   boost::smatch regexMatch;
   std::pair <shared_string *, std::size_t> tempString;
   int waitTime;

   // Parse the command
   patternCommand = "^(get|on|off|pri|sec):(.*)$";
   if (!boost::regex_match(passCommand, regexMatch, patternCommand)) {
      pfailover::reportError("Invalid command");
      return pfailover::reportUsage();
   }
   
   // Open shared memory
   try {
      boost::interprocess::managed_shared_memory sharedMem(boost::interprocess::open_only, SHAREDNAME);

      // Clear output buffer in case we have old messages in it
      tempString = sharedMem.find<shared_string> ("outgoing");
      tempString.first->clear();

      // Copy command
      tempString = sharedMem.find<shared_string> ("incoming");
      *tempString.first = passCommand.c_str();      
    
      // Wait for response
      tempString = sharedMem.find<shared_string> ("outgoing");
      waitTime = 0;
      while (*tempString.first == "") {
         boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(250));  
         // If we time out waiting, the daemon is most likely not running
         if (waitTime++ > 4*4) return pfailover::reportError("Unable to contact pfailover daemon, is it running?");
      }

      // Return value
      returnStream << *tempString.first;
      returnResult = returnStream.str();
   }
   catch(...) {
      return pfailover::reportError("Unable to contact pfailover daemon, is it running?");
   }

   return 0;
}

int pfailover::IPC::process(std::vector<pfailover::Monitor *> passMonitors) {
   std::stringstream responseStream;
   std::stringstream rawStream;
   std::string commandString;
   boost::regex commandPattern;
   boost::smatch regexMatch;
   int lcv;

   // Always populate the raw buffer with current information
   for (lcv = 0 ; lcv < pfailover::GlobalConfig::getConfigCount() ; lcv++) {
      // Insert separator for next monitor
      if (lcv > 0) rawStream << "~";

      rawStream << passMonitors.at(lcv)->getConfig()->getName();
      rawStream << passMonitors.at(lcv)->getStatus();
      rawStream << passMonitors.at(lcv)->getState();
   }
   *raw_ = rawStream.str().c_str();

   // Process incoming commands
   commandString = incoming_->c_str();
   
   // Quit if invalid
   commandPattern = "^(get|on|off|pri|sec):(.*)$";
   if (!boost::regex_match(commandString, regexMatch, commandPattern)) {      
      return 0;
   }

   // Request for current status and state information
   if (regexMatch[1] == "get") {
      responseStream << "\nMonitor Information:\n\n";
      for (lcv = 0 ; lcv < pfailover::GlobalConfig::getConfigCount() ; lcv++) {
         responseStream << "  " + passMonitors.at(lcv)->getConfig()->getName() + " [";
         responseStream << "Status=";
         if (passMonitors.at(lcv)->getStatus() == 0) responseStream << "Off";
         if (passMonitors.at(lcv)->getStatus() == 1) responseStream << "On";
         if (passMonitors.at(lcv)->getStatus() == 2) responseStream << "Shutting Down";         
         responseStream << ",State=";
         if (passMonitors.at(lcv)->getState() == 0) responseStream << "Primary";
         if (passMonitors.at(lcv)->getState() == 1) responseStream << "Secondary";
         responseStream << "]\n";
      }
   }
   else {
      // First make sure we have a valid monitor name
      for (lcv = 0 ; lcv < pfailover::GlobalConfig::getConfigCount() ; lcv++) {
         // Found it         
         if (passMonitors.at(lcv)->getConfig()->getName() == regexMatch[2]) {
               if (regexMatch[1] == "on") {
                  if (passMonitors.at(lcv)->getStatus() == 1) {
                     responseStream << "\nMonitor already on!\n";
                  }
                  else {
                     passMonitors.at(lcv)->start();
                     responseStream << "\nMonitor started!\n";
                  }                     
               }
               if (regexMatch[1] == "off") {
                  if (passMonitors.at(lcv)->getStatus() != 1) {
                     responseStream << "\nMonitor already off!\n";
                  }
                  else {
                     passMonitors.at(lcv)->stop();
                     responseStream << "\nMonitor stopped!\n";
                  }                     
               }
               if (regexMatch[1] == "pri") {
                  if (passMonitors.at(lcv)->getState() == 0) {
                     responseStream << "\nPrimary connection already active!\n";
                  }
                  else {
                     passMonitors.at(lcv)->activate(0);
                     responseStream << "\nPrimary connection now active!\n";
                  }                     
               }
               if (regexMatch[1] == "sec") {
                  if (passMonitors.at(lcv)->getState() == 1) {
                     responseStream << "\nSecondary connection already active!\n";
                  }
                  else {
                     passMonitors.at(lcv)->activate(1);
                     responseStream << "\nSecondary connection now active!\n";
                  }                     
               }

            break;
         } 
      }

      // Didn't find it
      if (lcv == pfailover::GlobalConfig::getConfigCount()) {
         responseStream << "\nCouldn't find monitor \"";
         responseStream << regexMatch[2];
         responseStream << "\"\n";
      }
   }
   
   // Send response
   *outgoing_ = responseStream.str().c_str();

   // Remove request from incoming buffer
   incoming_->clear();

   return 0;
}

