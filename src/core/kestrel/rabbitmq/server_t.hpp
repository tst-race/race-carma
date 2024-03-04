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

#ifndef KESTREL_RABBITMQ_SERVER_T_HPP
#define KESTREL_RABBITMQ_SERVER_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/moved_from.hpp>

#include <kestrel/rabbitmq/link_addrinfo_t.hpp>

namespace kestrel {
namespace rabbitmq {

class server_t final {

  sst::moved_from moved_from_;

  link_addrinfo_t addrinfo_;

public:

  explicit server_t(link_addrinfo_t const & addrinfo);

  server_t(server_t const &) = default;
  server_t & operator=(server_t const &) = default;
  server_t(server_t &&) noexcept = default;
  server_t & operator=(server_t &&) noexcept = default;
  ~server_t() noexcept = default;

  //--------------------------------------------------------------------
  // Comparisons
  //--------------------------------------------------------------------

public:

  SST_NODISCARD() int compare(server_t const & b) const noexcept {
    server_t const & a = *this;
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.addrinfo_.compare(b.addrinfo_);
  }

  SST_NODISCARD()
  friend bool operator<(server_t const & a,
                        server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) < 0;
  }

  SST_NODISCARD()
  friend bool operator>(server_t const & a,
                        server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) > 0;
  }

  SST_NODISCARD()
  friend bool operator<=(server_t const & a,
                         server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) <= 0;
  }

  SST_NODISCARD()
  friend bool operator>=(server_t const & a,
                         server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) >= 0;
  }

  SST_NODISCARD()
  friend bool operator==(server_t const & a,
                         server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) == 0;
  }

  SST_NODISCARD()
  friend bool operator!=(server_t const & a,
                         server_t const & b) noexcept {
    SST_ASSERT(!a.moved_from_);
    SST_ASSERT(!b.moved_from_);
    return a.compare(b) != 0;
  }

  //--------------------------------------------------------------------
  // generate_link
  //--------------------------------------------------------------------
  //
  // Returns a creator-side link_addrinfo_t for this server with a
  // randomly generated queue name.
  //

  link_addrinfo_t generate_link() const;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_SERVER_T_HPP
