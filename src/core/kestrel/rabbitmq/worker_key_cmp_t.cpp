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

// Include first to test independence.
#include <kestrel/rabbitmq/worker_key_cmp_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/worker_key_cmp_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <sst/catalog/SST_ASSERT.h>

#include <kestrel/rabbitmq/link_addrinfo_t.hpp>

namespace kestrel {
namespace rabbitmq {

bool worker_key_cmp_t::operator()(
    link_addrinfo_t const & a,
    link_addrinfo_t const & b) const noexcept {
  SST_ASSERT(!a.moved_from_);
  SST_ASSERT(!b.moved_from_);
  {
    if (a.role() != b.role()) {
      return a.role() < b.role();
    }
  }
  {
    int const x = a.host().compare(b.host());
    if (x != 0) {
      return x < 0;
    }
  }
  {
    if (a.port() != b.port()) {
      return a.port() < b.port();
    }
  }
  {
    int const x = a.username().compare(b.username());
    if (x != 0) {
      return x < 0;
    }
  }
  {
    int const x = a.password().compare(b.password());
    if (x != 0) {
      return x < 0;
    }
  }
  {
    int const x = a.vhost().compare(b.vhost());
    if (x != 0) {
      return x < 0;
    }
  }
  return false;
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
