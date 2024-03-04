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

#ifndef KESTREL_SODIUM_HPP
#define KESTREL_SODIUM_HPP

#include <utility>

#include <sodium.h>

namespace kestrel {
namespace sodium {

void init_once();

#define KESTREL_SODIUM_FUNCTION(F)                                     \
  template<class... Args>                                              \
  auto F(Args &&... args)                                              \
      ->decltype(::F(std::forward<Args>(args)...)) {                   \
    init_once();                                                       \
    return ::F(std::forward<Args>(args)...);                           \
  }

KESTREL_SODIUM_FUNCTION(crypto_box_easy)
KESTREL_SODIUM_FUNCTION(crypto_box_keypair)
KESTREL_SODIUM_FUNCTION(crypto_box_open_easy)
KESTREL_SODIUM_FUNCTION(crypto_box_seal)
KESTREL_SODIUM_FUNCTION(crypto_box_seal_open)
KESTREL_SODIUM_FUNCTION(crypto_generichash_final)
KESTREL_SODIUM_FUNCTION(crypto_generichash_init)
KESTREL_SODIUM_FUNCTION(crypto_generichash_update)
KESTREL_SODIUM_FUNCTION(crypto_vrf_keypair)
KESTREL_SODIUM_FUNCTION(crypto_vrf_proof_to_hash)
KESTREL_SODIUM_FUNCTION(crypto_vrf_prove)
KESTREL_SODIUM_FUNCTION(crypto_vrf_verify)

#undef KESTREL_SODIUM_FUNCTION

} // namespace sodium
} // namespace kestrel

#endif // #ifndef KESTREL_SODIUM_HPP
