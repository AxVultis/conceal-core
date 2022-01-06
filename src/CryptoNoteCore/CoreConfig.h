// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2021 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <boost/program_options.hpp>
#include <string>

namespace cn
{
  class CoreConfig
  {
   private:
      public:
    CoreConfig();

    static void initOptions(boost::program_options::options_description& desc);
    void init(const boost::program_options::variables_map& options);
    bool testnet = false;

    std::string configFolder;
    bool configFolderDefaulted = true;
  };

}  // namespace cn
