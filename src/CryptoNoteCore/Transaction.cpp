// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ITransaction.h"
#include "TransactionApiExtra.h"
#include "TransactionUtils.h"

#include "Account.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteConfig.h"

#include <boost/optional.hpp>
#include <numeric>
#include <unordered_set>

using namespace crypto;

namespace {

  using namespace cn;

  void derivePublicKey(const AccountPublicAddress& to, const SecretKey& txKey, size_t outputIndex, PublicKey& ephemeralKey) {
    KeyDerivation derivation;
    generate_key_derivation(to.viewPublicKey, txKey, derivation);
    derive_public_key(derivation, outputIndex, to.spendPublicKey, ephemeralKey);
  }

}

namespace cn {

  using namespace crypto;

  ////////////////////////////////////////////////////////////////////////
  // class Transaction declaration
  ////////////////////////////////////////////////////////////////////////

  class TransactionImpl : public ITransaction {
  public:
    TransactionImpl();
    explicit TransactionImpl(const BinaryArray& txblob);
    explicit TransactionImpl(const cn::Transaction& tx);
  
    // ITransactionReader
    Hash getTransactionHash() const override;
    Hash getTransactionPrefixHash() const override;
    Hash getTransactionInputsHash() const override;
    PublicKey getTransactionPublicKey() const override;
    uint64_t getUnlockTime() const override;
    bool getPaymentId(Hash& hash) const override;
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

    size_t getRequiredSignaturesCount(size_t index) const override;
    bool findOutputsToAccount(const AccountPublicAddress& addr, const SecretKey& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const override;

    // various checks
    bool validateInputs() const override;
    bool validateOutputs() const override;
    bool validateSignatures() const override;

    // get serialized transaction
    BinaryArray getTransactionData() const override;
    TransactionPrefix getTransactionPrefix() const override;
    // ITransactionWriter

    void setUnlockTime(uint64_t unlockTime) override;
    void setPaymentId(const Hash& hash) override;
    void setExtraNonce(const BinaryArray& nonce) override;
    void appendExtra(const BinaryArray& extraData) override;

    // Inputs/Outputs 
    size_t addInput(const KeyInput& input) override;
    size_t addInput(const MultisignatureInput& input) override;
    size_t addInput(const AccountKeys& senderKeys, const transaction_types::InputKeyInfo& info, KeyPair& ephKeys) override;

    size_t addOutput(uint64_t amount, const AccountPublicAddress& to) override;
    size_t addOutput(uint64_t amount, const std::vector<AccountPublicAddress>& to, uint8_t requiredSignatures, uint32_t term = 0) override;
    size_t addOutput(uint64_t amount, const KeyOutput& out) override;
    size_t addOutput(uint64_t amount, const MultisignatureOutput& out) override;

    void signInputKey(size_t input, const transaction_types::InputKeyInfo& info, const KeyPair& ephKeys) override;
    void signInputMultisignature(size_t input, const PublicKey& sourceTransactionKey, size_t outputIndex, const AccountKeys& accountKeys) override;
    void signInputMultisignature(size_t input, const KeyPair& ephemeralKeys) override;


    // secret key
    bool getTransactionSecretKey(SecretKey& key) const override;
    void setTransactionSecretKey(const SecretKey& key) override;
    void setDeterministicTransactionSecretKey(const SecretKey& key) override;

  private:

    void invalidateHash();

    std::vector<Signature>& getSignatures(size_t input);

    const SecretKey& txSecretKey() const {
      if (!secretKey) {
        throw std::runtime_error("Operation requires transaction secret key");
      }
      return *secretKey;
    }

    void checkIfSigning() const {
      if (!transaction.signatures.empty()) {
        throw std::runtime_error("Cannot perform requested operation, since it will invalidate transaction signatures");
      }
    }

    cn::Transaction transaction;
    boost::optional<SecretKey> secretKey;
    mutable boost::optional<Hash> transactionHash;
    TransactionExtra extra;
  };


  ////////////////////////////////////////////////////////////////////////
  // class Transaction implementation
  ////////////////////////////////////////////////////////////////////////

  std::unique_ptr<ITransaction> createTransaction() {
    return std::make_unique<TransactionImpl>();
  }

  std::unique_ptr<ITransaction> createTransaction(const BinaryArray& transactionBlob) {
    return std::make_unique<TransactionImpl>(transactionBlob);
  }

  std::unique_ptr<ITransaction> createTransaction(const cn::Transaction& tx) {
    return std::make_unique<TransactionImpl>(tx);
  }

  TransactionImpl::TransactionImpl() {   
    cn::KeyPair txKeys(cn::generateKeyPair());

    TransactionExtraPublicKey pk = { txKeys.publicKey };
    extra.set(pk);

    transaction.version = TRANSACTION_VERSION_1;
    transaction.unlockTime = 0;
    transaction.extra = extra.serialize();

    secretKey = txKeys.secretKey;
  }

  TransactionImpl::TransactionImpl(const BinaryArray& ba) {
    if (!fromBinaryArray(transaction, ba)) {
      throw std::runtime_error("Invalid transaction data");
    }
    
    extra.parse(transaction.extra);
    transactionHash = getBinaryArrayHash(ba); // avoid serialization if we already have blob
  }

  TransactionImpl::TransactionImpl(const cn::Transaction& tx) : transaction(tx) {
    extra.parse(transaction.extra);
  }

  void TransactionImpl::invalidateHash() {
    if (transactionHash.is_initialized()) {
      transactionHash = decltype(transactionHash)();
    }
  }

  Hash TransactionImpl::getTransactionHash() const {
    if (!transactionHash.is_initialized()) {
      transactionHash = getObjectHash(transaction);
    }

    return transactionHash.get();   
  }

  Hash TransactionImpl::getTransactionPrefixHash() const {
    return getObjectHash(*static_cast<const TransactionPrefix*>(&transaction));
  }

  PublicKey TransactionImpl::getTransactionPublicKey() const {
    PublicKey pk(NULL_PUBLIC_KEY);
    extra.getPublicKey(pk);
    return pk;
  }

  uint64_t TransactionImpl::getUnlockTime() const {
    return transaction.unlockTime;
  }

  void TransactionImpl::setUnlockTime(uint64_t unlockTime) {
    checkIfSigning();
    transaction.unlockTime = unlockTime;
    invalidateHash();
  }

  Hash TransactionImpl::getTransactionInputsHash() const
  {
    return getObjectHash(transaction.inputs);
  }

  bool TransactionImpl::getTransactionSecretKey(SecretKey& key) const {
    if (!secretKey) {
      return false;
    }
    key = reinterpret_cast<const SecretKey&>(secretKey.get());
    return true;
  }

  void TransactionImpl::setTransactionSecretKey(const SecretKey& key) {
    const auto& sk = reinterpret_cast<const SecretKey&>(key);
    PublicKey pk;
    PublicKey txPubKey;

    secret_key_to_public_key(sk, pk);
    extra.getPublicKey(txPubKey);

    if (txPubKey != pk) {
      throw std::runtime_error("Secret transaction key does not match public key");
    }

    secretKey = key;
  }

  void TransactionImpl::setDeterministicTransactionSecretKey(const SecretKey& key)
  {
    checkIfSigning();
    KeyPair deterministicTxKeys;
    generateDeterministicTransactionKeys(getTransactionInputsHash(), key, deterministicTxKeys);

    TransactionExtraPublicKey pk = { deterministicTxKeys.publicKey };
    extra.set(pk);

    transaction.extra = extra.serialize();

    secretKey = deterministicTxKeys.secretKey;
  }

  size_t TransactionImpl::addInput(const KeyInput& input) {
    checkIfSigning();
    transaction.inputs.emplace_back(input);
    invalidateHash();
    return transaction.inputs.size() - 1;
  }

  size_t TransactionImpl::addInput(const AccountKeys& senderKeys, const transaction_types::InputKeyInfo& info, KeyPair& ephKeys) {
    checkIfSigning();
    KeyInput input;
    input.amount = info.amount;

    generate_key_image_helper(
      senderKeys,
      info.realOutput.transactionPublicKey,
      info.realOutput.outputInTransaction,
      ephKeys,
      input.keyImage);

    // fill outputs array and use relative offsets
    for (const auto& out : info.outputs) {
      input.outputIndexes.push_back(out.outputIndex);
    }

    input.outputIndexes = absolute_output_offsets_to_relative(input.outputIndexes);
    return addInput(input);
  }

  size_t TransactionImpl::addInput(const MultisignatureInput& input) {
    checkIfSigning();
    transaction.inputs.emplace_back(input);
    transaction.version = TRANSACTION_VERSION_2;
    invalidateHash();
    return transaction.inputs.size() - 1;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const AccountPublicAddress& to) {
    checkIfSigning();

    KeyOutput outKey;
    derivePublicKey(to, txSecretKey(), transaction.outputs.size(), outKey.key);
    transaction.outputs.emplace_back(TransactionOutput{amount, outKey});
    invalidateHash();

    return transaction.outputs.size() - 1;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const std::vector<AccountPublicAddress>& to, uint8_t requiredSignatures, uint32_t term) {
    checkIfSigning();

    const auto& txKey = txSecretKey();
    size_t outputIndex = transaction.outputs.size();
    MultisignatureOutput outMsig;
    outMsig.requiredSignatureCount = requiredSignatures;
    outMsig.keys.resize(to.size());
    outMsig.term = term;
    
    for (size_t i = 0; i < to.size(); ++i) {
      derivePublicKey(to[i], txKey, outputIndex, outMsig.keys[i]);
    }

    transaction.outputs.emplace_back(TransactionOutput{amount, outMsig});
    transaction.version = TRANSACTION_VERSION_2;
    invalidateHash();

    return outputIndex;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const KeyOutput& out) {
    checkIfSigning();
    size_t outputIndex = transaction.outputs.size();
    transaction.outputs.emplace_back(TransactionOutput{amount, out});
    invalidateHash();
    return outputIndex;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const MultisignatureOutput& out) {
    checkIfSigning();
    size_t outputIndex = transaction.outputs.size();
    transaction.outputs.emplace_back(TransactionOutput{amount, out});
    invalidateHash();
    return outputIndex;
  }

  void TransactionImpl::signInputKey(size_t index, const transaction_types::InputKeyInfo& info, const KeyPair& ephKeys) {
    const auto& input = boost::get<KeyInput>(getInputChecked(transaction, index, transaction_types::InputType::Key));
    Hash prefixHash = getTransactionPrefixHash();

    std::vector<Signature> signatures;
    std::vector<const PublicKey*> keysPtrs;

    for (const auto& o : info.outputs) {
      keysPtrs.push_back(reinterpret_cast<const PublicKey*>(&o.targetKey));
    }

    signatures.resize(keysPtrs.size());

    generate_ring_signature(
      reinterpret_cast<const Hash&>(prefixHash),
      reinterpret_cast<const KeyImage&>(input.keyImage),
      keysPtrs,
      reinterpret_cast<const SecretKey&>(ephKeys.secretKey),
      info.realOutput.transactionIndex,
      signatures.data());

    getSignatures(index) = signatures;
    invalidateHash();
  }

  void TransactionImpl::signInputMultisignature(size_t index, const PublicKey& sourceTransactionKey, size_t outputIndex, const AccountKeys& accountKeys) {
    KeyDerivation derivation;
    PublicKey ephemeralPublicKey;
    SecretKey ephemeralSecretKey;

    generate_key_derivation(
      reinterpret_cast<const PublicKey&>(sourceTransactionKey),
      reinterpret_cast<const SecretKey&>(accountKeys.viewSecretKey),
      derivation);

    derive_public_key(derivation, outputIndex,
      reinterpret_cast<const PublicKey&>(accountKeys.address.spendPublicKey), ephemeralPublicKey);
    derive_secret_key(derivation, outputIndex,
      reinterpret_cast<const SecretKey&>(accountKeys.spendSecretKey), ephemeralSecretKey);

    Signature signature;
    auto txPrefixHash = getTransactionPrefixHash();

    generate_signature(reinterpret_cast<const Hash&>(txPrefixHash),
      ephemeralPublicKey, ephemeralSecretKey, signature);

    getSignatures(index).push_back(signature);
    invalidateHash();
  }

  void TransactionImpl::signInputMultisignature(size_t index, const KeyPair& ephemeralKeys) {
    Signature signature;
    auto txPrefixHash = getTransactionPrefixHash();

    generate_signature(txPrefixHash, ephemeralKeys.publicKey, ephemeralKeys.secretKey, signature);

    getSignatures(index).push_back(signature);
    invalidateHash();
  }

  std::vector<Signature>& TransactionImpl::getSignatures(size_t input) {
    // update signatures container size if needed
    if (transaction.signatures.size() < transaction.inputs.size()) {
      transaction.signatures.resize(transaction.inputs.size());
    }
    // check range
    if (input >= transaction.signatures.size()) {
      throw std::runtime_error("Invalid input index");
    }

    return transaction.signatures[input];
  }

  BinaryArray TransactionImpl::getTransactionData() const {
    return toBinaryArray(transaction);
  }

  TransactionPrefix TransactionImpl::getTransactionPrefix() const
  {
    return transaction;
  }

  void TransactionImpl::setPaymentId(const Hash& hash) {
    checkIfSigning();
    BinaryArray paymentIdBlob;
    setPaymentIdToTransactionExtraNonce(paymentIdBlob, reinterpret_cast<const Hash&>(hash));
    setExtraNonce(paymentIdBlob);
  }

  bool TransactionImpl::getPaymentId(Hash& hash) const {
    BinaryArray nonce;
    if (getExtraNonce(nonce)) {
      Hash paymentId;
      if (getPaymentIdFromTransactionExtraNonce(nonce, paymentId)) {
        hash = reinterpret_cast<const Hash&>(paymentId);
        return true;
      }
    }
    return false;
  }

  void TransactionImpl::setExtraNonce(const BinaryArray& nonce) {
    checkIfSigning();
    TransactionExtraNonce extraNonce = { nonce };
    extra.set(extraNonce);
    transaction.extra = extra.serialize();
    invalidateHash();
  }

  void TransactionImpl::appendExtra(const BinaryArray& extraData) {
    checkIfSigning();
    transaction.extra.insert(
      transaction.extra.end(), extraData.begin(), extraData.end());
  }

  bool TransactionImpl::getExtraNonce(BinaryArray& nonce) const {
    TransactionExtraNonce extraNonce;
    if (extra.get(extraNonce)) {
      nonce = extraNonce.nonce;
      return true;
    }
    return false;
  }

  BinaryArray TransactionImpl::getExtra() const {
    return transaction.extra;
  }

  size_t TransactionImpl::getInputCount() const {
    return transaction.inputs.size();
  }

  uint64_t TransactionImpl::getInputTotalAmount() const {
    return std::accumulate(transaction.inputs.begin(), transaction.inputs.end(), 0ULL, [](uint64_t val, const TransactionInput& in) {
      return val + getTransactionInputAmount(in); });
  }

  transaction_types::InputType TransactionImpl::getInputType(size_t index) const {
    return getTransactionInputType(getInputChecked(transaction, index));
  }

  void TransactionImpl::getInput(size_t index, KeyInput& input) const {
    input = boost::get<KeyInput>(getInputChecked(transaction, index, transaction_types::InputType::Key));
  }

  void TransactionImpl::getInput(size_t index, MultisignatureInput& input) const {
    input = boost::get<MultisignatureInput>(getInputChecked(transaction, index, transaction_types::InputType::Multisignature));
  }

  size_t TransactionImpl::getOutputCount() const {
    return transaction.outputs.size();
  }

  std::vector<TransactionInput> TransactionImpl::getInputs() const
  {
    return transaction.inputs;
  }

  uint64_t TransactionImpl::getOutputTotalAmount() const {
    return std::accumulate(transaction.outputs.begin(), transaction.outputs.end(), 0ULL, [](uint64_t val, const TransactionOutput& out) {
      return val + out.amount; });
  }

  transaction_types::OutputType TransactionImpl::getOutputType(size_t index) const {
    return getTransactionOutputType(getOutputChecked(transaction, index).target);
  }

  void TransactionImpl::getOutput(size_t index, KeyOutput& output, uint64_t& amount) const {
    const auto& out = getOutputChecked(transaction, index, transaction_types::OutputType::Key);
    output = boost::get<KeyOutput>(out.target);
    amount = out.amount;
  }

  void TransactionImpl::getOutput(size_t index, MultisignatureOutput& output, uint64_t& amount) const {
    const auto& out = getOutputChecked(transaction, index, transaction_types::OutputType::Multisignature);
    output = boost::get<MultisignatureOutput>(out.target);
    amount = out.amount;
  }

  bool TransactionImpl::findOutputsToAccount(const AccountPublicAddress& addr, const SecretKey& viewSecretKey, std::vector<uint32_t>& out, uint64_t& amount) const {
    return ::cn::findOutputsToAccount(transaction, addr, viewSecretKey, out, amount);
  }

  size_t TransactionImpl::getRequiredSignaturesCount(size_t index) const {
    return ::getRequiredSignaturesCount(getInputChecked(transaction, index));
  }

  bool TransactionImpl::validateInputs() const {
    return
      check_inputs_types_supported(transaction) &&
      check_inputs_overflow(transaction) &&
      checkInputsKeyimagesDiff(transaction) &&
      checkMultisignatureInputsDiff(transaction);
  }

  bool TransactionImpl::validateOutputs() const {
    return
      check_outs_valid(transaction) &&
      check_outs_overflow(transaction);
  }

  bool TransactionImpl::validateSignatures() const {
    if (transaction.signatures.size() < transaction.inputs.size()) {
      return false;
    }

    for (size_t i = 0; i < transaction.inputs.size(); ++i) {
      if (getRequiredSignaturesCount(i) > transaction.signatures[i].size()) {
        return false;
      }
    }

    return true;
  }
}
