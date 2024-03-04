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
#include <kestrel/carma/bootstrap_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/bootstrap_config_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/unknown_key.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void from_json(json_t src, bootstrap_config_t & dst) {
  SST_ASSERT((!dst.moved_from_));

  dst.parse_bootstrapper(src);
  dst.parse_channel_id(src);
  dst.parse_link_address(src);

  sst::json::unknown_key(src);

  dst.exists(true);
}

} // namespace carma
} // namespace kestrel
