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

#ifndef KESTREL_RABBITMQ_PLUGIN_T_HPP
#define KESTREL_RABBITMQ_PLUGIN_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <IRaceSdkComms.h>
#include <PluginConfig.h>

#include <kestrel/basic_config_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/easy_ta2_plugin_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/rabbitmq/server_t.hpp>
#include <kestrel/rabbitmq/worker_key_cmp_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

struct plugin_types_t {
  using Config = basic_config_t;
  using Link = link_t;
  using LinkAddrinfo = link_addrinfo_t;
};

class plugin_t final
    : public easy_ta2_plugin_t<plugin_t, plugin_types_t> {

  friend class link_t;
  friend class connection_t;
  friend class worker_t;

  //--------------------------------------------------------------------
  // Channel properties
  //--------------------------------------------------------------------

public:

  static ChannelProperties initial_properties();

  //--------------------------------------------------------------------
  // Server tracking
  //--------------------------------------------------------------------
  //
  // TODO: Server information should be saved to disk.
  //
  // TODO: Servers can be scored somehow over time and be pruned from
  //       this list. E.g., if we fail to connect to a server a lot, we
  //       can stop using it.
  //

private:

  using servers_t = std::set<server_t>;

  sst::unique_ptr<servers_t> servers_{sst::in_place};

  servers_t & servers() noexcept {
    SST_ASSERT((servers_));
    // TODO: Do we want to default to the default address long term?
    if (servers_->empty()) {
      servers_->emplace(link_addrinfo_t());
    }
    return *servers_;
  }

  //--------------------------------------------------------------------

private:

  using workers_t = std::
      map<link_addrinfo_t, std::weak_ptr<worker_t>, worker_key_cmp_t>;

  sst::unique_ptr<workers_t> workers_{sst::in_place};

  // The number of entries in workers_ that need to be pruned. In other
  // words, the number of entries (k,v) for which v.expired() is true.
  unsigned long long workers_garbage_ = 0;

  workers_t & workers() noexcept {
    SST_ASSERT(workers_ != nullptr);
    return *workers_;
  }

  //--------------------------------------------------------------------

private:

  // The number of connections across all links that need to be garbage
  // collected. In other words, the sum of v->connections_garbage_ for
  // all values v in links_ for which v is nonnull and v->garbage() is
  // false.
  long long link_connections_garbage_ = 0;

  //--------------------------------------------------------------------

private:

  using connections_t = std::map<connection_id_t, connection_t *>;

  sst::unique_ptr<connections_t> connections_{sst::in_place};

  // The number of entries in connections_ that need to be pruned. In
  // other words, the number of entries (k,v) for which v is null.
  unsigned long long connections_garbage_ = 0;

  connections_t & connections() noexcept {
    SST_ASSERT(connections_ != nullptr);
    return *connections_;
  }

  SST_NODISCARD()
  connection_t &
  expect_connection(connection_id_t const & connection_id);

  SST_NODISCARD()
  connection_t const &
  expect_connection(connection_id_t const & connection_id) const {
    return const_cast<plugin_t *>(this)->expect_connection(
        connection_id);
  }

  //--------------------------------------------------------------------
  // Garbage collection
  //--------------------------------------------------------------------

protected:

  void collect_garbage(tracing_event_t tev) override;

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------

protected:

  void inner_activateChannel(tracing_event_t tev) final;

  void
  inner_closeConnection(tracing_event_t tev,
                        race_handle_t const & handle,
                        connection_id_t const & connectionId) final;

  void inner_createBootstrapLink(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 channel_id_t const & channelGid,
                                 std::string const & passphrase) final;

  void inner_createLink(tracing_event_t tev,
                        race_handle_t const & handle) final;

  void inner_deactivateChannel(tracing_event_t tev) final;

  void inner_init(tracing_event_t tev,
                  PluginConfig const & pluginConfig) final;

  void inner_loadLinkAddress(tracing_event_t tev,
                             race_handle_t const & handle,
                             link_address_t const & linkAddress,
                             link_status_t const & status) final;

  void inner_onUserAcknowledgementReceived(
      tracing_event_t tev,
      race_handle_t const & handle) final;

  void inner_onUserInputReceived(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 bool answered,
                                 std::string const & response) final;

  void inner_openConnection(tracing_event_t tev,
                            race_handle_t const & handle,
                            link_type_t const & linkType,
                            link_id_t const & linkId,
                            std::string const & linkHints,
                            std::int32_t sendTimeout) final;

  void inner_sendPackage(tracing_event_t tev,
                         race_handle_t const & handle,
                         connection_id_t const & connectionId,
                         RawData && pkg,
                         double timeoutTimestamp,
                         std::uint64_t batchId) final;

  void inner_serveFiles(tracing_event_t tev,
                        link_id_t const & linkId,
                        std::string const & path) final;

  void inner_shutdown(tracing_event_t tev) noexcept final;

  void
  inner_xGetChannelGids(tracing_event_t tev,
                        std::vector<std::string> & channelGids) final;

  void inner_xGetChannelProperties(
      tracing_event_t tev,
      std::string const & channelGid,
      ChannelProperties & channelProperties) final;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  explicit plugin_t(IRaceSdkComms & sdk);

  ~plugin_t() SST_NOEXCEPT(true) override = default;

  plugin_t(plugin_t const &) = delete;

  plugin_t & operator=(plugin_t const &) = delete;

  plugin_t(plugin_t &&) = delete;

  plugin_t & operator=(plugin_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_PLUGIN_T_HPP
