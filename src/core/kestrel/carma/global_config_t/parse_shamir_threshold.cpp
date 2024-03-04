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

#include <exception>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/remove_to.hpp>

#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void global_config_t::parse_shamir_threshold(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (shamir_threshold_) {
    return;
  }
  node_count_t x{};
  sst::json::remove_to(src, x, "shamir_threshold");
  if (x < 1U) {
    try {
      throw sst::json::exception::invalid_value();
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("shamir_threshold"));
    }
  }
  shamir_threshold_.emplace(x);
}

} // namespace carma
} // namespace kestrel
