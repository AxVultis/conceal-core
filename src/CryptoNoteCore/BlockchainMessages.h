// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <memory>
#include <vector>

#include <CryptoNote.h>

namespace cn {

class NewBlockMessage {
public:
  explicit NewBlockMessage(const crypto::Hash& hash);
  NewBlockMessage() = default;
  void get(crypto::Hash& hash) const;
private:
  crypto::Hash blockHash;
};

class NewAlternativeBlockMessage {
public:
  explicit NewAlternativeBlockMessage(const crypto::Hash& hash);
  NewAlternativeBlockMessage() = default;
  void get(crypto::Hash& hash) const;
private:
  crypto::Hash blockHash;
};

class ChainSwitchMessage {
public:
  explicit ChainSwitchMessage(std::vector<crypto::Hash>&& hashes);
  void get(std::vector<crypto::Hash>& hashes) const;
private:
  std::vector<crypto::Hash> blocksFromCommonRoot;
};

class BlockchainMessage {
public:
  enum class MessageType {
    NEW_BLOCK_MESSAGE,
    NEW_ALTERNATIVE_BLOCK_MESSAGE,
    CHAIN_SWITCH_MESSAGE
  };

  explicit BlockchainMessage(NewBlockMessage&& message);
  explicit BlockchainMessage(NewAlternativeBlockMessage&& message);
  explicit BlockchainMessage(ChainSwitchMessage&& message);

  MessageType getType() const;

  bool getNewBlockHash(crypto::Hash& hash) const;
  bool getNewAlternativeBlockHash(crypto::Hash& hash) const;
  bool getChainSwitch(std::vector<crypto::Hash>& hashes) const;
private:
  const MessageType type;

  using Message = boost::variant<
      NewBlockMessage,
      NewAlternativeBlockMessage,
      std::shared_ptr<ChainSwitchMessage>>;
  Message message;
};

}
