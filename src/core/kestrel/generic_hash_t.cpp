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
#include <kestrel/generic_hash_t.hpp>
// Include twice to test idempotence.
#include <kestrel/generic_hash_t.hpp>

#include <array>
#include <kestrel/sodium.hpp>
#include <cstddef>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/to_string.hpp>
#include <stdexcept>

namespace kestrel {

generic_hash_t::generic_hash_t() {
  auto const s = sodium::crypto_generichash_init(&state_,
                                                 nullptr,
                                                 0,
                                                 hash_.size());
  if (s != 0) {
    throw std::runtime_error(
        "crypto_generichash_init() failed (return value: "
        + sst::to_string(s) + ")");
  }
}

template void generic_hash_t::update(unsigned char const * x,
                                     std::size_t n);

std::array<unsigned char, crypto_generichash_BYTES> const &
generic_hash_t::finish() {
  if (updating_) {
    auto const s = sodium::crypto_generichash_final(&state_,
                                                    hash_.data(),
                                                    hash_.size());
    if (s != 0) {
      throw std::runtime_error(
          "crypto_generichash_final() failed (return value: "
          + sst::to_string(s) + ")");
    }
    updating_ = false;
  }
  return hash_;
}

} // namespace kestrel
