// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <Common/IInputStream.h>
#include "ISerializer.h"
#include "SerializationOverloads.h"

namespace cn {

class BinaryInputStreamSerializer : public ISerializer {
public:
  explicit BinaryInputStreamSerializer(common::IInputStream& strm) : stream(strm) {};
  ~BinaryInputStreamSerializer() override= default;

  ISerializer::SerializerType type() const override;

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

  void checkedRead(char* buf, size_t size);
  common::IInputStream& stream;
};

}
