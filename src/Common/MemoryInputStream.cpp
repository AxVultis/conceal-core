// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MemoryInputStream.h"
#include <algorithm>
#include <cassert>
#include <cstring> // memcpy

namespace common {

MemoryInputStream::MemoryInputStream(const char* buffer, size_t bufferSize) : 
buffer(buffer), bufferSize(bufferSize) {}

MemoryInputStream::MemoryInputStream(const uint8_t* buffer, size_t bufferSize) : 
buffer(reinterpret_cast<const char*>(buffer)), bufferSize(bufferSize) {}

size_t MemoryInputStream::getPosition() const {
  return position;
}

bool MemoryInputStream::endOfStream() const {
  return position == bufferSize;
}

size_t MemoryInputStream::readSome(uint8_t* data, size_t size) {
  assert(position <= bufferSize);
  size_t readSize = std::min(size, bufferSize - position);

  if (readSize > 0) {
    memcpy(data, buffer + position, readSize);
    position += readSize;
  }
  
  return readSize;
}

}
