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

#ifndef KESTREL_PSN_HASH_1_T_HPP
#define KESTREL_PSN_HASH_1_T_HPP

#include <array>
#include <cstring>

#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/to_unsigned.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/serialization.hpp>

namespace kestrel {

class psn_hash_1_t final {

  std::array<unsigned char, 16> h_{};

  //--------------------------------------------------------------------

public:

  static bytes_t::size_type size() noexcept {
    return 16;
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  psn_hash_1_t() SST_NOEXCEPT(true) = default;

  psn_hash_1_t(psn_hash_1_t const &) SST_NOEXCEPT(true) = default;

  psn_hash_1_t & operator=(psn_hash_1_t const &)
      SST_NOEXCEPT(true) = default;

  psn_hash_1_t(psn_hash_1_t &&) SST_NOEXCEPT(true) = default;

  psn_hash_1_t & operator=(psn_hash_1_t &&)
      SST_NOEXCEPT(true) = default;

  ~psn_hash_1_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Construction from a PSN
  //--------------------------------------------------------------------

public:

  explicit psn_hash_1_t(psn_t const & psn);

  //--------------------------------------------------------------------
  // Comparisons
  //--------------------------------------------------------------------

private:

  SST_NODISCARD() int compare(psn_hash_1_t const & b) const {
    psn_hash_1_t const & a = *this;
    return std::memcmp(a.h_.data() + 1,
                       b.h_.data() + 1,
                       sst::to_unsigned(a.h_.size()) - 1U);
  }

public:

  SST_NODISCARD()
  friend bool operator<(psn_hash_1_t const & a,
                        psn_hash_1_t const & b) {
    return a.compare(b) < 0;
  }

  SST_NODISCARD()
  friend bool operator>(psn_hash_1_t const & a,
                        psn_hash_1_t const & b) {
    return a.compare(b) > 0;
  }

  SST_NODISCARD()
  friend bool operator<=(psn_hash_1_t const & a,
                         psn_hash_1_t const & b) {
    return a.compare(b) <= 0;
  }

  SST_NODISCARD()
  friend bool operator>=(psn_hash_1_t const & a,
                         psn_hash_1_t const & b) {
    return a.compare(b) >= 0;
  }

  SST_NODISCARD()
  friend bool operator==(psn_hash_1_t const & a,
                         psn_hash_1_t const & b) {
    return a.compare(b) == 0;
  }

  SST_NODISCARD()
  friend bool operator!=(psn_hash_1_t const & a,
                         psn_hash_1_t const & b) {
    return a.compare(b) != 0;
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

public:

  friend serialize_size_t serialize_size(psn_hash_1_t const & src);

  friend bytes_t & serialize(bytes_t & dst, psn_hash_1_t const & src);

  bool try_deserialize(bytes_t const & src, bytes_t::size_type & idx);

  friend void deserialize(bytes_t const & src,
                          bytes_t::size_type & idx,
                          psn_hash_1_t & dst);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_PSN_HASH_1_T_HPP
