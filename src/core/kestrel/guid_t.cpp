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
#include <kestrel/guid_t.hpp>
// Include twice to test idempotence.
#include <kestrel/guid_t.hpp>

#include <cstddef>
#include <sst/catalog/crypto_rng.hpp>

namespace kestrel {

guid_t guid_t::generate() {
  guid_t guid;
  sst::crypto_rng()(guid.value().begin(), guid.value().size());
  return guid;
}

template std::size_t guid_t::to_bytes_size<std::size_t>() const;

} // namespace kestrel
