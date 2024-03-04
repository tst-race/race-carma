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
#include <kestrel/vrf_shell_t.hpp>
// Include twice to test idempotence.
#include <kestrel/vrf_shell_t.hpp>
//

#include <sst/catalog/SST_UNREACHABLE.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/vrf_algorand_t.hpp>

namespace kestrel {

void to_json(json_t & dst, vrf_shell_t const & src) {

  dst = json_t::object();

  {
    vrf_algorand_t const * const core =
        dynamic_cast<vrf_algorand_t const *>(src.core_.get());
    if (core != nullptr) {
      dst["name"] = "algorand";
      dst["params"] = core->params();
      return;
    }
  }

  SST_UNREACHABLE();

} //

} // namespace kestrel
