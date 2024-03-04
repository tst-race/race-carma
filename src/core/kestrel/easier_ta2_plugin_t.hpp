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

#ifndef KESTREL_EASIER_TA2_PLUGIN_T_HPP
#define KESTREL_EASIER_TA2_PLUGIN_T_HPP

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <LinkStatus.h>

#include <kestrel/easy_ta2_plugin_t.hpp>

namespace kestrel {

// TODO: This class should eventually be fully merged with
//       easy_ta2_plugin_t. This class was broken off to use for newer
//       plugins because the structure of kestrel::rabbitmq is/was
//       holding back easy_ta2_plugin_t.

template<class Plugin, class Types>
class easier_ta2_plugin_t : public easy_ta2_plugin_t<Plugin, Types> {

  using Link = typename Types::Link;
  using Connection = typename Types::Connection;

  template<class, class, class>
  friend class easy_ta2_link_t;

  template<class, class, class, class>
  friend class easier_ta2_link_t;

  template<class, class, class, class>
  friend class easier_ta2_connection_t;

  template<class, class, class, class>
  friend class easier_ta2_package_t;

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

protected:

  explicit easier_ta2_plugin_t(IRaceSdkComms & sdk,
                               std::string const & plugin_name)
      : easier_ta2_plugin_t::common_plugin_t(sdk, plugin_name),
        easier_ta2_plugin_t::easy_ta2_plugin_t(sdk, plugin_name) {
  }

  //--------------------------------------------------------------------
  // Connections
  //--------------------------------------------------------------------

protected:

  using connections_t = std::map<connection_id_t, Connection *>;

  using connections_iterator_t = typename connections_t::iterator;

  connections_t connections_;

  SST_NODISCARD()
  connections_t & connections() noexcept {
    return connections_;
  }

  SST_NODISCARD()
  connections_t const & connections() const noexcept {
    return connections_;
  }

  connections_iterator_t expect_connection(tracing_event_t tev,
                                           connection_id_t const & id) {
    SST_TEV_TOP(tev);
    auto const it = this->connections().find(id);
    if (it == this->connections().end()) {
      throw this->sdk().unknown_connection_id(id);
    }
    return it;
    SST_TEV_BOT(tev);
  }

  void inner_openConnection(tracing_event_t tev,
                            race_handle_t const & handle,
                            link_type_t const & linkType,
                            link_id_t const & linkId,
                            std::string const & linkHints,
                            std::int32_t sendTimeout) override {
    SST_TEV_TOP(tev);
    (void)linkHints;
    (void)sendTimeout;
    Link & link = this->expect_link(SST_TEV_ARG(tev), linkId)->second;
    link.validate_type(linkType);
    auto const a = link.open_connection(SST_TEV_ARG(tev));
    try {
      Connection & connection = *a->second;
      auto const b =
          this->connections().emplace(connection.id(), &connection);
      SST_ASSERT((b.second));
      try {
        this->sdk().onConnectionStatusChanged(SST_TEV_ARG(tev),
                                              handle.value(),
                                              connection.id().value(),
                                              CONNECTION_OPEN,
                                              link.properties(),
                                              0);
      } catch (...) {
        this->connections().erase(b.first);
        throw;
      }
      connection.set_lookup(b.first);
    } catch (...) {
      link.connections().erase(a);
      throw;
    }
    SST_TEV_BOT(tev);
  }

  void
  inner_closeConnection(tracing_event_t tev,
                        race_handle_t const & handle,
                        connection_id_t const & connectionId) override {
    SST_TEV_TOP(tev);
    this->expect_connection(SST_TEV_ARG(tev), connectionId)
        ->second->link()
        .close_connection(SST_TEV_ARG(tev), connectionId, handle);
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
  // Packages
  //--------------------------------------------------------------------

public:

  void inner_sendPackage(tracing_event_t tev,
                         race_handle_t const & handle,
                         connection_id_t const & connectionId,
                         RawData && pkg,
                         double,
                         std::uint64_t) override {
    SST_TEV_TOP(tev);
    this->expect_connection(SST_TEV_ARG(tev), connectionId)
        ->second->send_package(SST_TEV_ARG(tev), handle, pkg);
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  easier_ta2_plugin_t() = delete;

  ~easier_ta2_plugin_t() SST_NOEXCEPT(true) override {
    tracing_event_t SST_TEV_DEF(tev);
    this->close_all_links(SST_TEV_ARG(tev));
  }

  easier_ta2_plugin_t(easier_ta2_plugin_t const &) = delete;

  easier_ta2_plugin_t & operator=(easier_ta2_plugin_t const &) = delete;

  easier_ta2_plugin_t(easier_ta2_plugin_t &&) = delete;

  easier_ta2_plugin_t & operator=(easier_ta2_plugin_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASIER_TA2_PLUGIN_T_HPP
