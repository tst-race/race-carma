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
#include <kestrel/psn_hash_1_t.hpp>
// Include twice to test idempotence.
#include <kestrel/psn_hash_1_t.hpp>
//

#include <algorithm>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bytes.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/sha256_t.hpp>
#include <sst/catalog/size.hpp>
#include <sst/catalog/unsigned_gt.hpp>
#include <sst/catalog/unsigned_min.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/corruption_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/serialization.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// Construction from a PSN
//----------------------------------------------------------------------

psn_hash_1_t::psn_hash_1_t(psn_t const & psn) {
  static unsigned char const salt[] = {
      static_cast<unsigned char>('k'),
      static_cast<unsigned char>('e'),
      static_cast<unsigned char>('s'),
      static_cast<unsigned char>('t'),
      static_cast<unsigned char>('r'),
      static_cast<unsigned char>('e'),
      static_cast<unsigned char>('l'),
  };
  auto const h =
      sst::sha256_t()
          .init()
          .update(sst::bytes(psn.value()), sst::size(psn.value()))
          .update(sst::bytes(salt), sst::size(salt))
          .finish()
          .output();
  auto const n =
      sst::unsigned_min<decltype(sst::size(h_))>(sst::size(h_) - 1,
                                                 sst::size(h));
  h_[0] = static_cast<unsigned char>(0x0F);
  std::copy_n(sst::bytes(h), n, sst::bytes(h_) + 1);
  std::fill_n(sst::bytes(h_) + 1 + n,
              sst::size(h_) - 1 - n,
              static_cast<unsigned char>(0));
}

//----------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------

serialize_size_t serialize_size(psn_hash_1_t const & src) {
  auto const a = src.h_.size();
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

bytes_t & serialize(bytes_t & dst, psn_hash_1_t const & src) {
  bytes_t::size_type const old_dst_size = dst.size();
  dst.insert(dst.end(), src.h_.begin(), src.h_.end());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

bool psn_hash_1_t::try_deserialize(bytes_t const & src,
                                   bytes_t::size_type & idx) {
  psn_hash_1_t & dst = *this;
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  auto const n = dst.h_.size();
  if (sst::unsigned_gt(n, src.size() - idx)) {
    return false;
  }
  if (src[idx] != static_cast<unsigned char>(0x0F)) {
    return false;
  }
  std::copy_n(&src[idx], n, &dst.h_[0]);
  idx += static_cast<bytes_t::size_type>(n);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
  return true;
}

void deserialize(bytes_t const & src,
                 bytes_t::size_type & idx,
                 psn_hash_1_t & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  if (!dst.try_deserialize(src, idx)) {
    throw corruption_t();
  }
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------

} // namespace kestrel
