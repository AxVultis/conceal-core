// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "LoggerGroup.h"
#include <algorithm>

namespace logging {

LoggerGroup::LoggerGroup(Level level) : CommonLogger(level) {
}

void LoggerGroup::addLogger(std::unique_ptr<ILogger> logger) {
  loggers.push_back(std::move(logger));
}

void LoggerGroup::operator()(const std::string& category, Level level, boost::posix_time::ptime time, const std::string& body) {
  if (level <= logLevel && !disabledCategories.contains(category)) {
    for (const auto& logger : loggers) {
      (*logger)(category, level, time, body);
    }
  }
}

}
