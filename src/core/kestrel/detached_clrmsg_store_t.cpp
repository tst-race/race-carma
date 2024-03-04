//
// Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS
// IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

// Include first to test independence.
#include <kestrel/detached_clrmsg_store_t.hpp>
// Include twice to test idempotence.
#include <kestrel/detached_clrmsg_store_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

detached_clrmsg_store_t::detached_clrmsg_store_t(
    tracing_event_t tev,
    std::string dir,
    common_sdk_t * const sdk)
    : dir_(std::move(dir)),
      sdk_(sdk) {
  SST_TEV_TOP(tev);
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------
// add
//----------------------------------------------------------------------

void detached_clrmsg_store_t::add(tracing_event_t tev,
                                  sst::unique_ptr<entry_t> && entry) {
  SST_TEV_TOP(tev);
  SST_ASSERT((entry));
  auto & list = map_[entry->clrmsg.recver()];
  list.emplace_back();
  list.back() = std::move(entry);
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------
// get
//----------------------------------------------------------------------

sst::unique_ptr<detached_clrmsg_store_t::entry_t> *
detached_clrmsg_store_t::get(tracing_event_t tev, psn_t const & psn) {
  SST_TEV_TOP(tev);
  auto & list = map_[psn];
  while (!list.empty() && list.front() == nullptr) {
    list.pop_front();
  }
  if (list.empty()) {
    return nullptr;
  }
  return &list.front();
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------
// flush
//----------------------------------------------------------------------

void detached_clrmsg_store_t::flush(tracing_event_t tev) {
  SST_TEV_TOP(tev);
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

} // namespace kestrel
