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

#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void to_json(json_t & dst, global_config_t const & src) {

  SST_ASSERT((!src.moved_from_));

  json_t d = json_t::object();

  d["correctness_test_only"] = src.correctness_test_only();

  d["corruption_rate"] = src.corruption_rate();

  d["epoch_nonce"] = sst::to_hex(src.epoch_nonce());

  d["num_clients"] = sst::to_string(src.num_clients());

  src.unparse_num_servers(d);

  //--------------------------------------------------------------------
  // prime
  //--------------------------------------------------------------------

  d["prime"] = sst::to_string(src.prime());

  //--------------------------------------------------------------------

  d["privacy_failure_rate"] = src.privacy_failure_rate();

  d["robustness_failure_rate"] = src.robustness_failure_rate();

  src.unparse_shamir_threshold(d);

  d["vrf"] = src.vrf();

  dst = std::move(d);
}

} // namespace carma
} // namespace kestrel
