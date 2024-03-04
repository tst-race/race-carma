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

#ifndef KESTREL_EASY_TA2_LINK_T_HPP
#define KESTREL_EASY_TA2_LINK_T_HPP

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <ChannelProperties.h>
#include <LinkProperties.h>
#include <LinkType.h>

#include <kestrel/link_id_t.hpp>
#include <kestrel/link_role_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>

namespace kestrel {

template<class Plugin, class Link, class Addrinfo>
class easy_ta2_link_t {

  template<class, class>
  friend class easy_ta2_plugin_t;

  template<class, class>
  friend class easier_ta2_plugin_t;

  //--------------------------------------------------------------------
  // Fully derived *this
  //--------------------------------------------------------------------

protected:

  Link & link() noexcept {
    return static_cast<Link &>(*this);
  }

  Link const & link() const noexcept {
    return static_cast<Link const &>(*this);
  }

  //--------------------------------------------------------------------
  // plugin
  //--------------------------------------------------------------------

private:

  Plugin * plugin_;

protected:

  SST_NODISCARD()
  Plugin & plugin() noexcept {
    SST_ASSERT((plugin_ != nullptr));
    return *plugin_;
  }

  SST_NODISCARD()
  Plugin const & plugin() const noexcept {
    SST_ASSERT((plugin_ != nullptr));
    return *plugin_;
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

  link_id_t id_;

public:

  SST_NODISCARD()
  link_id_t const & id() const noexcept {
    return id_;
  }

  //--------------------------------------------------------------------
  // addrinfo
  //--------------------------------------------------------------------

private:

  Addrinfo addrinfo_;

public:

  SST_NODISCARD()
  Addrinfo const & addrinfo() const noexcept {
    return addrinfo_;
  }

  //--------------------------------------------------------------------
  // properties
  //--------------------------------------------------------------------

private:

  LinkProperties properties_;

public:

  SST_NODISCARD()
  LinkProperties const & properties() const noexcept {
    return properties_;
  }

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit easy_ta2_link_t(Plugin & plugin,
                           link_id_t const & id,
                           Addrinfo const & addrinfo)
      : plugin_(&plugin),
        id_(id),
        addrinfo_(addrinfo) {
    link_role_t const r1 = addrinfo_.role();
    link_role_t const r2 = r1 == link_role_t::creator() ?
                               link_role_t::loader() :
                               link_role_t::creator();
    ChannelProperties const & a = plugin.properties();
    LinkProperties & b = properties_;
    b.linkType = a.linkDirection == LD_BIDI ? LT_BIDI :
                 (a.linkDirection == LD_LOADER_TO_CREATOR ? r1 : r2)
                         == link_role_t::loader() ?
                                              LT_SEND :
                                              LT_RECV;
    b.transmissionType = a.transmissionType;
    b.connectionType = a.connectionType;
    b.sendType = a.sendType;
    b.reliable = a.reliable;
    b.isFlushable = a.isFlushable;
    b.duration_s = a.duration_s;
    b.period_s = a.period_s;
    b.mtu = a.mtu;
    b.worst = a.creatorExpected;
    b.expected = a.creatorExpected;
    b.best = a.creatorExpected;
    b.supported_hints = a.supported_hints;
    b.channelGid = a.channelGid;
    addrinfo_.role(r2);
    b.linkAddress = addrinfo_.to_link_address().value();
    addrinfo_.role(r1);
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  easy_ta2_link_t() = delete;

  virtual ~easy_ta2_link_t() SST_NOEXCEPT(true) = default;

  easy_ta2_link_t(easy_ta2_link_t const &) = delete;

  easy_ta2_link_t & operator=(easy_ta2_link_t const &) = delete;

  easy_ta2_link_t(easy_ta2_link_t &&) = delete;

  easy_ta2_link_t & operator=(easy_ta2_link_t &&) = delete;

  //--------------------------------------------------------------------

public:

  void validate_type(link_type_t const & b) const {
    link_type_t const a(this->properties().linkType);
    if (a != link_type_t::bidi() && a != b) {
      throw std::runtime_error("Incompatible link type");
    }
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASY_TA2_LINK_T_HPP
