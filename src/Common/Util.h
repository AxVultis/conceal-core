// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2022 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <system_error>

namespace tools
{
  std::string getDefaultDataDirectory(bool testnet = false);
  std::string getOSVersion();
  bool createDirectoriesIfNecessary(const std::string& path);
  std::error_code replaceFile(const std::string& replacement_name,
                              const std::string& replaced_name);
  bool directoryExists(const std::string& path);
}  // namespace Tools
