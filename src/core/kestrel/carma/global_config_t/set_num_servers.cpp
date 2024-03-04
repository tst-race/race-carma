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
#include <sst/catalog/to_string.hpp>

#include <kestrel/carma/node_count_t.hpp>

namespace kestrel {
namespace carma {

void global_config_t::set_num_servers(node_count_t const src) {
  SST_ASSERT((!moved_from_));
  if (src < min_servers()) {
    throw std::runtime_error("Invalid num_servers: "
                             + sst::to_string(src));
  }
  num_servers_ = src;
}

} // namespace carma
} // namespace kestrel
