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
#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unsigned_ge.hpp>

#include <kestrel/carma/rangegen/nonrigid_correctness_only.hpp>
#include <kestrel/carma/rangegen/nonrigid_default.hpp>
#include <kestrel/carma/rangegen/rigid_correctness_only.hpp>
#include <kestrel/carma/rangegen/rigid_default.hpp>
#include <kestrel/carma/rangegen/row.hpp>

namespace kestrel {
namespace carma {

// TODO: Support exotic rates. See the True_Genesis algorithm in the
//       "CARMA logics" document.

rangegen::row const & global_config_t::rangegen_row() const {
  SST_ASSERT((!moved_from_));
  auto const entries =
      is_rigid() ?
          (correctness_test_only() ?
               rangegen::rigid_correctness_only() :
           default_rate() ? rangegen::rigid_default() :
                            throw std::runtime_error(
                                "Exotic rates are not supported yet")) :
          (correctness_test_only() ?
               rangegen::nonrigid_correctness_only() :
           default_rate() ? rangegen::nonrigid_default() :
                            throw std::runtime_error(
                                "Exotic rates are not supported yet"));
  if (sst::unsigned_ge(num_servers(), entries.second)
      || entries.first[num_servers()].num_servers < 0) {
    throw std::runtime_error(
        std::string("Missing rangegen_row for is_rigid = ")
        + (is_rigid() ? "true" : "false") + ", correctness_test_only = "
        + (correctness_test_only() ? "true" : "false")
        + ", and num_servers = " + sst::to_string(num_servers()));
  }
  return entries.first[num_servers()];
}

} // namespace carma
} // namespace kestrel
