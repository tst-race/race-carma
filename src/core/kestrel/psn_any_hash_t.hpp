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

#ifndef KESTREL_PSN_ANY_HASH_T_HPP
#define KESTREL_PSN_ANY_HASH_T_HPP

#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/psn_hash_1_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

class psn_any_hash_t final {

  //--------------------------------------------------------------------
  // Data
  //--------------------------------------------------------------------

public:

  int type;

  union {
    psn_hash_1_t hash_1;
  };

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  psn_any_hash_t() SST_NOEXCEPT(noexcept(psn_hash_1_t()))
      : type(1),
        hash_1() {
  }

  psn_any_hash_t(psn_any_hash_t const &) SST_NOEXCEPT(true) = default;

  psn_any_hash_t & operator=(psn_any_hash_t const &)
      SST_NOEXCEPT(true) = default;

  psn_any_hash_t(psn_any_hash_t &&) SST_NOEXCEPT(true) = default;

  psn_any_hash_t & operator=(psn_any_hash_t &&)
      SST_NOEXCEPT(true) = default;

  ~psn_any_hash_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  friend bytes_t & serialize(bytes_t & dst, psn_any_hash_t const & src);

  friend void deserialize(bytes_t const & src,
                          bytes_t::size_type & idx,
                          psn_any_hash_t & dst);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_PSN_ANY_HASH_T_HPP
