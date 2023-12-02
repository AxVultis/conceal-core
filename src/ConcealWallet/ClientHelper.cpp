// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <future>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>

#include "ClientHelper.h"

#include "CryptoNoteConfig.h"

#include "Common/JsonValue.h"
#include "Common/StringTools.h"
#include "CryptoNoteCore/TransactionExtra.h"
#include "Wallet/LegacyKeysImporter.h"
#include "WalletLegacy/WalletHelper.h"

#include <Logging/LoggerRef.h>

using namespace common;

namespace cn
{
  uint32_t ClientHelper::deposit_term(const cn::Deposit &deposit) const
  {
    return deposit.term;
  }

  std::string ClientHelper::deposit_amount(const cn::Deposit &deposit, const Currency &currency) const
  {
    return currency.formatAmount(deposit.amount);
  }

  std::string ClientHelper::deposit_interest(const cn::Deposit &deposit, const Currency &currency) const
  {
    return currency.formatAmount(deposit.interest);
  }

  std::string ClientHelper::deposit_status(const cn::Deposit &deposit) const
  {
    std::string status_str = "";

    if (deposit.locked)
      status_str = "Locked";
    else if (deposit.spendingTransactionId == cn::WALLET_INVALID_TRANSACTION_ID)
      status_str = "Unlocked";
    else
      status_str = "Withdrawn";

    return status_str;
  }

  size_t ClientHelper::deposit_creating_tx_id(const cn::Deposit &deposit) const
  {
    return deposit.creatingTransactionId;
  }

  size_t ClientHelper::deposit_spending_tx_id(const cn::Deposit &deposit) const
  {
    return deposit.spendingTransactionId;
  }

  std::string ClientHelper::deposit_unlock_height(const cn::Deposit &deposit, const uint32_t &blockHeight) const
  {
    std::string unlock_str = "";

    if (blockHeight > cn::parameters::CRYPTONOTE_MAX_BLOCK_NUMBER)
    {
      unlock_str = "Please wait.";
    }
    else
    {
      unlock_str = std::to_string(blockHeight + deposit_term(deposit));
    }

    bool bad_unlock2 = unlock_str == "0";
    if (bad_unlock2)
    {
      unlock_str = "ERROR";
    }

    return unlock_str;
  }

  std::string ClientHelper::deposit_height(const uint32_t &blockHeight) const
  {
    std::string height_str = "";

    bool bad_unlock = blockHeight > cn::parameters::CRYPTONOTE_MAX_BLOCK_NUMBER;
    if (bad_unlock)
    {
      height_str = "Please wait.";
    }
    else
    {
      height_str = std::to_string(blockHeight);
    }

    bool bad_unlock2 = height_str == "0";
    if (bad_unlock2)
    {
      height_str = "ERROR";
    }

    return height_str;
  }

  std::string ClientHelper::get_deposit_info(const cn::Deposit &deposit, cn::DepositId did, const Currency &currency, const uint32_t &blockHeight) const
  {
    std::stringstream full_info;

    full_info << std::left <<
      std::setw(8)  << makeCenteredString(8, std::to_string(did)) << " | " <<
      std::setw(20) << makeCenteredString(20, deposit_amount(deposit, currency)) << " | " <<
      std::setw(20) << makeCenteredString(20, deposit_interest(deposit, currency)) << " | " <<
      std::setw(16) << makeCenteredString(16, deposit_unlock_height(deposit, blockHeight)) << " | " <<
      std::setw(12) << makeCenteredString(12, deposit_status(deposit));
    
    std::string as_str = full_info.str();

    return as_str;
  }

  std::string ClientHelper::get_full_deposit_info(const cn::Deposit &deposit, cn::DepositId did, const Currency &currency, const uint32_t &blockHeight) const
  {
    std::stringstream full_info;

    full_info << "ID:            " << std::to_string(did) << "\n"
              << "Amount:        " << deposit_amount(deposit, currency) << "\n"
              << "Interest:      " << deposit_interest(deposit, currency) << "\n"
              << "Height:        " << deposit_height(blockHeight) << "\n"
              << "Unlock Height: " << deposit_unlock_height(deposit, blockHeight) << "\n"
              << "Status:        " << deposit_status(deposit) << "\n";

    std::string as_str = full_info.str();

    return as_str;
  }

  std::string ClientHelper::list_deposit_item(const WalletTransaction& txInfo, const Deposit& deposit, std::string listed_deposit, DepositId id, const Currency &currency) const
  {
    std::string format_amount = currency.formatAmount(deposit.amount);
    std::string format_interest = currency.formatAmount(deposit.interest);
    std::string format_total = currency.formatAmount(deposit.amount + deposit.interest);

    std::stringstream ss_id(makeCenteredString(8, std::to_string(id)));
    std::stringstream ss_amount(makeCenteredString(20, format_amount));
    std::stringstream ss_interest(makeCenteredString(20, format_interest));
    std::stringstream ss_height(makeCenteredString(16, deposit_height(txInfo.blockHeight)));
    std::stringstream ss_unlockheight(makeCenteredString(16, deposit_unlock_height(deposit, txInfo.blockHeight)));
    std::stringstream ss_status(makeCenteredString(12, deposit_status(deposit)));

    ss_id >> std::setw(8);
    ss_amount >> std::setw(20);
    ss_interest >> std::setw(20);
    ss_height >> std::setw(16);
    ss_unlockheight >> std::setw(16);
    ss_status >> std::setw(12);

    listed_deposit = ss_id.str() + " | " + ss_amount.str() + " | " + ss_interest.str() + " | " + ss_height.str() + " | "
      + ss_unlockheight.str() + " | " + ss_status.str() + "\n";

    return listed_deposit;
  }

  std::string ClientHelper::list_tx_item(const WalletTransaction& txInfo, std::string listed_tx, const Currency &currency) const
  {
    std::vector<uint8_t> extraVec = asBinaryArray(txInfo.extra);

    crypto::Hash paymentId;
    std::string paymentIdStr = (cn::getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId != NULL_HASH ? podToHex(paymentId) : "");

    std::string timeString = formatTimestamp(static_cast<time_t>(txInfo.timestamp));

    std::string format_amount = currency.formatAmount(txInfo.totalAmount);

    std::stringstream ss_time(makeCenteredString(32, timeString));
    std::stringstream ss_hash(makeCenteredString(64, podToHex(txInfo.hash)));
    std::stringstream ss_amount(makeCenteredString(20, currency.formatAmount(txInfo.totalAmount)));
    std::stringstream ss_fee(makeCenteredString(14, currency.formatAmount(txInfo.fee)));
    std::stringstream ss_blockheight(makeCenteredString(8, std::to_string(txInfo.blockHeight)));
    std::stringstream ss_unlocktime(makeCenteredString(12, std::to_string(txInfo.unlockTime)));

    ss_time >> std::setw(32);
    ss_hash >> std::setw(64);
    ss_amount >> std::setw(20);
    ss_fee >> std::setw(14);
    ss_blockheight >> std::setw(8);
    ss_unlocktime >> std::setw(12);

    listed_tx = ss_time.str() + " | " + ss_hash.str() + " | " + ss_amount.str() + " | " + ss_fee.str() + " | "
      + ss_blockheight.str() + " | " + ss_unlocktime.str() + "\n";

    return listed_tx;
  }

  bool ClientHelper::confirm_deposit(uint32_t term, uint64_t amount, bool is_testnet, const Currency &currency, const logging::LoggerRef &logger) const
  {
    uint64_t interest = currency.calculateInterestV3(amount, term);
    uint64_t min_term = is_testnet ? parameters::TESTNET_DEPOSIT_MIN_TERM_V3 : parameters::DEPOSIT_MIN_TERM_V3;

    logger(logging::INFO) << "Confirm deposit details:\n"
      << "\tAmount: " << currency.formatAmount(amount) << "\n"
      << "\tMonths: " << term / min_term << "\n"
      << "\tInterest: " << currency.formatAmount(interest) << "\n";

    logger(logging::INFO) << "Is this correct? (Y/N): \n";

    char c;
    std::cin >> c;
    c = std::tolower(c);

    if (c == 'y')
    {
      return true;
    }
    else if (c == 'n')
    {
      return false;
    }
    else
    {
      logger(logging::ERROR) << "Bad input, please enter either Y or N.";
    }

    return false;
  }

  JsonValue ClientHelper::buildLoggerConfiguration(logging::Level level, const std::string& logfile) const
  {
    using common::JsonValue;

    JsonValue loggerConfiguration(JsonValue::OBJECT);
    loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

    JsonValue& cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

    JsonValue& consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
    consoleLogger.insert("type", "console");
    consoleLogger.insert("level", static_cast<int64_t>(logging::TRACE));
    consoleLogger.insert("pattern", "");

    JsonValue& fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
    fileLogger.insert("type", "file");
    fileLogger.insert("filename", logfile);
    fileLogger.insert("level", static_cast<int64_t>(logging::TRACE));

    return loggerConfiguration;
  }

  bool ClientHelper::parseUrlAddress(const std::string& url, std::string& address, uint16_t& port)
  {
    auto pos = url.find("://");
    size_t addrStart = 0;

    if (pos != std::string::npos)
      addrStart = pos + 3;

    auto addrEnd = url.find(':', addrStart);

    if (addrEnd != std::string::npos)
    {
      auto portEnd = url.find('/', addrEnd);
      port = common::fromString<uint16_t>(url.substr(
        addrEnd + 1, portEnd == std::string::npos ? std::string::npos : portEnd - addrEnd - 1));
    }
    else
    {
      addrEnd = url.find('/');
      port = 80;
    }

    address = url.substr(addrStart, addrEnd - addrStart);

    return true;
  }

  std::error_code ClientHelper::initAndLoadWallet(cn::IWalletLegacy& wallet, std::istream& walletFile, const std::string& password)
  {
    WalletHelper::InitWalletResultObserver initObserver;
    std::future<std::error_code> f_initError = initObserver.initResult.get_future();

    WalletHelper::IWalletRemoveObserverGuard removeGuard(wallet, initObserver);
    wallet.initAndLoad(walletFile, password);
    auto initError = f_initError.get();

    return initError;
  }

  std::string ClientHelper::tryToOpenWalletOrLoadKeysOrThrow(logging::LoggerRef& logger, std::unique_ptr<cn::IWalletLegacy>& wallet, const std::string& walletFile, const std::string& password)
  {
    std::string keys_file, walletFileName;
    WalletHelper::prepareFileNames(walletFile, keys_file, walletFileName);

    boost::system::error_code ignore;

    bool keysExists = boost::filesystem::exists(keys_file, ignore);
    bool walletExists = boost::filesystem::exists(walletFileName, ignore);

    if (!walletExists && !keysExists && boost::filesystem::exists(walletFile, ignore))
    {
      boost::system::error_code renameEc;
      boost::filesystem::rename(walletFile, walletFileName, renameEc);

      if (renameEc)
        throw std::runtime_error("failed to rename file '" + walletFile + "' to '" + walletFileName + "': " + renameEc.message());

      walletExists = true;
    }

    if (walletExists)
    {
      logger(logging::INFO) << "Loading wallet...";

      std::ifstream walletFile;
      walletFile.open(walletFileName, std::ios_base::binary | std::ios_base::in);

      if (walletFile.fail())
        throw std::runtime_error("error opening wallet file '" + walletFileName + "'");

      auto initError = initAndLoadWallet(*wallet, walletFile, password);

      walletFile.close();

      if (initError)
      { //bad password, or legacy format
        if (keysExists)
        {
          std::stringstream ss;
          cn::importLegacyKeys(keys_file, password, ss);

          boost::filesystem::rename(keys_file, keys_file + ".back");
          boost::filesystem::rename(walletFileName, walletFileName + ".back");

          initError = initAndLoadWallet(*wallet, ss, password);
          if (initError)
            throw std::runtime_error("failed to load wallet: " + initError.message());

          // logger(logging::INFO) << "Saving wallet...";
          // save_wallet(*wallet, walletFileName, logger);
          // logger(logging::INFO, logging::BRIGHT_GREEN) << "Saving successful";

          return walletFileName;
        }
        else
        { // no keys, wallet error loading
          throw std::runtime_error("can't load wallet file '" + walletFileName + "', check password");
        }
      }
      else
      { //new wallet ok
        return walletFileName;
      }
    }
    else if (keysExists)
    { //wallet not exists but keys presented
      std::stringstream ss;
      cn::importLegacyKeys(keys_file, password, ss);
      boost::filesystem::rename(keys_file, keys_file + ".back");

      WalletHelper::InitWalletResultObserver initObserver;
      std::future<std::error_code> f_initError = initObserver.initResult.get_future();

      WalletHelper::IWalletRemoveObserverGuard removeGuard(*wallet, initObserver);
      wallet->initAndLoad(ss, password);
      auto initError = f_initError.get();

      removeGuard.removeObserver();

      if (initError) {
        throw std::runtime_error("failed to load wallet: " + initError.message());
      }

      // logger(logging::INFO) << "Saving wallet...";
      // save_wallet(*wallet, walletFileName, logger);
      // logger(logging::INFO, logging::BRIGHT_GREEN) << "Saved successful";

      return walletFileName;
    }
    else
    { //no wallet no keys
      throw std::runtime_error("wallet file '" + walletFileName + "' is not found");
    }
  }

  std::stringstream ClientHelper::balances(const cn::IWallet &wallet, const Currency &currency) const
  {
    std::stringstream balances;

    uint64_t actualBalance = wallet.getActualBalance();
    uint64_t pendingBalance = wallet.getPendingBalance();
    uint64_t unlockedDepositBalance = wallet.getUnlockedDepositBalance();
    uint64_t lockedDepositBalance = wallet.getLockedDepositBalance();

    uint64_t totalBalance = actualBalance + pendingBalance + unlockedDepositBalance + lockedDepositBalance;
    balances << "Total Balance: " + currency.formatAmount(totalBalance) << std::endl
             << "Available: " + currency.formatAmount(actualBalance) << std::endl
             << "Locked: " + currency.formatAmount(pendingBalance) << std::endl
             << "Unlocked Balance: " + currency.formatAmount(unlockedDepositBalance) << std::endl
             << "Locked Deposits: " + currency.formatAmount(lockedDepositBalance);

    return balances;
  }
}
