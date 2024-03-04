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
#include <kestrel/psn_any_hash_t.hpp>
// Include twice to test idempotence.
#include <kestrel/psn_any_hash_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_UNREACHABLE.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/corruption_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------

bytes_t & serialize(bytes_t & dst, psn_any_hash_t const & src) {
  switch (src.type) {
    case 1:
      return serialize(dst, src.hash_1);
  }
  SST_UNREACHABLE();
}

void deserialize(bytes_t const & src,
                 bytes_t::size_type & idx,
                 psn_any_hash_t & dst) {
  SST_ASSERT((idx < src.size()));

  dst.type = 1;
  dst.hash_1 = psn_hash_1_t();
  if (dst.hash_1.try_deserialize(src, idx)) {
    return;
  }

  throw corruption_t();
}

//----------------------------------------------------------------------

} // namespace kestrel
