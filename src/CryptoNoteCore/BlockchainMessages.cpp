// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteCore/BlockchainMessages.h"

namespace cn {

NewBlockMessage::NewBlockMessage(const crypto::Hash& hash) : blockHash(hash) {}

void NewBlockMessage::get(crypto::Hash& hash) const {
  hash = blockHash;
}

NewAlternativeBlockMessage::NewAlternativeBlockMessage(const crypto::Hash& hash) : blockHash(hash) {}

void NewAlternativeBlockMessage::get(crypto::Hash& hash) const {
  hash = blockHash;
}

ChainSwitchMessage::ChainSwitchMessage(std::vector<crypto::Hash>&& hashes) : blocksFromCommonRoot(std::move(hashes)) {}

void ChainSwitchMessage::get(std::vector<crypto::Hash>& hashes) const {
  hashes = blocksFromCommonRoot;
}

BlockchainMessage::BlockchainMessage(NewBlockMessage&& message) : type(MessageType::NEW_BLOCK_MESSAGE), message(std::move(message)) {}

BlockchainMessage::BlockchainMessage(NewAlternativeBlockMessage&& message) : type(MessageType::NEW_ALTERNATIVE_BLOCK_MESSAGE), message(std::move(message)) {}

BlockchainMessage::BlockchainMessage(ChainSwitchMessage&& message) : type(MessageType::CHAIN_SWITCH_MESSAGE), message(std::make_shared<ChainSwitchMessage>(std::move(message))) {}

BlockchainMessage::MessageType BlockchainMessage::getType() const {
  return type;
}

bool BlockchainMessage::getNewBlockHash(crypto::Hash& hash) const {
  if (type == MessageType::NEW_BLOCK_MESSAGE) {
    boost::get<NewBlockMessage>(message).get(hash);
    return true;
  } else {
    return false;
  }
}

bool BlockchainMessage::getNewAlternativeBlockHash(crypto::Hash& hash) const {
  if (type == MessageType::NEW_ALTERNATIVE_BLOCK_MESSAGE) {
    boost::get<NewAlternativeBlockMessage>(message).get(hash);
    return true;
  } else {
    return false;
  }
}

bool BlockchainMessage::getChainSwitch(std::vector<crypto::Hash>& hashes) const {
  if (type == MessageType::CHAIN_SWITCH_MESSAGE) {
    boost::get<std::shared_ptr<ChainSwitchMessage>>(message)->get(hashes);
    return true;
  } else {
    return false;
  }
}

}
