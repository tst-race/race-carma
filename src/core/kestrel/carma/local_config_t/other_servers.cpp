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

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::parse_other_servers(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (other_servers_.load()) {
    return;
  }
  parse_role(src);
  if (role() == role_t::client()) {
    return;
  }
  parse_phonebook_set(src, "other_servers", other_servers_, true);
}

void local_config_t::unparse_other_servers(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (role() != role_t::client()) {
    unparse_phonebook_set(dst, "other_servers", other_servers());
  }
}

phonebook_set_t const & local_config_t::other_servers() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() != role_t::client()));
  phonebook_set_t const * const val = other_servers_.load();
  SST_ASSERT((val != nullptr));
  return *val;
}

phonebook_set_t & local_config_t::mutable_other_servers() {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((role() != role_t::client()));
  phonebook_set_t * val = other_servers_.load();
  if (!val) {
    val = other_servers_.store(
        sst::unique_ptr<phonebook_set_t>(sst::in_place));
  }
  return *val;
}

} // namespace carma
} // namespace kestrel
