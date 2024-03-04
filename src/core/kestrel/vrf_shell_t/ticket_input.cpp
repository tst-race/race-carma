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

#include <vector>

#include <sst/catalog/SST_ASSERT.h>

namespace kestrel {

std::vector<unsigned char> vrf_shell_t::ticket_input(
    std::vector<unsigned char> const & epoch_nonce) {
  std::vector<unsigned char> v;
  v.push_back(static_cast<unsigned char>(0x1));
  v.insert(v.end(), epoch_nonce.begin(), epoch_nonce.end());
  return v;
}

} // namespace kestrel
