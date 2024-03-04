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

#ifndef KESTREL_EASIER_TA2_CONNECTION_T_HPP
#define KESTREL_EASIER_TA2_CONNECTION_T_HPP

#include <map>
#include <memory>
#include <tuple>
#include <utility>

#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/easier_ta2_parent_ptr_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

// TODO: This class should eventually be renamed to
//       easy_ta2_connection_t. This class was named this way for newer
//       plugins because the structure of kestrel::rabbitmq is/was
//       holding back easy_ta2_plugin_t et al.

template<class Plugin, class Link, class Connection, class Package>
class easier_ta2_connection_t {

  template<class, class>
  friend class easy_ta2_plugin_t;

  template<class, class>
  friend class easier_ta2_plugin_t;

  template<class, class, class>
  friend class easy_ta2_link_t;

  template<class, class, class, class>
  friend class easier_ta2_link_t;

  template<class, class, class, class>
  friend class easier_ta2_package_t;

  //--------------------------------------------------------------------
  // Fully derived *this
  //--------------------------------------------------------------------

protected:

  Connection & connection() noexcept {
    return static_cast<Connection &>(*this);
  }

  Connection const & connection() const noexcept {
    return static_cast<Connection const &>(*this);
  }

  //--------------------------------------------------------------------
  // link
  //--------------------------------------------------------------------

private:

  Link * link_;

protected:

  SST_NODISCARD()
  Link & link() noexcept {
    SST_ASSERT((link_ != nullptr));
    return *link_;
  }

  SST_NODISCARD()
  Link const & link() const noexcept {
    SST_ASSERT((link_ != nullptr));
    return *link_;
  }

  //--------------------------------------------------------------------
  // plugin
  //--------------------------------------------------------------------

public:

  SST_NODISCARD()
  Plugin & plugin() noexcept {
    return this->link().plugin();
  }

  SST_NODISCARD()
  Plugin const & plugin() const noexcept {
    return this->link().plugin();
  }

  //--------------------------------------------------------------------
  // sdk
  //--------------------------------------------------------------------

protected:

  SST_NODISCARD()
  ta2_sdk_t & sdk() noexcept {
    return this->plugin().sdk();
  }

  //--------------------------------------------------------------------
  // id
  //--------------------------------------------------------------------

private:

  connection_id_t id_;

public:

  SST_NODISCARD()
  connection_id_t const & id() const noexcept {
    return id_;
  }

  //--------------------------------------------------------------------
  // lookup
  //--------------------------------------------------------------------

private:

  using lookup_t =
      typename std::map<connection_id_t, Connection *>::iterator;

  sst::optional<lookup_t> lookup_;

public:

  void set_lookup(lookup_t const & lookup) noexcept {
    lookup_ = lookup;
  }

  lookup_t const & lookup() const noexcept {
    SST_ASSERT((lookup_));
    return *lookup_;
  }

  //--------------------------------------------------------------------
  // self
  //--------------------------------------------------------------------

private:

  using self_t = std::shared_ptr<Connection> *;

  sst::optional<self_t> self_;

public:

  void set_self(self_t const & self) noexcept {
    self_ = self;
  }

  self_t const & self() const noexcept {
    SST_ASSERT((self_));
    return *self_;
  }

  //--------------------------------------------------------------------
  // ptr
  //--------------------------------------------------------------------

public:

  easier_ta2_parent_ptr_t<Connection> ptr() const noexcept {
    return *this->self();
  }

  //--------------------------------------------------------------------
  // Packages
  //--------------------------------------------------------------------

protected:

  using packages_t = std::map<race_handle_t, std::shared_ptr<Package>>;

  using packages_iterator_t = typename packages_t::iterator;

  packages_t packages_;

  SST_NODISCARD()
  packages_t & packages() noexcept {
    return packages_;
  }

  SST_NODISCARD()
  packages_t const & packages() const noexcept {
    return packages_;
  }

  void send_package(tracing_event_t tev,
                    race_handle_t const & handle,
                    bytes_t const & data) {
    SST_TEV_TOP(tev);
    auto const r = this->packages().emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(
            std::make_shared<Package>(this->connection(),
                                      handle,
                                      data)));
    if (!r.second) {
      // TODO: We should probably have a utility function to construct
      //       this exception.
      throw std::logic_error("The SDK produced a duplicate handle");
    }
    packages_iterator_t const & it = r.first;
    Package & package = *it->second;
    package.set_lookup(it);
    package.send();
    SST_TEV_BOT(tev);
  }

  packages_iterator_t
  cancel_package(tracing_event_t tev,
                 packages_iterator_t const & it) noexcept {
    SST_TEV_TOP(tev);
    SST_ASSERT((it != this->packages().end()));
    SST_ASSERT((it == this->packages().find(it->first)));
    SST_ASSERT((it->second != nullptr));
    Package & package = *it->second;
    try {
      this->sdk().onPackageStatusChanged(
          SST_TEV_ARG(tev),
          package.handle().value(),
          package_status_t::failed_generic().value(),
          0);
    } catch (...) {
      ;
    }
    return this->packages().erase(it);
    SST_TEV_BOT(tev);
  }

  void cancel_all_packages(tracing_event_t tev) noexcept {
    SST_TEV_TOP(tev);
    auto it = this->packages().begin();
    while (it != this->packages().end()) {
      it = this->cancel_package(SST_TEV_ARG(tev), it);
    }
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------

public:

  virtual void recv() {
  }

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit easier_ta2_connection_t(Link & link,
                                   connection_id_t const & id)
      : link_(&link),
        id_(id) {
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  easier_ta2_connection_t() = delete;

  virtual ~easier_ta2_connection_t() noexcept {
    tracing_event_t SST_TEV_DEF(tev);
    this->cancel_all_packages(SST_TEV_ARG(tev));
  }

  easier_ta2_connection_t(easier_ta2_connection_t const &) = delete;

  easier_ta2_connection_t &
  operator=(easier_ta2_connection_t const &) = delete;

  easier_ta2_connection_t(easier_ta2_connection_t &&) = delete;

  easier_ta2_connection_t &
  operator=(easier_ta2_connection_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASIER_TA2_CONNECTION_T_HPP
