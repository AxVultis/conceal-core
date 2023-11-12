// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <Common/IOutputStream.h>
#include "ISerializer.h"
#include "MemoryStream.h"

namespace cn {

class KVBinaryOutputStreamSerializer : public ISerializer {
public:

  KVBinaryOutputStreamSerializer();
  ~KVBinaryOutputStreamSerializer() override = default;

  void dump(common::IOutputStream& target);

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
  bool binary(void* value, size_t size, std::string_view name) override;
  bool binary(std::string& value, std::string_view name) override;

  template<typename T>
  bool operator()(T& value, std::string_view name) {
    return ISerializer::operator()(value, name);
  }

private:

  void writeElementPrefix(uint8_t type, std::string_view name);
  void checkArrayPreamble(uint8_t type);
  void updateState(uint8_t type);
  MemoryStream& stream();

  enum class State {
    Root,
    Object,
    ArrayPrefix,
    Array
  };

  struct Level {
    State state = State::Object;
    std::string name;
    size_t count = 0;

    explicit Level(std::string_view nm) :
      name(nm) {}

    Level(std::string_view nm, size_t arraySize) :
      state(State::ArrayPrefix), name(nm), count(arraySize) {}
  };

  std::vector<MemoryStream> m_objectsStack;
  std::vector<Level> m_stack;
};

}
