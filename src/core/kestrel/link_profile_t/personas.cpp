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
#include <kestrel/link_profile_t.hpp>
// Include twice to test idempotence.
#include <kestrel/link_profile_t.hpp>
//

#include <unordered_set>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

std::unordered_set<psn_t> const &
link_profile_t::parse_personas(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (!have_personas_) {
    sst::json::remove_to(src, personas_, "personas");
    for (psn_t const & persona : personas_) {
      personas_vector_.push_back(persona.string());
    }
    have_personas_ = true;
  }
  return personas_;
}

} // namespace kestrel
