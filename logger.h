/*
 * Driver type: GPhoto Camera INDI Driver
 *
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef INDI_GPHOTO_CCD_NG_LOGGER_H
#define INDI_GPHOTO_CCD_NG_LOGGER_H

#include <defaultdevice.h>
#include <indilogger.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iostream>
namespace INDI {
namespace GPhoto {
class Logger {
public:
  Logger(DefaultDevice *device, const std::string &module = {}) : device{device}, module{module} {}
  class Log {
  public:
    Log() = delete;
    Log(const Log &) = delete;
    template<typename T> Log &operator<<(const T &t) { message << t; return *this; }
    Log(Log &&other) : device_name{std::move(other.device_name)}, level{std::move(other.level)}, message{std::move(other.message)} { other.to_log = false; }
    ~Log() {
      if(to_log)
	DEBUGDEVICE(device_name.c_str(), level, message.str().c_str());
    }
  private:
    friend class Logger;
    Log(const std::string &device_name, INDI::Logger::VerbosityLevel level, const std::string &prefix) 
      : device_name{device_name}, level{level} { message << prefix; }
    std::string device_name;
    INDI::Logger::VerbosityLevel level;
    std::stringstream message;
    bool to_log = true;
  };
  
  Log debug() const { return   mkLogger(INDI::Logger::DBG_DEBUG); }
  Log warning() const { return mkLogger(INDI::Logger::DBG_WARNING); }
  Log error() const { return   mkLogger(INDI::Logger::DBG_ERROR); }
  Log session() const { return mkLogger(INDI::Logger::DBG_SESSION); }
  Log extra1() const { return  mkLogger(INDI::Logger::DBG_EXTRA_1); }
  Log extra2() const { return  mkLogger(INDI::Logger::DBG_EXTRA_2); }
  Log extra3() const { return  mkLogger(INDI::Logger::DBG_EXTRA_3); }
  Log extra4() const { return  mkLogger(INDI::Logger::DBG_EXTRA_4); }
private:
  Log mkLogger(const INDI::Logger::VerbosityLevel level) const {  return Log{device->getDeviceName(), INDI::Logger::DBG_DEBUG, module.empty() ? "" : module + " - "}; }
  DefaultDevice *device;
  std::string module;
};
}
}

#endif