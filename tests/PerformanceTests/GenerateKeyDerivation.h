// Copyright (c) 2012-2017 The Cryptonote developers
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"
#include "CryptoNoteCore/CryptoNoteBasic.h"

#include "SingleTransactionTestBase.h"

class test_generate_key_derivation : public single_tx_test_base
{
public:
  static const size_t loop_count = 1000;

  bool test()
  {
    crypto::KeyDerivation recv_derivation;
    crypto::generate_key_derivation(m_tx_pub_key, m_bob.getAccountKeys().viewSecretKey, recv_derivation);
    return true;
  }
};
