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

#ifndef KESTREL_RABBITMQ_MANAGEMENT_LINK_ADDRINFO_T_HPP
#define KESTREL_RABBITMQ_MANAGEMENT_LINK_ADDRINFO_T_HPP

#include <kestrel/rabbitmq/link_addrinfo_template_t.hpp>

namespace kestrel {
namespace rabbitmq_management {

namespace guts {
namespace link_addrinfo_t {

extern char const default_host[];

} // namespace link_addrinfo_t
} // namespace guts

using link_addrinfo_t = rabbitmq::link_addrinfo_template_t<
    guts::link_addrinfo_t::default_host,
    15671U>;

} // namespace rabbitmq_management
} // namespace kestrel

#endif // #ifndef KESTREL_RABBITMQ_MANAGEMENT_LINK_ADDRINFO_T_HPP
