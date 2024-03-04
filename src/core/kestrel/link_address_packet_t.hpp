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

#ifndef KESTREL_LINK_ADDRESS_PACKET_T_HPP
#define KESTREL_LINK_ADDRESS_PACKET_T_HPP

// Standard C++ headers
#include <cstddef>

// SST headers
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/perfect_ge.hpp>

// CARMA headers
#include <kestrel/channel_id_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/origin_handle_t.hpp>
#include <kestrel/origin_span_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_span_t.hpp>

namespace kestrel {

class link_address_packet_t final : origin_handle_t, origin_span_t {

public:
  packet_type_t type = packet_type_t::link_address_packet();
  channel_id_t channel_id;
  link_address_t link_address;

  //--------------------------------------------------------------------
  // Initialization and cleanup
  //--------------------------------------------------------------------
  //
  // Objects of this type are intended to be acquired from a pool and
  // released back to reduce memory churn. The init and dnit functions
  // should be called just after an object of this type is acquired and
  // just before it is released, respectively.
  //

  void clear() {
    channel_id.clear();
    link_address.clear();
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size = std::size_t>
  Size to_bytes_size() const {
    SST_ASSERT((type == packet_type_t::link_address_packet()));

    sst::checked_t<Size> n = 0;

    n += type.to_bytes_size<Size>();

    n += channel_id.to_bytes_size<Size>();

    n += link_address.to_bytes_size<Size>();

    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    SST_ASSERT((type == packet_type_t::link_address_packet()));

    dst = type.to_bytes(dst);

    dst = channel_id.to_bytes(dst);

    dst = link_address.to_bytes(dst);

    return dst;
  }

  void from_bytes_prep() {
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT((sst::perfect_ge(avail, 0)));

    // Why assert? Because the caller should have already parsed and
    // verified the packet type. This function is not intended to be
    // used to probe for the packet type.
    src = type.from_bytes(src, avail);
    SST_ASSERT((type == packet_type_t::link_address_packet()));

    src = channel_id.from_bytes(src, avail);

    src = link_address.from_bytes(src, avail);

    return src;
  }

  //--------------------------------------------------------------------
};

extern template std::size_t
link_address_packet_t::to_bytes_size<std::size_t>() const;

} // namespace kestrel

#endif // #ifndef KESTREL_LINK_ADDRESS_PACKET_T_HPP
