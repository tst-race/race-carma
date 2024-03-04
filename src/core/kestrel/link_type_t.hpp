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

#ifndef KESTREL_LINK_TYPE_T_HPP
#define KESTREL_LINK_TYPE_T_HPP

#include <sst/catalog/boxed.hpp>

#include <LinkProperties.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

#define CARMA_LINK_TYPE_LIST                                           \
  CARMA_ITEM(undef, LT_UNDEF)                                          \
  CARMA_ITEM(send, LT_SEND)                                            \
  CARMA_ITEM(recv, LT_RECV)                                            \
  CARMA_ITEM(bidi, LT_BIDI)

class link_type_t final : sst::boxed<LinkType, link_type_t> {
  using boxed = sst::boxed<LinkType, link_type_t>;
  friend class sst::boxed<LinkType, link_type_t>;

  enum class enum_t : std::underlying_type<LinkType>::type {
#define CARMA_ITEM(a, b) a = b,
    CARMA_LINK_STATUS_LIST
#undef CARMA_ITEM
  };

public:
  using boxed::boxed;
  using boxed::value;
  using boxed::operator<;
  using boxed::operator==;
  using boxed::operator!=;

  constexpr operator enum_t() const noexcept {
    return static_cast<enum_t>(value());
  }

#define CARMA_ITEM(a, b)                                               \
  static constexpr link_type_t a() noexcept { return link_type_t(b); }
  CARMA_LINK_TYPE_LIST
#undef CARMA_ITEM

  constexpr bool can_send() const noexcept {
    return *this == send() || *this == bidi();
  }

  constexpr bool can_recv() const noexcept {
    return *this == recv() || *this == bidi();
  }

  //--------------------------------------------------------------------
  // to_json (ADL)
  //--------------------------------------------------------------------

  friend void to_json(json_t & dst, link_type_t const & src);

  //--------------------------------------------------------------------
  // to_json (member)
  //--------------------------------------------------------------------

public:

  json_t to_json() const;

  //--------------------------------------------------------------------
};

//----------------------------------------------------------------------
// to_json (ADL)
//----------------------------------------------------------------------

void to_json(json_t & dst, link_type_t const & src);

//----------------------------------------------------------------------

} // namespace kestrel

#endif // #ifndef KESTREL_LINK_TYPE_T_HPP
