// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <list>
#include <memory>

namespace cn {

class P2pContext;

class P2pContextOwner {
public:

  using ContextList = std::list<std::unique_ptr<P2pContext>>;

  P2pContextOwner(P2pContext* ctx, ContextList& contextList);
  P2pContextOwner(P2pContextOwner&& other) noexcept;
  P2pContextOwner(const P2pContextOwner& other) = delete;
  ~P2pContextOwner();

  P2pContext& get();
  P2pContext* operator -> ();

private:

  ContextList& contextList;
  ContextList::iterator contextIterator;
};

}
