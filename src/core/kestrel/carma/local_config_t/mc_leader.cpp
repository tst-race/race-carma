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
#include <kestrel/carma/local_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/local_config_t.hpp>
//

#include <exception>
#include <memory>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/config_exception.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::clear_mc_leader() const {
  SST_ASSERT((!moved_from_));
  mc_leader_.store(nullptr);
}

void local_config_t::parse_mc_leader(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (mc_leader_.load()) {
    return;
  }
  parse_role(src);
  if (role() != role_t::mc_follower()) {
    return;
  }
  auto const it = src.find("mc_leader");
  if (it == src.end()) {
    return;
  }
  try {
    mc_leader_.store(sst::unique_ptr<phonebook_pair_t const *>(
        sst::in_place,
        &phonebook().expect<sst::json::exception>(
            sst::json::get_as<psn_t>(*it))));
    src.erase(it);
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("mc_leader"));
  }
}

void local_config_t::unparse_mc_leader(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  phonebook_pair_t const * const * const val = mc_leader_.load();
  if (val) {
    SST_ASSERT((role() == role_t::mc_follower()));
    dst["mc_leader"] = (*val)->first;
  }
}

phonebook_pair_t const &
local_config_t::mc_leader(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() == role_t::mc_follower()));
  phonebook_pair_t const ** val = mc_leader_.load();
  if (!val) {
    sst::unique_ptr<phonebook_pair_t const *> new_val;
    node_count_t const x = group();
    for (phonebook_pair_t const & pair : phonebook()) {
      std::shared_ptr<phonebook_entry_t const> const entry =
          phonebook().at(SST_TEV_ARG(tev), pair);
      if (entry->role() == role_t::mc_leader() && entry->group() == x) {
        new_val.emplace(&pair);
        break;
      }
    }
    if (!new_val) {
      throw config_exception("MC group " + sst::to_string(x)
                             + " is missing an MC leader");
    }
    mc_leader_.compare_exchange_strong(val, std::move(new_val));
  }
  return **val;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
