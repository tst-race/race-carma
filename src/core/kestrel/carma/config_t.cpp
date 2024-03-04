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
#include <kestrel/carma/config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/config_t.hpp>
//

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/detached_clrmsg_store_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------
// detached_clrmsg_store
//----------------------------------------------------------------------

detached_clrmsg_store_t &
config_t::detached_clrmsg_store(tracing_event_t tev) {
  SST_TEV_TOP(tev);
  if (!detached_clrmsg_store_) {
    detached_clrmsg_store_.emplace(SST_TEV_ARG(tev),
                                   dir_ + "/detached_clrmsg_store",
                                   sdk_);
  }
  return *detached_clrmsg_store_;
  SST_TEV_BOT(tev);
}

detached_clrmsg_store_t const &
config_t::detached_clrmsg_store(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  return const_cast<config_t &>(*this).detached_clrmsg_store(
      SST_TEV_ARG(tev));
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
