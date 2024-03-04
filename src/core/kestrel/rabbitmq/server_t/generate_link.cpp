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
#include <kestrel/rabbitmq/server_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/server_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <kestrel/rabbitmq/link_addrinfo_t.hpp>

namespace kestrel {
namespace rabbitmq {

link_addrinfo_t server_t::generate_link() const {
  link_addrinfo_t x;
  x.host(addrinfo_.host());
  x.port(addrinfo_.port());
  x.username(addrinfo_.username());
  x.password(addrinfo_.password());
  x.vhost(addrinfo_.vhost());
  return x;
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
