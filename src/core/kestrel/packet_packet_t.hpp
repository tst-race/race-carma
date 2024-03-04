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

#ifndef KESTREL_PACKET_PACKET_T_HPP
#define KESTREL_PACKET_PACKET_T_HPP

// Standard C++ headers
#include <cstddef>
#include <string>
#include <vector>

// SST headers
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/old/from_bytes.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/old/to_bytes.hpp>

// CARMA headers
#include <kestrel/origin_handle_t.hpp>
#include <kestrel/origin_span_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_span_t.hpp>

namespace kestrel {

class packet_packet_t final : origin_handle_t, origin_span_t {

  static constexpr int receiver_size_size_ = 1;
  static constexpr int payload_size_size_ = 2;

public:
  packet_type_t type = packet_type_t::packet_packet();
  psn_t receiver;
  std::vector<unsigned char> packet;

  //--------------------------------------------------------------------
  // Initialization and cleanup
  //--------------------------------------------------------------------
  //
  // Objects of this type are intended to be acquired from a pool and
  // released back to reduce memory churn. The init and dnit functions
  // should be called just after an object of this type is acquired and
  // just before it is released, respectively.
  //

  void dnit() noexcept {
    packet.clear();
  }

  void init(race_handle_t const & origin_handle,
            sdk_span_t const & origin_span) {
    dnit();
    set_origin_handle(origin_handle);
    set_origin_span(origin_span);
    receiver.clear();
  }

  void clear() {
    receiver.clear();
    packet.clear();
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size = std::size_t>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;

    SST_ASSERT((type == packet_type_t::packet_packet()));
    n += type.to_bytes_size<Size>();

    n += receiver.to_bytes_size<Size>();

    n += payload_size_size_;
    n += packet.size();

    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    SST_ASSERT((type == packet_type_t::packet_packet()));
    dst = type.to_bytes(dst);
    dst = receiver.to_bytes(dst);
    dst = sst::old::to_bytes(packet.size(),
                        dst,
                        payload_size_size_,
                        sst::integer_rep::pure_unsigned());
    dst = sst::copy_bytes(packet.begin(), packet.size(), dst);
    return dst;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT((sst::perfect_ge(avail, 0)));

    // Why assert? Because the caller should have already parsed and
    // verified the packet type. This function is not intended to be
    // used to probe for the packet type.
    src = type.from_bytes(src, avail);
    SST_ASSERT((type == packet_type_t::packet_packet()));

    src = receiver.from_bytes(src, avail);

    sst::checked_resize(packet,
                        sst::old::from_bytes<decltype(packet.size())>(
                            &src,
                            payload_size_size_,
                            avail,
                            sst::integer_rep::pure_unsigned()));
    sst::copy_bytes(&src, packet.size(), avail, packet.begin());

    return src;
  }

  //--------------------------------------------------------------------
};

extern template std::size_t
packet_packet_t::to_bytes_size<std::size_t>() const;

} // namespace kestrel

#endif // #ifndef KESTREL_PACKET_PACKET_T_HPP
