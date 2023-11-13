// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "Common/JsonValue.h"
#include "ISerializer.h"

namespace cn {

//deserialization
class JsonInputValueSerializer : public ISerializer {
public:
  explicit JsonInputValueSerializer(const common::JsonValue& value);
  explicit JsonInputValueSerializer(common::JsonValue&& value);
  ~JsonInputValueSerializer() override = default;

  SerializerType type() const override;

  bool beginObject(std::string_view name) override;
  void endObject() override;

  bool beginArray(size_t& size, std::string_view name) override;
  void endArray() override;

  bool operator()(uint8_t& value, std::string_view name) override;
  bool operator()(int16_t& value, std::string_view name) override;
  bool operator()(uint16_t& value, std::string_view name) override;
  bool operator()(int32_t& value, std::string_view name) override;
  bool operator()(uint32_t& value, std::string_view name) override;
  bool operator()(int64_t& value, std::string_view name) override;
  bool operator()(uint64_t& value, std::string_view name) override;
  bool operator()(double& value, std::string_view name) override;
  bool operator()(bool& value, std::string_view name) override;
  bool operator()(std::string& value, std::string_view name) override;
  bool binary(uint8_t* value, size_t size, std::string_view name) override;
  bool binary(std::string& value, std::string_view name) override;

  template<typename T>
  bool operator()(T& value, std::string_view name) {
    return ISerializer::operator()(value, name);
  }

private:
  common::JsonValue root;
  std::vector<const common::JsonValue*> chain;
  std::vector<size_t> idxs;

  const common::JsonValue* getValue(std::string_view name);

  template <typename T>
  bool getNumber(std::string_view name, T& v) {
    auto ptr = getValue(name);

    if (!ptr) {
      return false;
    }

    v = static_cast<T>(ptr->getInteger());
    return true;
  }
};

}
