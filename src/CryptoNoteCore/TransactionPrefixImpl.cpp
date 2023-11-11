// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ITransaction.h"

#include <numeric>
#include <system_error>

#include "CryptoNoteCore/CryptoNoteBasic.h"
#include "CryptoNoteCore/TransactionApiExtra.h"
#include "TransactionUtils.h"
#include "CryptoNoteCore/CryptoNoteTools.h"

using namespace crypto;

namespace cn {

class TransactionPrefixImpl : public ITransactionReader {
public:
  TransactionPrefixImpl() = default;
  TransactionPrefixImpl(const TransactionPrefix& prefix, const Hash& transactionHash);

  ~TransactionPrefixImpl() override = default;

  Hash getTransactionHash() const override;
  Hash getTransactionPrefixHash() const override;
  Hash getTransactionInputsHash() const override;
  PublicKey getTransactionPublicKey() const override;
  uint64_t getUnlockTime() const override;

  // extra
  bool getPaymentId(Hash& paymentId) const override;
  bool getExtraNonce(BinaryArray& nonce) const override;
  BinaryArray getExtra() const override;

  // inputs
  size_t getInputCount() const override;
  uint64_t getInputTotalAmount() const override;
  transaction_types::InputType getInputType(size_t index) const override;
  void getInput(size_t index, KeyInput& input) const override;
  void getInput(size_t index, MultisignatureInput& input) const override;
  std::vector<TransactionInput> getInputs() const override;

  // outputs
  size_t getOutputCount() const override;
  uint64_t getOutputTotalAmount() const override;
  transaction_types::OutputType getOutputType(size_t index) const override;
  void getOutput(size_t index, KeyOutput& output, uint64_t& amount) const override;
  void getOutput(size_t index, MultisignatureOutput& output, uint64_t& amount) const override;

  // signatures
  size_t getRequiredSignaturesCount(size_t inputIndex) const override;
  bool findOutputsToAccount(const AccountPublicAddress& addr, const SecretKey& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const override;

  // various checks
  bool validateInputs() const override;
  bool validateOutputs() const override;
  bool validateSignatures() const override;

  // serialized transaction
  BinaryArray getTransactionData() const override;
  TransactionPrefix getTransactionPrefix() const override;
  bool getTransactionSecretKey(SecretKey& key) const override;

private:
  TransactionPrefix m_txPrefix;
  TransactionExtra m_extra;
  Hash m_txHash;
};

TransactionPrefixImpl::TransactionPrefixImpl(const TransactionPrefix& prefix, const Hash& transactionHash): m_txPrefix(prefix), m_txHash(transactionHash) {
  m_extra.parse(prefix.extra);
}

Hash TransactionPrefixImpl::getTransactionHash() const {
  return m_txHash;
}

Hash TransactionPrefixImpl::getTransactionPrefixHash() const {
  return getObjectHash(m_txPrefix);
}

Hash TransactionPrefixImpl::getTransactionInputsHash() const
{
  return getObjectHash(m_txPrefix.inputs);
}

PublicKey TransactionPrefixImpl::getTransactionPublicKey() const {
  crypto::PublicKey pk(NULL_PUBLIC_KEY);
  m_extra.getPublicKey(pk);
  return pk;
}

uint64_t TransactionPrefixImpl::getUnlockTime() const {
  return m_txPrefix.unlockTime;
}

bool TransactionPrefixImpl::getPaymentId(Hash& hash) const {
  BinaryArray nonce;

  if (getExtraNonce(nonce)) {
    crypto::Hash paymentId;
    if (getPaymentIdFromTransactionExtraNonce(nonce, paymentId)) {
      hash = reinterpret_cast<const Hash&>(paymentId);
      return true;
    }
  }

  return false;
}

bool TransactionPrefixImpl::getExtraNonce(BinaryArray& nonce) const {
  TransactionExtraNonce extraNonce;

  if (m_extra.get(extraNonce)) {
    nonce = extraNonce.nonce;
    return true;
  }

  return false;
}

BinaryArray TransactionPrefixImpl::getExtra() const {
  return m_txPrefix.extra;
}

size_t TransactionPrefixImpl::getInputCount() const {
  return m_txPrefix.inputs.size();
}

uint64_t TransactionPrefixImpl::getInputTotalAmount() const {
  return std::accumulate(m_txPrefix.inputs.begin(), m_txPrefix.inputs.end(), 0ULL, [](uint64_t val, const TransactionInput& in) {
    return val + getTransactionInputAmount(in); });
}

transaction_types::InputType TransactionPrefixImpl::getInputType(size_t index) const {
  return getTransactionInputType(getInputChecked(m_txPrefix, index));
}

void TransactionPrefixImpl::getInput(size_t index, KeyInput& input) const {
  input = boost::get<KeyInput>(getInputChecked(m_txPrefix, index, transaction_types::InputType::Key));
}

void TransactionPrefixImpl::getInput(size_t index, MultisignatureInput& input) const {
  input = boost::get<MultisignatureInput>(getInputChecked(m_txPrefix, index, transaction_types::InputType::Multisignature));
}

size_t TransactionPrefixImpl::getOutputCount() const {
  return m_txPrefix.outputs.size();
}

uint64_t TransactionPrefixImpl::getOutputTotalAmount() const {
  return std::accumulate(m_txPrefix.outputs.begin(), m_txPrefix.outputs.end(), 0ULL, [](uint64_t val, const TransactionOutput& out) {
    return val + out.amount; });
}

transaction_types::OutputType TransactionPrefixImpl::getOutputType(size_t index) const {
  return getTransactionOutputType(getOutputChecked(m_txPrefix, index).target);
}

void TransactionPrefixImpl::getOutput(size_t index, KeyOutput& output, uint64_t& amount) const {
  const auto& out = getOutputChecked(m_txPrefix, index, transaction_types::OutputType::Key);
  output = boost::get<KeyOutput>(out.target);
  amount = out.amount;
}

std::vector<TransactionInput> TransactionPrefixImpl::getInputs() const
{
  return m_txPrefix.inputs;
}

void TransactionPrefixImpl::getOutput(size_t index, MultisignatureOutput& output, uint64_t& amount) const {
  const auto& out = getOutputChecked(m_txPrefix, index, transaction_types::OutputType::Multisignature);
  output = boost::get<MultisignatureOutput>(out.target);
  amount = out.amount;
}

size_t TransactionPrefixImpl::getRequiredSignaturesCount(size_t inputIndex) const {
  return ::cn::getRequiredSignaturesCount(getInputChecked(m_txPrefix, inputIndex));
}

bool TransactionPrefixImpl::findOutputsToAccount(const AccountPublicAddress& addr, const SecretKey& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const {
  return ::cn::findOutputsToAccount(m_txPrefix, addr, viewSecretKey, outs, outputAmount);
}

bool TransactionPrefixImpl::validateInputs() const {
  return check_inputs_types_supported(m_txPrefix) &&
          check_inputs_overflow(m_txPrefix) &&
          checkInputsKeyimagesDiff(m_txPrefix) &&
          checkMultisignatureInputsDiff(m_txPrefix);
}

bool TransactionPrefixImpl::validateOutputs() const {
  return check_outs_valid(m_txPrefix) &&
          check_outs_overflow(m_txPrefix);
}

bool TransactionPrefixImpl::validateSignatures() const {
  throw std::system_error(std::make_error_code(std::errc::function_not_supported), "Validating signatures is not supported for transaction prefix");
}

BinaryArray TransactionPrefixImpl::getTransactionData() const {
  return toBinaryArray(m_txPrefix);
}

TransactionPrefix TransactionPrefixImpl::getTransactionPrefix() const
{
  return m_txPrefix;
}

bool TransactionPrefixImpl::getTransactionSecretKey(SecretKey& key) const {
  return false;
}


std::unique_ptr<ITransactionReader> createTransactionPrefix(const TransactionPrefix& prefix, const Hash& transactionHash) {
  return std::make_unique<TransactionPrefixImpl>(prefix, transactionHash);
}

std::unique_ptr<ITransactionReader> createTransactionPrefix(const Transaction& fullTransaction) {
  return std::make_unique<TransactionPrefixImpl>(fullTransaction, getObjectHash(fullTransaction));
}

}
