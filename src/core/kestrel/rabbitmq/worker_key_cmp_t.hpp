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

#ifndef KESTREL_RABBITMQ_WORKER_KEY_CMP_T_HPP
#define KESTREL_RABBITMQ_WORKER_KEY_CMP_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <kestrel/rabbitmq/link_addrinfo_t.hpp>

namespace kestrel {
namespace rabbitmq {

class worker_key_cmp_t final {

public:

  bool operator()(link_addrinfo_t const & a,
                  link_addrinfo_t const & b) const noexcept;

}; //

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_WORKER_KEY_CMP_T_HPP
