// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <mutex>
#include "CommonLogger.h"

namespace logging {

class ConsoleLogger : public CommonLogger {
public:
  explicit ConsoleLogger(Level level = DEBUGGING);

protected:
  void doLogString(const std::string& message) override;

private:
  std::mutex mutex;
};

}
