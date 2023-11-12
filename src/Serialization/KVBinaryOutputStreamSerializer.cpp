// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KVBinaryOutputStreamSerializer.h"
#include "KVBinaryCommon.h"
#include <limits>
#include <cassert>
#include <stdexcept>
#include <Common/StreamTools.h>

using namespace common;
using namespace cn;

namespace {

template <typename T>
void writePod(IOutputStream& s, const T& value) {
  write(s, &value, sizeof(T));
}

template<class T>
size_t packVarint(IOutputStream& s, uint8_t type_or, size_t pv) {
  auto v = static_cast<T>(pv << 2);
  v |= type_or;
  write(s, &v, sizeof(T));
  return sizeof(T);
}

void writeElementName(IOutputStream& s, std::string_view name) {
  if (name.size() > std::numeric_limits<uint8_t>::max()) {
    throw std::runtime_error("Element name is too long");
  }

  auto len = static_cast<uint8_t>(name.size());
  write(s, &len, sizeof(len));
  write(s, name.data(), len);
}

size_t writeArraySize(IOutputStream& s, size_t val) {
  if (val <= 63) {
    return packVarint<uint8_t>(s, PORTABLE_RAW_SIZE_MARK_BYTE, val);
  } else if (val <= 16383) {
    return packVarint<uint16_t>(s, PORTABLE_RAW_SIZE_MARK_WORD, val);
  } else if (val <= 1073741823) {
    return packVarint<uint32_t>(s, PORTABLE_RAW_SIZE_MARK_DWORD, val);
  } else {
    if (val > 4611686018427387903) {
      throw std::runtime_error("failed to pack varint - too big amount");
    }
    return packVarint<uint64_t>(s, PORTABLE_RAW_SIZE_MARK_INT64, val);
  }
}

}

namespace cn {

KVBinaryOutputStreamSerializer::KVBinaryOutputStreamSerializer() {
  beginObject(std::string());
}

void KVBinaryOutputStreamSerializer::dump(IOutputStream& target) {
  assert(m_objectsStack.size() == 1);
  assert(m_stack.size() == 1);

  KVBinaryStorageBlockHeader hdr;
  hdr.m_signature_a = PORTABLE_STORAGE_SIGNATUREA;
  hdr.m_signature_b = PORTABLE_STORAGE_SIGNATUREB;
  hdr.m_ver = PORTABLE_STORAGE_FORMAT_VER;

  common::write(target, &hdr, sizeof(hdr));
  writeArraySize(target, m_stack.front().count);
  write(target, stream().data(), stream().size());
}

ISerializer::SerializerType KVBinaryOutputStreamSerializer::type() const {
  return ISerializer::SerializerType::OUTPUT;
}

bool KVBinaryOutputStreamSerializer::beginObject(std::string_view name) {
  checkArrayPreamble(BIN_KV_SERIALIZE_TYPE_OBJECT);
 
  m_stack.emplace_back(name);
  m_objectsStack.emplace_back();

  return true;
}

void KVBinaryOutputStreamSerializer::endObject() {
  assert(m_objectsStack.size());

  auto level = std::move(m_stack.back());
  m_stack.pop_back();

  auto objStream = std::move(m_objectsStack.back());
  m_objectsStack.pop_back();

  auto& out = stream();

  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_OBJECT, level.name);

  writeArraySize(out, level.count);
  write(out, objStream.data(), objStream.size());
}

bool KVBinaryOutputStreamSerializer::beginArray(size_t& size, std::string_view name) {
  m_stack.emplace_back(name, size);
  return true;
}

void KVBinaryOutputStreamSerializer::endArray() {
  bool validArray = m_stack.back().state == State::Array;
  m_stack.pop_back();

  if (m_stack.back().state == State::Object && validArray) {
    ++m_stack.back().count;
  }
}

bool KVBinaryOutputStreamSerializer::operator()(uint8_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_UINT8, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(uint16_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_UINT16, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(int16_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_INT16, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(uint32_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_UINT32, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(int32_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_INT32, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(int64_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_INT64, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(uint64_t& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_UINT64, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(bool& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_BOOL, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(double& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_DOUBLE, name);
  writePod(stream(), value);
  return true;
}

bool KVBinaryOutputStreamSerializer::operator()(std::string& value, std::string_view name) {
  writeElementPrefix(BIN_KV_SERIALIZE_TYPE_STRING, name);

  auto& out = stream();
  writeArraySize(out, value.size());
  write(out, value.data(), value.size());
  return true;
}

bool KVBinaryOutputStreamSerializer::binary(void* value, size_t size, std::string_view name) {
  if (size > 0) {
    writeElementPrefix(BIN_KV_SERIALIZE_TYPE_STRING, name);
    auto& out = stream();
    writeArraySize(out, size);
    write(out, value, size);
  }
  return true;
}

bool KVBinaryOutputStreamSerializer::binary(std::string& value, std::string_view name) {
  return binary(const_cast<char*>(value.data()), value.size(), name);
}

void KVBinaryOutputStreamSerializer::writeElementPrefix(uint8_t type, std::string_view name) {  
  assert(m_stack.size());

  checkArrayPreamble(type);
  Level& level = m_stack.back();
  
  if (level.state != State::Array) {
    if (!name.empty()) {
      auto& s = stream();
      writeElementName(s, name);
      write(s, &type, 1);
    }
    ++level.count;
  }
}

void KVBinaryOutputStreamSerializer::checkArrayPreamble(uint8_t type) {
  if (m_stack.empty()) {
    return;
  }

  Level& level = m_stack.back();

  if (level.state == State::ArrayPrefix) {
    auto& s = stream();
    writeElementName(s, level.name);
    char c = BIN_KV_SERIALIZE_FLAG_ARRAY | type;
    write(s, &c, 1);
    writeArraySize(s, level.count);
    level.state = State::Array;
  }
}


MemoryStream& KVBinaryOutputStreamSerializer::stream() {
  assert(m_objectsStack.size());
  return m_objectsStack.back();
}

}
