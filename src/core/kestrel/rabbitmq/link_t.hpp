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

#ifndef KESTREL_RABBITMQ_LINK_T_HPP
#define KESTREL_RABBITMQ_LINK_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <map>
#include <memory>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ChannelProperties.h>
#include <LinkProperties.h>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/easy_ta2_link_t.hpp>
#include <kestrel/easy_ta2_plugin_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

class plugin_t;

class link_t final
    : public easy_ta2_link_t<plugin_t, link_t, link_addrinfo_t> {

  template<class, class>
  friend class kestrel::easy_ta2_plugin_t;

  friend class plugin_t;

  friend class connection_t;

  friend class worker_t;

  //--------------------------------------------------------------------

  using connections_t =
      std::map<connection_id_t, sst::unique_ptr<connection_t>>;

  sst::unique_ptr<connections_t> connections_{sst::in_place};

  SST_NODISCARD() connections_t & connections() noexcept {
    SST_ASSERT(connections_ != nullptr);
    return *connections_;
  }

  SST_NODISCARD()
  connection_t & open_connection(tracing_event_t tev,
                                 long send_timeout);

  SST_NODISCARD()
  connections_t::iterator close_connection(
      tracing_event_t tev,
      connections_t::iterator it,
      race_handle_t const & handle = race_handle_t::null());

  void close_connection(tracing_event_t tev,
                        connection_t & connection,
                        race_handle_t const & handle);

  void close_all_connections(tracing_event_t tev) noexcept;

  //--------------------------------------------------------------------

public:

  using easy_ta2_link_t::easy_ta2_link_t;

  link_t(link_t const &) = delete;
  link_t & operator=(link_t const &) = delete;
  link_t(link_t &&) = delete;
  link_t & operator=(link_t &&) = delete;
  ~link_t() noexcept = default;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_LINK_T_HPP
