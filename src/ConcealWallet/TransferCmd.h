// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IWallet.h"
#include "CryptoNoteCore/Currency.h"
#include "Common/StringTools.h"

#include <Logging/LoggerRef.h>

using namespace logging;

namespace cn
{
  class TransferCmd
  {
    public:
      const cn::Currency& m_currency;
      size_t fake_outs_count = 0;
      std::vector<WalletOrder> dsts;
      std::vector<uint8_t> extra;
      uint64_t fee;
      std::map<std::string, std::vector<WalletOrder>> aliases;
      std::vector<std::string> messages;
      uint64_t ttl = 0;
      std::string m_remote_address;

      TransferCmd(const cn::Currency& currency, const std::string& remote_fee_address);

      bool parseTx(const LoggerRef& logger, const std::vector<std::string> &args);
  };

  template <typename IterT, typename ValueT = typename IterT::value_type>
  class ArgumentReader {
  public:

    ArgumentReader(IterT begin, IterT end) :
      m_begin(begin), m_end(end), m_cur(begin) {
    }

    bool eof() const {
      return m_cur == m_end;
    }

    ValueT next() {
      if (eof()) {
        throw std::runtime_error("unexpected end of arguments");
      }

      return *m_cur++;
    }

  private:

    IterT m_cur;
    IterT m_begin;
    IterT m_end;
  };
} 