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

#ifndef KESTREL_PACKET_TYPE_T_HPP
#define KESTREL_PACKET_TYPE_T_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <sst/catalog/boxed.hpp>
#include <sst/catalog/from_varint.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/old/from_bytes.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_varint.hpp>
#include <sst/catalog/track_it.hpp>
#include <sst/catalog/varint_size.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

#define CARMA_PACKET_TYPE_LIST                                         \
  CARMA_ITEM(add_contact_request)                                      \
  CARMA_ITEM(client_mb_packet)                                         \
  CARMA_ITEM(endorsement_request)                                      \
  CARMA_ITEM(link_address_packet)                                      \
  CARMA_ITEM(mb_client_packet)                                         \
  CARMA_ITEM(mb_mc_up_bundle_packet)                                   \
  CARMA_ITEM(mb_mc_up_packet)                                          \
  CARMA_ITEM(mc_leader_init_packet)                                    \
  CARMA_ITEM(mc_mb_down_packet)                                        \
  CARMA_ITEM(mc_v_packet)                                              \
  CARMA_ITEM(packet_packet)                                            \
  CARMA_ITEM(registration_complete)

class packet_type_t final : sst::boxed<unsigned int, packet_type_t> {
  using boxed = sst::boxed<unsigned int, packet_type_t>;
  friend class sst::boxed<unsigned int, packet_type_t>;

  // clang-format off
  enum class enum_t : value_type {
#define CARMA_ITEM(a) a,
    CARMA_PACKET_TYPE_LIST
#undef CARMA_ITEM
    count
  };
  // clang-format on

public:
  using boxed::boxed;
  using boxed::operator==;
  using boxed::operator!=;
  using boxed::value;

#define CARMA_ITEM(a)                                                  \
  static constexpr packet_type_t a() noexcept {                        \
    return packet_type_t(static_cast<value_type>(enum_t::a));          \
  }
  CARMA_PACKET_TYPE_LIST
#undef CARMA_ITEM

  //--------------------------------------------------------------------

  std::string to_string() const {
    // clang-format off
    switch (value()) {
#define CARMA_ITEM(x) case x().value(): return #x;
      CARMA_PACKET_TYPE_LIST
#undef CARMA_ITEM
    }
    // clang-format on
    return sst::to_string(value());
  }

  //--------------------------------------------------------------------

  nlohmann::json to_json() const {
    return to_string();
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size>
  Size to_bytes_size() const {
    return sst::checked_cast<Size>(sst::varint_size(value()));
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    sst::to_varint(value(), sst::track_it(dst));
    return dst;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    value() = sst::from_varint<value_type>(sst::track_it(src, &avail));
    if (value() >= static_cast<value_type>(enum_t::count)) {
      throw std::runtime_error("unknown packet type: "
                               + sst::to_string(value()));
    }
    return src;
  }
};

} // namespace kestrel

#endif // #ifndef KESTREL_PACKET_TYPE_T_HPP
