// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2021 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreConfig.h"

#include "Common/CommandLine.h"
#include "Common/Util.h"

namespace CryptoNote
{
  CoreConfig::CoreConfig() { configFolder = Tools::getDefaultDataDirectory(); }

  void CoreConfig::init(const boost::program_options::variables_map& options)
  {
    testnet = options[command_line::arg_testnet_on.name].as<bool>();
    if (options.count(command_line::arg_data_dir.name) != 0 &&
        !options[command_line::arg_data_dir.name].defaulted())
    {
      configFolder = command_line::get_arg(options, command_line::arg_data_dir);
      configFolderDefaulted = options[command_line::arg_data_dir.name].defaulted();
    }
    else
    {
      configFolder = Tools::getDefaultDataDirectory(testnet);
      configFolderDefaulted = true;
    }
  }

  void CoreConfig::initOptions(boost::program_options::options_description& desc) { }

}  // namespace CryptoNote
