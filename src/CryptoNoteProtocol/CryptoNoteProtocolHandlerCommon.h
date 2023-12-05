// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

namespace cn
{
  struct NOTIFY_NEW_BLOCK_request;
  struct NOTIFY_NEW_TRANSACTIONS_request;

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  struct ICryptonoteProtocol {
    virtual ~ICryptonoteProtocol() = default;
    virtual void relay_block(NOTIFY_NEW_BLOCK_request& arg) = 0;
    virtual void relay_transactions(NOTIFY_NEW_TRANSACTIONS_request& arg) = 0;
  };

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  struct CryptonoteProtocolStub: public ICryptonoteProtocol {
    ~CryptonoteProtocolStub() override = default;
    void relay_block(NOTIFY_NEW_BLOCK_request& arg) override {}
    void relay_transactions(NOTIFY_NEW_TRANSACTIONS_request& arg) override {}
  };
}
