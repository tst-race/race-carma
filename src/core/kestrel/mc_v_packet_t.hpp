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

#ifndef KESTREL_MC_V_PACKET_T_HPP
#define KESTREL_MC_V_PACKET_T_HPP

#include <kestrel/old_config_t.hpp>
#include <kestrel/guid_t.hpp>
#include <kestrel/origin_handle_t.hpp>
#include <kestrel/origin_span_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/prime_size_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_span_t.hpp>
#include <cstddef>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <vector>

namespace kestrel {

struct mc_v_packet_t final : origin_handle_t, origin_span_t {
  packet_type_t type = packet_type_t::mc_v_packet();
  prime_size_t prime_size;
  decltype(std::declval<old_config_t>().mixsize) mixsize;

  guid_t mpcid;
  std::vector<sst::bigint> v;

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;

    SST_ASSERT((type == packet_type_t::mc_v_packet()));
    n += type.to_bytes_size<Size>();

    n += mpcid.to_bytes_size<Size>();
    n += sst::checked(prime_size) * v.size();
    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    SST_ASSERT((type == packet_type_t::mc_v_packet()));
    dst = type.to_bytes(dst);
    dst = mpcid.to_bytes(dst);
    for (decltype(v.size()) i = 0; i != v.size(); ++i) {
      dst = v[i].to_bytes(dst,
                          prime_size,
                          sst::integer_rep::pure_unsigned());
    }
    return dst;
  }

  void from_bytes_prep(race_handle_t const & origin_handle,
                       sdk_span_t const & origin_span,
                       decltype(prime_size) const a_prime_size,
                       decltype(mixsize) const a_mixsize) {
    set_origin_handle(origin_handle);
    set_origin_span(origin_span);
    prime_size = a_prime_size;
    mixsize = a_mixsize;
    sst::checked_resize(v, mixsize);
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT(sst::perfect_ge(avail, 0));

    // Why assert? Because the caller should have already parsed and
    // verified the packet type. This function is not intended to be
    // used to probe for the packet type.
    src = type.from_bytes(src, avail);
    SST_ASSERT((type == packet_type_t::mc_v_packet()));

    src = mpcid.from_bytes(src, avail);
    for (decltype(v.size()) i = 0; i != v.size(); ++i) {
      src = v[i].set_from_bytes(src,
                                prime_size,
                                avail,
                                sst::integer_rep::pure_unsigned());
    }
    return src;
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_MC_V_PACKET_T_HPP
