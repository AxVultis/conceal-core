// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "JsonOutputStreamSerializer.h"
#include <cassert>
#include <stdexcept>
#include "Common/StringTools.h"

using common::JsonValue;
using namespace cn;

namespace cn {
std::ostream& operator<<(std::ostream& out, const JsonOutputStreamSerializer& enumerator) {
  out << enumerator.root;
  return out;
}
}

namespace {

template <typename T>
void insertOrPush(JsonValue& js, std::string_view name, const T& value) {
  if (js.isArray()) {
    js.pushBack(JsonValue(value));
  } else {
    js.insert(std::string(name), JsonValue(value));
  }
}

}

JsonOutputStreamSerializer::JsonOutputStreamSerializer() {
  chain.push_back(&root);
}

ISerializer::SerializerType JsonOutputStreamSerializer::type() const {
  return ISerializer::SerializerType::OUTPUT;
}

bool JsonOutputStreamSerializer::beginObject(std::string_view name) {
  JsonValue& parent = *chain.back();
  JsonValue obj(JsonValue::OBJECT);

  if (parent.isObject()) {
    chain.push_back(&parent.insert(std::string(name), obj));
  } else {
    chain.push_back(&parent.pushBack(obj));
  }

  return true;
}

void JsonOutputStreamSerializer::endObject() {
  assert(!chain.empty());
  chain.pop_back();
}

bool JsonOutputStreamSerializer::beginArray(size_t& size, std::string_view name) {
  JsonValue val(JsonValue::ARRAY);
  JsonValue& res = chain.back()->insert(std::string(name), val);
  chain.push_back(&res);
  return true;
}

void JsonOutputStreamSerializer::endArray() {
  assert(!chain.empty());
  chain.pop_back();
}

bool JsonOutputStreamSerializer::operator()(uint64_t& value, std::string_view name) {
  auto v = static_cast<int64_t>(value);
  return operator()(v, name);
}

bool JsonOutputStreamSerializer::operator()(uint16_t& value, std::string_view name) {
  auto v = static_cast<uint64_t>(value);
  return operator()(v, name);
}

bool JsonOutputStreamSerializer::operator()(int16_t& value, std::string_view name) {
  auto v = static_cast<int64_t>(value);
  return operator()(v, name);
}

bool JsonOutputStreamSerializer::operator()(uint32_t& value, std::string_view name) {
  auto v = static_cast<uint64_t>(value);
  return operator()(v, name);
}

bool JsonOutputStreamSerializer::operator()(int32_t& value, std::string_view name) {
  auto v = static_cast<int64_t>(value);
  return operator()(v, name);
}

bool JsonOutputStreamSerializer::operator()(int64_t& value, std::string_view name) {
  insertOrPush(*chain.back(), name, value);
  return true;
}

bool JsonOutputStreamSerializer::operator()(double& value, std::string_view name) {
  insertOrPush(*chain.back(), name, value);
  return true;
}

bool JsonOutputStreamSerializer::operator()(std::string& value, std::string_view name) {
  insertOrPush(*chain.back(), name, value);
  return true;
}

bool JsonOutputStreamSerializer::operator()(uint8_t& value, std::string_view name) {
  insertOrPush(*chain.back(), name, static_cast<int64_t>(value));
  return true;
}

bool JsonOutputStreamSerializer::operator()(bool& value, std::string_view name) {
  insertOrPush(*chain.back(), name, value);
  return true;
}

bool JsonOutputStreamSerializer::binary(void* value, size_t size, std::string_view name) {
  std::string hex = common::toHex(value, size);
  return (*this)(hex, name);
}

bool JsonOutputStreamSerializer::binary(std::string& value, std::string_view name) {
  return binary(const_cast<char*>(value.data()), value.size(), name);
}
