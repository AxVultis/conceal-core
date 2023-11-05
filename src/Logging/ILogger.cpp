// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ILogger.h"

namespace logging {

const std::string BLUE = "\037BLUE\037";
const std::string GREEN = "\037GREEN\037";
const std::string RED = "\037RED\037";
const std::string YELLOW = "\037YELLOW\037";
const std::string WHITE = "\037WHITE\037";
const std::string CYAN = "\037CYAN\037";
const std::string MAGENTA = "\037MAGENTA\037";
const std::string BRIGHT_BLUE = "\037BRIGHT_BLUE\037";
const std::string BRIGHT_GREEN = "\037BRIGHT_GREEN\037";
const std::string BRIGHT_RED = "\037BRIGHT_RED\037";
const std::string BRIGHT_YELLOW = "\037BRIGHT_YELLOW\037";
const std::string BRIGHT_WHITE = "\037BRIGHT_WHITE\037";
const std::string BRIGHT_CYAN = "\037BRIGHT_CYAN\037";
const std::string BRIGHT_MAGENTA = "\037BRIGHT_MAGENTA\037";
const std::string DEFAULT = "\037DEFAULT\037";

const char ILogger::COLOR_DELIMETER = '\037';

const std::array<std::string, 6> ILogger::LEVEL_NAMES = {
  {"FATAL",
  "ERROR",
  "WARNING",
  "INFO",
  "DEBUG",
  "TRACE"}
};

}
