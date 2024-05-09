// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <Common/IInputStream.h>
#include "ISerializer.h"
#include "JsonInputValueSerializer.h"

namespace cn {

class KVBinaryInputStreamSerializer : public JsonInputValueSerializer {
public:
  explicit KVBinaryInputStreamSerializer(common::IInputStream& strm);

  bool binary(uint8_t* value, size_t size, std::string_view name) override;
  bool binary(std::string& value, std::string_view name) override;
};

}
