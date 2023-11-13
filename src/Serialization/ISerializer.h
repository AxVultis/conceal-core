// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <cstdint>

namespace cn {

class ISerializer {
public:

  enum class SerializerType {
    INPUT,
    OUTPUT
  };

  virtual ~ISerializer() = default;

  virtual SerializerType type() const = 0;

  virtual bool beginObject(std::string_view name) = 0;
  virtual void endObject() = 0;
  virtual bool beginArray(size_t& size, std::string_view name) = 0;
  virtual void endArray() = 0;

  virtual bool operator()(uint8_t& value, std::string_view name) = 0;
  virtual bool operator()(int16_t& value, std::string_view name) = 0;
  virtual bool operator()(uint16_t& value, std::string_view name) = 0;
  virtual bool operator()(int32_t& value, std::string_view name) = 0;
  virtual bool operator()(uint32_t& value, std::string_view name) = 0;
  virtual bool operator()(int64_t& value, std::string_view name) = 0;
  virtual bool operator()(uint64_t& value, std::string_view name) = 0;
  virtual bool operator()(double& value, std::string_view name) = 0;
  virtual bool operator()(bool& value, std::string_view name) = 0;
  virtual bool operator()(std::string& value, std::string_view name) = 0;
  
  // read/write binary block
  virtual bool binary(uint8_t* value, size_t size, std::string_view name) = 0;
  virtual bool binary(std::string& value, std::string_view name) = 0;

  template<typename T>
  bool operator()(T& value, std::string_view name);
};

template<typename T>
bool ISerializer::operator()(T& value, std::string_view name) {
  return serialize(value, name, *this);
}

template<typename T>
bool serialize(T& value, std::string_view name, ISerializer& serializer) {
  if (!serializer.beginObject(name)) {
    return false;
  }

  serialize(value, serializer);
  serializer.endObject();
  return true;
}

template<typename T>
void serialize(T& value, ISerializer& serializer) {
  value.serialize(serializer);
}

#ifdef __clang__
template<> inline
bool ISerializer::operator()(size_t& value, std::string_view name) {
  return operator()(*reinterpret_cast<uint64_t*>(&value), name);
}
#endif

#define KV_MEMBER(member) s(member, #member);

}
