// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2019 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "Logging/LoggerMessage.h"
namespace CryptoNote
{
  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  struct tx_verification_context
  {
    bool m_should_be_relayed;
    bool m_verification_failed; //bad tx, should drop connection
    bool m_verification_impossible; //the transaction is related with an alternative blockchain
    bool m_added_to_pool;
    bool m_tx_fee_too_small;
  };

  struct block_verification_context
  {
    bool m_added_to_main_chain;
    bool m_verification_failed; //bad block, should drop connection
    bool m_marked_as_orphaned;
    bool m_already_exists;
    bool m_switched_to_alt_chain;

    void log(Logging::LoggerMessage &logger) const
    {
      logger << "~+~ ***** block_verification_context *****" << std::endl;
      logger << "~+~ m_added_to_main_chain:\t" << m_added_to_main_chain << std::endl;
      logger << "~+~ m_verification_failed:\t" << m_verification_failed << std::endl;
      logger << "~+~ m_marked_as_orphaned:\t\t" << m_marked_as_orphaned << std::endl;
      logger << "~+~ m_already_exists:\t\t\t" << m_already_exists << std::endl;
      logger << "~+~ m_switched_to_alt_chain:\t" << m_switched_to_alt_chain << std::endl;
      logger << "~+~ *** block_verification_context end ***" << std::endl;
    }
  };
}
