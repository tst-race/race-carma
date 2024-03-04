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

#include <memory>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_entry_t::clear_mc_leaders() const {
  SST_ASSERT((!moved_from_));
  if (global().is_rigid()) {
    mc_leaders_.store(nullptr);
  }
}

void phonebook_entry_t::parse_mc_leaders(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (mc_leaders_.load()) {
    return;
  }
  parse_role(src);
  if (role() != role_t::mb_server()) {
    return;
  }
  parse_phonebook_set(src,
                      "mc_leaders",
                      mc_leaders_,
                      dynamic_cast<local_config_t *>(this)
                          && !global().is_rigid());
}

void phonebook_entry_t::unparse_mc_leaders(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  unparse_phonebook_set(dst, "mc_leaders", mc_leaders_);
}

phonebook_set_t const &
phonebook_entry_t::mc_leaders(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() == role_t::mb_server()));
  phonebook_set_t * val = mc_leaders_.load();
  if (!val) {
    SST_ASSERT((global().is_rigid()));
    sst::unique_ptr<phonebook_set_t> new_val{sst::in_place};
    for (phonebook_pair_t const & pair : phonebook()) {
      std::shared_ptr<phonebook_entry_t const> const entry =
          phonebook().at(SST_TEV_ARG(tev), pair);
      if (entry->role() == role_t::mc_leader()) {
        new_val->emplace(&pair);
      }
    }
    mc_leaders_.compare_exchange_strong(val, std::move(new_val));
  }
  return *val;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
