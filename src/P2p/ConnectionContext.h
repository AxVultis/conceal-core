// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2019 The TurtleCoin developers
// Copyright (c) 2016-2020 The Karbo developers
// Copyright (c) 2018-2021 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <list>
#include <ostream>
#include <unordered_set>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include "Common/StringTools.h"
#include "P2p/PendingLiteBlock.h"
#include "crypto/hash.h"
#include "Logging/LoggerMessage.h"

namespace CryptoNote {

struct CryptoNoteConnectionContext {
  uint8_t version;
  boost::uuids::uuid m_connection_id;
  uint32_t m_remote_ip = 0;
  uint32_t m_remote_port = 0;
  bool m_is_income = false;
  time_t m_started = 0;

  enum state {
    state_befor_handshake = 0, //default state
    state_synchronizing,
    state_idle,
    state_normal,
    state_sync_required,
    state_pool_sync_required,
    state_shutdown
  };

  state m_state = state_befor_handshake;
  boost::optional<PendingLiteBlock> m_pending_lite_block;
  std::list<Crypto::Hash> m_needed_objects;
  std::unordered_set<Crypto::Hash> m_requested_objects;
  uint32_t m_remote_blockchain_height = 0;
  uint32_t m_last_response_height = 0;

  void log(Logging::LoggerMessage &logger){
    logger << "~+~ version:\t'" << unsigned(version) << "'" << std::endl;
    logger << "~+~ m_remote_ip:\t'" << Common::ipAddressToString(m_remote_ip) << "'" << std::endl;
    logger << "~+~ m_remote_port:\t'" << m_remote_port << "'" << std::endl;
    logger << "~+~ m_is_income:\t'" << m_is_income << "'" << std::endl;
    logger << "~+~ m_started:\t'" << m_started << "'" << std::endl;
    logger << "~+~ m_state:\t'" << m_state << "'" << std::endl;
    logger << "~+~ m_needed_objects size:\t'" << m_needed_objects.size() << "'" << std::endl;
    logger << "~+~ m_requested_objects size:\t'" << m_requested_objects.size() << "'" << std::endl;
    logger << "~+~ m_remote_blockchain_height:\t'" << m_remote_blockchain_height << "'" << std::endl;
    logger << "~+~ m_last_response_height:\t'" << m_last_response_height << "'" << std::endl;
  }
};

inline std::string get_protocol_state_string(CryptoNoteConnectionContext::state s) {
  switch (s)  {
  case CryptoNoteConnectionContext::state_befor_handshake:
    return "state_befor_handshake";
  case CryptoNoteConnectionContext::state_synchronizing:
    return "state_synchronizing";
  case CryptoNoteConnectionContext::state_idle:
    return "state_idle";
  case CryptoNoteConnectionContext::state_normal:
    return "state_normal";
  case CryptoNoteConnectionContext::state_sync_required:
    return "state_sync_required";
  case CryptoNoteConnectionContext::state_pool_sync_required:
    return "state_pool_sync_required";
  case CryptoNoteConnectionContext::state_shutdown:
    return "state_shutdown";
  default:
    return "unknown";
  }
}

}

namespace std {
inline std::ostream& operator << (std::ostream& s, const CryptoNote::CryptoNoteConnectionContext& context) {
  return s << "[" << Common::ipAddressToString(context.m_remote_ip) << ":" << 
    context.m_remote_port << (context.m_is_income ? " INC" : " OUT") << "] ";
}
}
