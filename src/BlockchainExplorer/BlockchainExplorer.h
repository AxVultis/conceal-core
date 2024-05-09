// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <mutex>
#include <atomic>
#include <unordered_set>

#include "IBlockchainExplorer.h"
#include "INode.h"

#include "Common/ObserverManager.h"
#include "BlockchainExplorerErrors.h"

#include "Wallet/WalletAsyncContextCounter.h"

#include "Logging/LoggerRef.h"

namespace cn {

class BlockchainExplorer : public IBlockchainExplorer, public INodeObserver {
public:
  BlockchainExplorer(INode& node, logging::ILogger& logger);

  BlockchainExplorer(const BlockchainExplorer&) = delete;
  BlockchainExplorer(BlockchainExplorer&&) = delete;

  BlockchainExplorer& operator=(const BlockchainExplorer&) = delete;
  BlockchainExplorer& operator=(BlockchainExplorer&&) = delete;

  ~BlockchainExplorer() override = default;
    
  bool addObserver(IBlockchainObserver* observer) override;
  bool removeObserver(IBlockchainObserver* observer) override;

  bool getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<BlockDetails>>& blocks) override;
  bool getBlocks(const std::vector<crypto::Hash>& blockHashes, std::vector<BlockDetails>& blocks) override;
  bool getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps) override;

  bool getBlockchainTop(BlockDetails& topBlock) override;

  bool getTransactions(const std::vector<crypto::Hash>& transactionHashes, std::vector<TransactionDetails>& transactions) override;
  bool getTransactionsByPaymentId(const crypto::Hash& paymentId, std::vector<TransactionDetails>& transactions) override;
  bool getPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<TransactionDetails>& transactions, uint64_t& transactionsNumberWithinTimestamps) override;
  bool getPoolState(const std::vector<crypto::Hash>& knownPoolTransactionHashes, crypto::Hash knownBlockchainTop, bool& isBlockchainActual, std::vector<TransactionDetails>& newTransactions, std::vector<crypto::Hash>& removedTransactions) override;

  uint64_t getRewardBlocksWindow() override;
  uint64_t getFullRewardMaxBlockSize(uint8_t majorVersion) override;

  bool isSynchronized() override;

  void init() override;
  void shutdown() override;

  void poolChanged() override;
  void blockchainSynchronized(uint32_t topHeight) override;
  void localBlockchainUpdated(uint32_t height) override;

  using AsyncContextCounter = WalletAsyncContextCounter;

private:
  void poolUpdateEndHandler();

  class PoolUpdateGuard {
  public:
    PoolUpdateGuard();

    bool beginUpdate();
    bool endUpdate();

  private:
    enum class State {
      NONE,
      UPDATING,
      UPDATE_REQUIRED
    };

    std::atomic<State> m_state;
  };

  enum State {
    NOT_INITIALIZED,
    INITIALIZED
  };

  BlockDetails knownBlockchainTop;
  uint32_t knownBlockchainTopHeight;
  std::unordered_set<crypto::Hash> knownPoolState;

  std::atomic<State> state;
  std::atomic<bool> synchronized;
  std::atomic<uint32_t> observersCounter;
  tools::ObserverManager<IBlockchainObserver> observerManager;

  std::mutex mutex;

  INode& node;
  logging::LoggerRef logger;

  AsyncContextCounter asyncContextCounter;
  PoolUpdateGuard poolUpdateGuard;
};
}
