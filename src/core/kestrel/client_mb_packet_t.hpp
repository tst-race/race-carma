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

#ifndef KESTREL_CLIENT_MB_PACKET_T_HPP
#define KESTREL_CLIENT_MB_PACKET_T_HPP

#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/guid_t.hpp>
#include <kestrel/origin_handle_t.hpp>
#include <kestrel/origin_span_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/prime_size_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_span_t.hpp>
#include <cstddef>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/from_varint.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/to_varint.hpp>
#include <sst/catalog/track_it.hpp>
#include <sst/catalog/varint_size.hpp>

namespace kestrel {

struct client_mb_packet_t final : origin_handle_t, origin_span_t {
  prime_size_t prime_size;

  packet_type_t type = packet_type_t::client_mb_packet();
  guid_t cid;
  sst::bigint x;
  kestrel::carma::node_count_t mc_group_number{};

  //--------------------------------------------------------------------

  nlohmann::json to_json() {
    return {
        {"x", x.to_string()},
    };
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;

    SST_ASSERT((type == packet_type_t::client_mb_packet()));
    n += type.to_bytes_size<Size>();

    n += cid.to_bytes_size<Size>();
    n += prime_size;

    n += sst::varint_size(mc_group_number);

    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    SST_ASSERT((type == packet_type_t::client_mb_packet()));
    dst = type.to_bytes(dst);
    dst = cid.to_bytes(dst);
    dst = x.to_bytes(dst, prime_size, sst::integer_rep::pure_unsigned());
    dst = sst::to_varint(mc_group_number, dst);
    return dst;
  }

  void from_bytes_prep(race_handle_t const & origin_handle,
                       sdk_span_t const & origin_span,
                       decltype(prime_size) const a_prime_size) {
    set_origin_handle(origin_handle);
    set_origin_span(origin_span);
    prime_size = a_prime_size;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT(sst::perfect_ge(avail, 0));

    // Why assert? Because the caller should have already parsed and
    // verified the packet type. This function is not intended to be
    // used to probe for the packet type.
    src = type.from_bytes(src, avail);
    SST_ASSERT((type == packet_type_t::client_mb_packet()));

    src = cid.from_bytes(src, avail);
    src = x.set_from_bytes(src,
                           prime_size,
                           avail,
                           sst::integer_rep::pure_unsigned());

    mc_group_number =
        sst::from_varint<kestrel::carma::node_count_t>(sst::track_it(src, &avail));

    return src;
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_CLIENT_MB_PACKET_T_HPP
