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

#include <stdexcept>

#include <sst/catalog/SST_ASSERT.h>

namespace kestrel {
namespace carma {

void global_config_t::validate() const {
  SST_ASSERT((!moved_from_));
  if (num_servers() - 2 < shamir_threshold()) {
    throw std::runtime_error(
        "num_servers must be at least shamir_threshold + 2");
  }
  if (!correctness_test_only() && !default_rate() && !exotic_rate()) {
    throw std::runtime_error("default_rate or exotic_rate must be true "
                             "when correctness_test_only = false");
  }
}

} // namespace carma
} // namespace kestrel
