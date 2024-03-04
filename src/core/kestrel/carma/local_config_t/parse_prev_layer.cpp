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

#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::parse_prev_layer(json_t & src) {
  SST_ASSERT((!moved_from_));
  parse_role(src);
  if (role() == role_t::rs_server() || role() == role_t::mb_server()) {
    parse_phonebook_set(src,
                        "prev_layer",
                        prev_layer_,
                        !global().is_rigid());
  }
}

} // namespace carma
} // namespace kestrel
