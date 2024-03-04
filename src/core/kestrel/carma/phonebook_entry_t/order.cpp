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
#include <kestrel/carma/phonebook_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_entry_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_entry_t::parse_order(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (order_.load()) {
    return;
  }
  parse_role(src);
  if (role() != role_t::mc_leader()
      && role() != role_t::mc_follower()) {
    return;
  }
  node_count_t val;
  sst::json::remove_to(src, val, "order");
  order_.store(val);
}

void phonebook_entry_t::unparse_order(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  sst::optional<node_count_t> const val = order_.load();
  if (val) {
    SST_ASSERT((role() == role_t::mc_leader()
                || role() == role_t::mc_follower()));
    dst["order"] = sst::to_string(*val);
  }
}

node_count_t phonebook_entry_t::order() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() == role_t::mc_leader()
              || role() == role_t::mc_follower()));
  return *order_.load();
}

void phonebook_entry_t::set_order(node_count_t const src) {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() == role_t::mc_leader()
              || role() == role_t::mc_follower()));
  order_.store(src);
}

} // namespace carma
} // namespace kestrel
