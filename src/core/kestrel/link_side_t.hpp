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

#ifndef KESTREL_LINK_SIDE_T_HPP
#define KESTREL_LINK_SIDE_T_HPP

#include <sst/catalog/boxed.hpp>
#include <sst/catalog/underlying_type_t.hpp>

#include <ChannelRole.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

#define KESTREL_LINK_SIDE_LIST                                         \
  KESTREL_ITEM(both, LS_BOTH)                                          \
  KESTREL_ITEM(creator, LS_CREATOR)                                    \
  KESTREL_ITEM(loader, LS_LOADER)                                      \
  KESTREL_ITEM(undef, LS_UNDEF)

class link_side_t : sst::boxed<LinkSide, link_side_t> {

  using boxed = sst::boxed<LinkSide, link_side_t>;
  friend class sst::boxed<LinkSide, link_side_t>;

  //--------------------------------------------------------------------
  // Inherited operations
  //--------------------------------------------------------------------

private:

  using boxed::boxed;

public:

  using boxed::value;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  link_side_t() noexcept : boxed(LS_UNDEF) {
  }

  link_side_t(link_side_t const &) noexcept = default;

  link_side_t & operator=(link_side_t const &) noexcept = default;

  link_side_t(link_side_t &&) noexcept = default;

  link_side_t & operator=(link_side_t &&) noexcept = default;

  ~link_side_t() noexcept = default;

  //--------------------------------------------------------------------
  // Named values
  //--------------------------------------------------------------------

public:

#define KESTREL_ITEM(a, b)                                             \
  static constexpr link_side_t a() noexcept { return link_side_t(b); }

  KESTREL_LINK_SIDE_LIST

#undef KESTREL_ITEM

  //--------------------------------------------------------------------
  // Switch statement support
  //--------------------------------------------------------------------

private:

#define KESTREL_ITEM(a, b) a = b,

  enum class enum_t : sst::underlying_type_t<LinkSide> {
    KESTREL_LINK_SIDE_LIST
  };

#undef KESTREL_ITEM

public:

  constexpr operator enum_t() const noexcept {
    return static_cast<enum_t>(value());
  }

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void to_json(json_t & dst, link_side_t const & src);

  friend void from_json(json_t const & src, link_side_t & dst);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_LINK_SIDE_T_HPP
