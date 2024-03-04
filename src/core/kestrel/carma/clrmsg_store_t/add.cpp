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
#include <kestrel/carma/clrmsg_store_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/clrmsg_store_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <kestrel/carma/clrmsg_store_entry_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

clrmsg_store_entry_t & clrmsg_store_t::add(tracing_event_t tev,
                                           clrmsg_store_entry_t entry) {
  SST_TEV_ADD(tev);
  try {
    SST_ASSERT((!moved_from_));
    entries_->emplace_back(std::move(entry));
    return entries_->back();
  }
  SST_TEV_RETHROW(tev);
}

} // namespace carma
} // namespace kestrel
