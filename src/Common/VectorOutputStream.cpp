// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VectorOutputStream.h"

namespace common {

VectorOutputStream::VectorOutputStream(std::vector<uint8_t>& out) : out(out) {
}

size_t VectorOutputStream::writeSome(const uint8_t* data, size_t size) {
  out.insert(out.end(), data, data + size);
  return size;
}

}
