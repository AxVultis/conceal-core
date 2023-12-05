// Copyright (c) 2012-2018, The CryptoNote developers, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <CryptoNote.h>

namespace common
{

    template <class It>
    inline cn::BinaryArray::iterator append(cn::BinaryArray &ba, It be, It en)
    {
        return ba.insert(ba.end(), be, en);
    }
    inline cn::BinaryArray::iterator append(cn::BinaryArray &ba, size_t add, cn::BinaryArray::value_type va)
    {
        return ba.insert(ba.end(), add, va);
    }
    inline cn::BinaryArray::iterator append(cn::BinaryArray &ba, const cn::BinaryArray &other)
    {
        return ba.insert(ba.end(), other.begin(), other.end());
    }

} // namespace common