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
#include <kestrel/vrf_algorand_t.hpp>
// Include twice to test idempotence.
#include <kestrel/vrf_algorand_t.hpp>
//

#include <cstddef>
#include <cstring>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked_cast.hpp>

#include <kestrel/sodium.hpp>

namespace kestrel {

bool vrf_algorand_t::verify(unsigned char const * const pk,
                            unsigned char const * const input,
                            std::size_t const size,
                            unsigned char const * const proof,
                            unsigned char const * const output) const {
  SST_ASSERT((pk != nullptr));
  SST_ASSERT((input != nullptr));
  SST_ASSERT((proof != nullptr));
  SST_ASSERT((output != nullptr));
  unsigned char output2[crypto_vrf_OUTPUTBYTES];
  int s = sodium::crypto_vrf_verify(
      output2,
      pk,
      proof,
      input,
      sst::checked_cast<unsigned long long>(size));
  return s == 0
         && std::memcmp(
                output,
                output2,
                static_cast<std::size_t>(crypto_vrf_OUTPUTBYTES))
                == 0;
}

} // namespace kestrel
