// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <time.h>

namespace cn {

  struct ITimeProvider {
    virtual time_t now() = 0;
    virtual ~ITimeProvider() = default;
  };

  struct RealTimeProvider : public ITimeProvider {
    time_t now() override {
      return time(nullptr);
    }
  };

}
