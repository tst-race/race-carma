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

#ifndef KESTREL_MB_CLIENT_PACKET_T_HPP
#define KESTREL_MB_CLIENT_PACKET_T_HPP

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
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/old/from_bytes.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/old/to_bytes.hpp>
#include <vector>

namespace kestrel {

struct mb_client_packet_t final : origin_handle_t, origin_span_t {
  packet_type_t type = packet_type_t::mb_client_packet();
  bytes_t a;
  guid_t oid;

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;

    SST_ASSERT((type == packet_type_t::mb_client_packet()));
    n += type.to_bytes_size<Size>();

    n += 4;
    n += a.size();
    n += oid.to_bytes_size<Size>();
    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    SST_ASSERT((type == packet_type_t::mb_client_packet()));
    dst = type.to_bytes(dst);
    dst = sst::old::to_bytes(a.size(),
                        dst,
                        4,
                        sst::integer_rep::pure_unsigned());
    dst = sst::copy_bytes(a.begin(), a.size(), dst);
    dst = oid.to_bytes(dst);
    return dst;
  }

  void from_bytes_prep(race_handle_t const & origin_handle,
                       sdk_span_t const & origin_span) {
    set_origin_handle(origin_handle);
    set_origin_span(origin_span);
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT(sst::perfect_ge(avail, 0));

    // Why assert? Because the caller should have already parsed and
    // verified the packet type. This function is not intended to be
    // used to probe for the packet type.
    src = type.from_bytes(src, avail);
    SST_ASSERT((type == packet_type_t::mb_client_packet()));

    a.resize(sst::old::from_bytes<decltype(a.size())>(
        &src,
        4,
        avail,
        sst::integer_rep::pure_unsigned()));
    sst::copy_bytes(&src, a.size(), avail, a.begin());
    src = oid.from_bytes(src, avail);
    return src;
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_MB_CLIENT_PACKET_T_HPP
