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
#include <stdexcept>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/overlap.hpp>

#include <kestrel/sodium.hpp>

namespace kestrel {

void vrf_algorand_t::eval(unsigned char const * const sk,
                          unsigned char const * const input,
                          std::size_t const size,
                          unsigned char * const proof,
                          unsigned char * const output) const {
  SST_ASSERT((sk != nullptr));
  SST_ASSERT((input != nullptr));
  SST_ASSERT((proof != nullptr));
  SST_ASSERT((output != nullptr));
  SST_ASSERT((!sst::overlap(sk, sk_size(), proof, proof_size())));
  SST_ASSERT((!sst::overlap(sk, sk_size(), output, output_size())));
  SST_ASSERT((!sst::overlap(input, size, proof, proof_size())));
  SST_ASSERT((!sst::overlap(input, size, output, output_size())));
  SST_ASSERT(
      (!sst::overlap(proof, proof_size(), output, output_size())));
  {
    int const s = sodium::crypto_vrf_prove(
        proof,
        sk,
        input,
        sst::checked_cast<unsigned long long>(size));
    if (s != 0) {
      throw std::runtime_error("crypto_vrf_prove() failed");
    }
  }
  {
    int const s = sodium::crypto_vrf_proof_to_hash(output, proof);
    if (s != 0) {
      throw std::runtime_error("crypto_vrf_proof_to_hash() failed");
    }
  }
}

} // namespace kestrel
