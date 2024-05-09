// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include "CommonLogger.h"

namespace logging {

class LoggerGroup : public CommonLogger {
public:
  explicit LoggerGroup(Level level = DEBUGGING);
  LoggerGroup(const LoggerGroup &logger) : CommonLogger(logger.logLevel) {};

  void addLogger(std::unique_ptr<ILogger> logger);
  void operator()(const std::string& category, Level level, boost::posix_time::ptime time, const std::string& body) override;

protected:
  std::vector<std::unique_ptr<ILogger>> loggers;
};

}
