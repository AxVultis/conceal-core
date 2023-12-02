// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <map>
#include <unordered_map>

#include "ITransfersContainer.h"
#include "IWallet.h"
#include "IWalletLegacy.h" //TODO: make common types for all of our APIs (such as PublicKey, KeyPair, etc)

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>

#include "Common/FileMappedVector.h"
#include "crypto/chacha8.h"

namespace cn
{

    const uint64_t ACCOUNT_CREATE_TIME_ACCURACY = 60 * 60 * 24;

    struct WalletRecord
    {
        crypto::PublicKey spendPublicKey;
        crypto::SecretKey spendSecretKey;
        cn::ITransfersContainer *container = nullptr;
        uint64_t pendingBalance = 0;
        uint64_t actualBalance = 0;
        uint64_t lockedDepositBalance = 0;
        uint64_t unlockedDepositBalance = 0;
        time_t creationTimestamp;
    };

#pragma pack(push, 1)
struct EncryptedWalletRecord {
  crypto::chacha8_iv iv;
  // Secret key, public key and creation timestamp
  uint8_t data[sizeof(crypto::PublicKey) + sizeof(crypto::SecretKey) + sizeof(uint64_t)];
};
#pragma pack(pop)

    struct RandomAccessIndex
    {
    };
    struct KeysIndex
    {
    };
    struct TransfersContainerIndex
    {
    };

    struct WalletIndex
    {
    };
    struct TransactionOutputIndex
    {
    };
    struct BlockHeightIndex
    {
    };

    struct TransactionHashIndex
    {
    };
    struct TransactionIndex
    {
    };
    struct BlockHashIndex
    {
    };

    using WalletsContainer =
        boost::multi_index_container<
            WalletRecord,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<boost::multi_index::tag<RandomAccessIndex>>,
                boost::multi_index::hashed_unique<boost::multi_index::tag<KeysIndex>,
                                                  boost::multi_index::member<WalletRecord, crypto::PublicKey, &cn::WalletRecord::spendPublicKey>>,
                boost::multi_index::hashed_unique<boost::multi_index::tag<TransfersContainerIndex>,
                                                  boost::multi_index::member<WalletRecord, cn::ITransfersContainer *, &cn::WalletRecord::container>>>>;

    struct UnlockTransactionJob
    {
        uint32_t blockHeight;
        cn::ITransfersContainer *container;
        crypto::Hash transactionHash;
    };

    using UnlockTransactionJobs =
        boost::multi_index_container<
            UnlockTransactionJob,
            boost::multi_index::indexed_by<
                boost::multi_index::ordered_non_unique<boost::multi_index::tag<BlockHeightIndex>,
                                                       boost::multi_index::member<UnlockTransactionJob, uint32_t, &cn::UnlockTransactionJob::blockHeight>>,
                boost::multi_index::hashed_non_unique<boost::multi_index::tag<TransactionHashIndex>,
                                                      boost::multi_index::member<UnlockTransactionJob, crypto::Hash, &cn::UnlockTransactionJob::transactionHash>>>>;

    using WalletDeposits =
        boost::multi_index_container<
            cn::Deposit,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<boost::multi_index::tag<RandomAccessIndex>>,
                boost::multi_index::hashed_unique<boost::multi_index::tag<TransactionIndex>,
                                                  boost::multi_index::member<cn::Deposit, crypto::Hash, &cn::Deposit::transactionHash>>,
                boost::multi_index::ordered_non_unique<boost::multi_index::tag<BlockHeightIndex>,
                                                       boost::multi_index::member<cn::Deposit, uint64_t, &cn::Deposit::height>>>>;

    using WalletTransactions =
        boost::multi_index_container<
            cn::WalletTransaction,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<boost::multi_index::tag<RandomAccessIndex>>,
                boost::multi_index::hashed_unique<boost::multi_index::tag<TransactionIndex>,
                                                  boost::multi_index::member<cn::WalletTransaction, crypto::Hash, &cn::WalletTransaction::hash>>,
                boost::multi_index::ordered_non_unique<boost::multi_index::tag<BlockHeightIndex>,
                                                       boost::multi_index::member<cn::WalletTransaction, uint32_t, &cn::WalletTransaction::blockHeight>>>>;

    using ContainerStorage = common::FileMappedVector<EncryptedWalletRecord>;
    using TransactionTransferPair = std::pair<size_t, cn::WalletTransfer>;
    using WalletTransfers = std::vector<TransactionTransferPair>;

    using UncommitedTransactions = std::map<size_t, cn::Transaction>;

    using BlockHashesContainer =
        boost::multi_index_container<
            crypto::Hash,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<
                    boost::multi_index::tag<BlockHeightIndex>>,
                boost::multi_index::hashed_unique<
                    boost::multi_index::tag<BlockHashIndex>,
                    boost::multi_index::identity<crypto::Hash>>>>;

    using WalletPaymentIds = std::unordered_map<crypto::Hash, std::vector<size_t>, boost::hash<crypto::Hash>>;

} // namespace cn
