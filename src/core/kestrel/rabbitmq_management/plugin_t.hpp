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

#ifndef KESTREL_RABBITMQ_MANAGEMENT_PLUGIN_T_HPP
#define KESTREL_RABBITMQ_MANAGEMENT_PLUGIN_T_HPP

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <IRaceSdkComms.h>

#include <kestrel/basic_config_t.hpp>
#include <kestrel/easier_ta2_plugin_t.hpp>
#include <kestrel/rabbitmq_management/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq_management/link_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

namespace rabbitmq_management {

class connection_t;

namespace guts {
namespace plugin_t {

struct types_t {
  using Config = basic_config_t;
  using Link = link_t;
  using LinkAddrinfo = ::kestrel::rabbitmq_management::link_addrinfo_t;
  using Connection = connection_t;
};

} // namespace plugin_t
} // namespace guts

class plugin_t final
    : public easier_ta2_plugin_t<plugin_t, guts::plugin_t::types_t> {

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit plugin_t(IRaceSdkComms & sdk);

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  plugin_t() = delete;

  ~plugin_t() SST_NOEXCEPT(true) override = default;

  plugin_t(plugin_t const &) = delete;

  plugin_t & operator=(plugin_t const &) = delete;

  plugin_t(plugin_t &&) = delete;

  plugin_t & operator=(plugin_t &&) = delete;

  //--------------------------------------------------------------------
  // TODO: Garbage collection? Or is default OK?
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
};

} // namespace rabbitmq_management
} // namespace kestrel

#endif // #ifndef KESTREL_RABBITMQ_MANAGEMENT_PLUGIN_T_HPP
