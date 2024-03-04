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

#ifndef KESTREL_EASIER_TA2_LINK_T_HPP
#define KESTREL_EASIER_TA2_LINK_T_HPP

#include <map>
#include <memory>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <ConnectionStatus.h>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/easier_ta2_connection_t.hpp>
#include <kestrel/easy_ta2_link_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

// TODO: This class should eventually be fully merged with
//       easy_ta2_link_t. This class was broken off to use for newer
//       plugins because the structure of kestrel::rabbitmq is/was
//       holding back easy_ta2_link_t.

template<class Plugin, class Link, class Addrinfo, class Connection>
class easier_ta2_link_t
    : public easy_ta2_link_t<Plugin, Link, Addrinfo> {

  using base = easy_ta2_link_t<Plugin, Link, Addrinfo>;

  template<class, class>
  friend class easy_ta2_plugin_t;

  template<class, class>
  friend class easier_ta2_plugin_t;

  template<class, class, class, class>
  friend class easier_ta2_connection_t;

  template<class, class, class, class>
  friend class easier_ta2_package_t;

  //--------------------------------------------------------------------
  // Connections
  //--------------------------------------------------------------------

protected:

  using connections_t =
      std::map<connection_id_t, std::shared_ptr<Connection>>;

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

  SST_NODISCARD()
  connections_iterator_t open_connection(tracing_event_t tev) {
    SST_TEV_TOP(tev);
    while (true) {
      connection_id_t const id(
          this->sdk().generateConnectionId(SST_TEV_ARG(tev),
                                           this->link().id().value()));
      auto const r = this->connections().emplace(
          id,
          std::make_shared<Connection>(this->link(), id));
      if (r.second) {
        r.first->second->set_self(&r.first->second);
        try {
          r.first->second->recv();
        } catch (...) {
          this->connections().erase(r.first);
          throw;
        }
        return r.first;
      }
    }
    SST_TEV_BOT(tev);
  }

  connections_iterator_t close_connection(
      tracing_event_t tev,
      connections_iterator_t const it,
      race_handle_t const & handle = race_handle_t::null()) noexcept {
    SST_TEV_TOP(tev);
    SST_ASSERT((it != this->connections().end()));
    SST_ASSERT((it == this->connections().find(it->first)));
    Connection & connection = *it->second;
    connection.cancel_all_packages(SST_TEV_ARG(tev));
    try {
      this->sdk().onConnectionStatusChanged(SST_TEV_ARG(tev),
                                            handle.value(),
                                            connection.id().value(),
                                            CONNECTION_CLOSED,
                                            this->properties(),
                                            0);
    } catch (...) {
      ;
    }
    this->plugin().connections().erase(connection.lookup());
    return this->connections().erase(it);
    SST_TEV_BOT(tev);
  }

  void close_connection(
      tracing_event_t tev,
      connection_id_t const & id,
      race_handle_t const & handle = race_handle_t::null()) {
    SST_TEV_TOP(tev);
    this->close_connection(
        SST_TEV_ARG(tev),
        this->expect_connection(SST_TEV_ARG(tev), id),
        handle);
    SST_TEV_BOT(tev);
  }

  void close_all_connections(tracing_event_t tev) noexcept {
    SST_TEV_TOP(tev);
    auto it = this->connections().begin();
    while (it != this->connections().end()) {
      it = this->close_connection(SST_TEV_ARG(tev), it);
    }
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

protected:

  using base::base;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  easier_ta2_link_t() = delete;

  ~easier_ta2_link_t() noexcept override {
    tracing_event_t SST_TEV_DEF(tev);
    this->close_all_connections(SST_TEV_ARG(tev));
  }

  easier_ta2_link_t(easier_ta2_link_t const &) = delete;

  easier_ta2_link_t & operator=(easier_ta2_link_t const &) = delete;

  easier_ta2_link_t(easier_ta2_link_t &&) = delete;

  easier_ta2_link_t & operator=(easier_ta2_link_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASIER_TA2_LINK_T_HPP
