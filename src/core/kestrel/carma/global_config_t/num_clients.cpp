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
#include <kestrel/carma/global_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/global_config_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void global_config_t::parse_num_clients(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (!have_num_clients_) {
    sst::json::remove_to(src, num_clients_, "num_clients");
    have_num_clients_ = true;
  }
}

long long global_config_t::num_clients() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((have_num_clients_));
  return num_clients_;
}

global_config_t & global_config_t::num_clients(long long const src) {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((src > 0));
  num_clients_ = src;
  have_num_clients_ = true;
  return *this;
}

} // namespace carma
} // namespace kestrel
