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

#ifndef KESTREL_EASY_TA2_PLUGIN_T_HPP
#define KESTREL_EASY_TA2_PLUGIN_T_HPP

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <EncPkg.h>
#include <IRacePluginComms.h>
#include <IRaceSdkComms.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <LinkType.h>
#include <PluginConfig.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/easy_common_plugin_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/ta2_plugin_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Plugin, class Types>
class easy_ta2_plugin_t
    : public easy_common_plugin_t<ta2_sdk_t, typename Types::Config>,
      public ta2_plugin_t {

  template<class, class, class>
  friend class easy_ta2_link_t;

  template<class, class, class, class>
  friend class easier_ta2_link_t;

  //--------------------------------------------------------------------

protected:

  using Config = typename Types::Config;
  using Link = typename Types::Link;
  using Addrinfo = typename Types::LinkAddrinfo;

  //--------------------------------------------------------------------
  // Fully derived *this
  //--------------------------------------------------------------------

protected:

  Plugin & plugin() noexcept {
    return static_cast<Plugin &>(*this);
  }

  Plugin const & plugin() const noexcept {
    return static_cast<Plugin const &>(*this);
  }

  //--------------------------------------------------------------------
  // Channel properties
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<ChannelProperties> properties_{sst::in_place};

protected:

  ChannelProperties & mutable_properties() noexcept {
    SST_ASSERT((properties_ != nullptr));
    return *properties_;
  }

public:

  ChannelProperties const & properties() const noexcept {
    SST_ASSERT((properties_ != nullptr));
    return *properties_;
  }

  //--------------------------------------------------------------------

private:

  void expect_active(bool const x = true) {
    ChannelStatus const y = x ? CHANNEL_AVAILABLE : CHANNEL_ENABLED;
    if (properties().channelStatus != y) {
      throw std::runtime_error(x ? "Plugin is not active." :
                                   "Plugin is already active.");
    }
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  explicit easy_ta2_plugin_t(IRaceSdkComms & sdk,
                             std::string const & plugin_name)
      : easy_ta2_plugin_t::common_plugin_t(sdk, plugin_name),
        easy_ta2_plugin_t::easy_common_plugin_t(sdk, plugin_name),
        easy_ta2_plugin_t::ta2_plugin_t(sdk, plugin_name) {
  }

  easy_ta2_plugin_t(easy_ta2_plugin_t const &) = delete;
  easy_ta2_plugin_t & operator=(easy_ta2_plugin_t const &) = delete;
  easy_ta2_plugin_t(easy_ta2_plugin_t &&) = delete;
  easy_ta2_plugin_t & operator=(easy_ta2_plugin_t &&) = delete;
  virtual ~easy_ta2_plugin_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------

protected:

  virtual void inner_activateChannel(tracing_event_t) {
    ;
  }

  void inner_activateChannel(tracing_event_t tev,
                             race_handle_t const &,
                             channel_id_t const &,
                             std::string const &) final {
    SST_TEV_TOP(tev);
    inner_activateChannel(SST_TEV_ARG(tev));
    SST_TEV_BOT(tev);
  }

  virtual void inner_createLink(tracing_event_t tev,
                                race_handle_t const & handle) {
    SST_TEV_TOP(tev);
    this->inner_loadLinkAddress(SST_TEV_ARG(tev),
                                handle,
                                Addrinfo().to_link_address(),
                                link_status_t::created());
    SST_TEV_BOT(tev);
  }

  void inner_createLink(tracing_event_t tev,
                        race_handle_t const & handle,
                        channel_id_t const &) final {
    SST_TEV_TOP(tev);
    inner_createLink(SST_TEV_ARG(tev), handle);
    SST_TEV_BOT(tev);
  }

  virtual void
  inner_createLinkFromAddress(tracing_event_t tev,
                              race_handle_t const & handle,
                              link_address_t const & linkAddress) {
    SST_TEV_TOP(tev);
    inner_loadLinkAddress(SST_TEV_ARG(tev), handle, linkAddress);
    SST_TEV_BOT(tev);
  }

  void inner_createLinkFromAddress(
      tracing_event_t tev,
      race_handle_t const & handle,
      channel_id_t const &,
      link_address_t const & linkAddress) final {
    SST_TEV_TOP(tev);
    inner_createLinkFromAddress(SST_TEV_ARG(tev), handle, linkAddress);
    SST_TEV_BOT(tev);
  }

  virtual void inner_deactivateChannel(tracing_event_t tev) {
    SST_TEV_TOP(tev);
    SST_THROW_UNIMPLEMENTED();
    SST_TEV_BOT(tev);
  }

  void inner_deactivateChannel(tracing_event_t tev,
                               race_handle_t const &,
                               channel_id_t const &) final {
    SST_TEV_TOP(tev);
    inner_deactivateChannel(SST_TEV_ARG(tev));
    SST_TEV_BOT(tev);
  }

  virtual void inner_flushChannel(tracing_event_t,
                                  race_handle_t const &,
                                  std::uint64_t) {
    ;
  }

  void inner_flushChannel(tracing_event_t tev,
                          race_handle_t const & handle,
                          channel_id_t const &,
                          std::uint64_t const batchId) final {
    SST_TEV_TOP(tev);
    inner_flushChannel(SST_TEV_ARG(tev), handle, batchId);
    SST_TEV_BOT(tev);
  }

  void inner_loadLinkAddress(tracing_event_t tev,
                             race_handle_t const & handle,
                             channel_id_t const &,
                             link_address_t const & linkAddress) final {
    SST_TEV_TOP(tev);
    inner_loadLinkAddress(SST_TEV_ARG(tev), handle, linkAddress);
    SST_TEV_BOT(tev);
  }

  virtual void
  inner_loadLinkAddresses(tracing_event_t tev,
                          race_handle_t const &,
                          std::vector<std::string> const &) {
    SST_TEV_TOP(tev);
    SST_THROW_UNIMPLEMENTED();
    SST_TEV_BOT(tev);
  }

  void inner_loadLinkAddresses(
      tracing_event_t tev,
      race_handle_t const & handle,
      channel_id_t const &,
      std::vector<std::string> const & linkAddresses) final {
    SST_TEV_TOP(tev);
    inner_loadLinkAddresses(SST_TEV_ARG(tev), handle, linkAddresses);
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
  // Outer plugin functions
  //--------------------------------------------------------------------

public:

  PluginResponse activateChannel(RaceHandle handle,
                                 std::string channelGid,
                                 std::string roleName) override;

  PluginResponse closeConnection(RaceHandle handle,
                                 ConnectionID connectionId) override;

  PluginResponse createBootstrapLink(RaceHandle handle,
                                     std::string channelGid,
                                     std::string passphrase) override;

  PluginResponse createLink(RaceHandle handle,
                            std::string channelGid) override;

  PluginResponse
  createLinkFromAddress(RaceHandle handle,
                        std::string channelGid,
                        std::string linkAddress) override;

  PluginResponse deactivateChannel(RaceHandle handle,
                                   std::string channelGid) override;

  PluginResponse destroyLink(RaceHandle handle, LinkID linkId) override;

  PluginResponse flushChannel(RaceHandle handle,
                              std::string channelGid,
                              std::uint64_t batchId) override;

  PluginResponse loadLinkAddress(RaceHandle handle,
                                 std::string channelGid,
                                 std::string linkAddress) override;

  PluginResponse
  loadLinkAddresses(RaceHandle handle,
                    std::string channelGid,
                    std::vector<std::string> linkAddresses) override;

  PluginResponse
  onUserAcknowledgementReceived(RaceHandle handle) override;

  PluginResponse
  onUserInputReceived(RaceHandle handle,
                      bool answered,
                      std::string const & response) override;

  PluginResponse openConnection(RaceHandle handle,
                                LinkType linkType,
                                LinkID linkId,
                                std::string linkHints,
                                std::int32_t sendTimeout) override;

  PluginResponse sendPackage(RaceHandle handle,
                             ConnectionID connectionId,
                             EncPkg pkg,
                             double timeoutTimestamp,
                             std::uint64_t batchId) override;

  PluginResponse shutdown() override;

  //--------------------------------------------------------------------
  // Miscellaneous functions
  //--------------------------------------------------------------------

protected:

  void verify_channel_id(std::string const & channel_id);

  //--------------------------------------------------------------------
  // Links
  //--------------------------------------------------------------------

protected:

  using links_t = std::map<link_id_t, Link>;

  using links_iterator_t = typename links_t::iterator;

  links_t links_;

  SST_NODISCARD()
  links_t & links() noexcept {
    return links_;
  }

  SST_NODISCARD()
  links_t const & links() const noexcept {
    return links_;
  }

  links_iterator_t expect_link(tracing_event_t tev,
                               link_id_t const & id) {
    SST_TEV_TOP(tev);
    auto const it = this->links().find(id);
    if (it == this->links().end()) {
      throw this->sdk().unknown_link_id(id);
    }
    return it;
    SST_TEV_BOT(tev);
  }

  SST_NODISCARD()
  links_iterator_t open_link(tracing_event_t tev,
                             Addrinfo const & addrinfo) {
    SST_TEV_TOP(tev);
    while (true) {
      link_id_t const id(
          this->sdk().generateLinkId(SST_TEV_ARG(tev),
                                     this->properties().channelGid));
      auto const r = this->links().emplace(
          std::piecewise_construct,
          std::forward_as_tuple(id),
          std::forward_as_tuple(this->plugin(), id, addrinfo));
      if (r.second) {
        return r.first;
      }
    }
    SST_TEV_BOT(tev);
  }

  links_iterator_t close_link(
      tracing_event_t tev,
      links_iterator_t const it,
      race_handle_t const & handle = race_handle_t::null()) noexcept {
    SST_TEV_TOP(tev);
    SST_ASSERT((it != this->links().end()));
    SST_ASSERT((it == this->links().find(it->first)));
    Link & link = it->second;
    link.close_all_connections(SST_TEV_ARG(tev));
    try {
      this->sdk().onLinkStatusChanged(SST_TEV_ARG(tev),
                                      handle.value(),
                                      link.id().value(),
                                      LINK_DESTROYED,
                                      link.properties(),
                                      0);
    } catch (...) {
      ;
    }
    return this->links().erase(it);
    SST_TEV_BOT(tev);
  }

  void close_all_links(tracing_event_t tev) noexcept {
    SST_TEV_TOP(tev);
    auto it = this->links().begin();
    while (it != this->links().end()) {
      it = this->close_link(SST_TEV_ARG(tev), it);
    }
    SST_TEV_BOT(tev);
  }

  virtual void inner_loadLinkAddress(
      tracing_event_t tev,
      race_handle_t const & handle,
      link_address_t const & linkAddress,
      link_status_t const & status = link_status_t::loaded()) {
    SST_TEV_TOP(tev);
    this->collect_garbage(SST_TEV_ARG(tev));
    Addrinfo const addrinfo(linkAddress);
    links_iterator_t const it =
        this->open_link(SST_TEV_ARG(tev), addrinfo);
    Link & link = it->second;
    try {
      this->sdk().onLinkStatusChanged(SST_TEV_ARG(tev),
                                      handle.value(),
                                      link.id().value(),
                                      status.value(),
                                      link.properties(),
                                      0);
    } catch (...) {
      this->links().erase(it);
      throw;
    }
    SST_TEV_BOT(tev);
  }

  void inner_destroyLink(tracing_event_t tev,
                         race_handle_t const & handle,
                         link_id_t const & linkId) override {
    SST_TEV_TOP(tev);
    this->collect_garbage(SST_TEV_ARG(tev));
    this->close_link(SST_TEV_ARG(tev),
                     this->expect_link(SST_TEV_ARG(tev), linkId),
                     handle);
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
  // Garbage collection
  //--------------------------------------------------------------------
  //
  // Removes any garbage in this channel's data structures. This should
  // be called at the beginning of every plugin function. Garbage is
  // generally only generated when errors occur. In essence, this
  // function delays any rollback process until the next call.
  //

protected:

  virtual void collect_garbage(tracing_event_t tev) {
    SST_TEV_TOP(tev);
    SST_TEV_BOT(tev);
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

//----------------------------------------------------------------------

#include <kestrel/easy_ta2_plugin_t/activateChannel.hpp>
#include <kestrel/easy_ta2_plugin_t/closeConnection.hpp>
#include <kestrel/easy_ta2_plugin_t/createBootstrapLink.hpp>
#include <kestrel/easy_ta2_plugin_t/createLink.hpp>
#include <kestrel/easy_ta2_plugin_t/createLinkFromAddress.hpp>
#include <kestrel/easy_ta2_plugin_t/deactivateChannel.hpp>
#include <kestrel/easy_ta2_plugin_t/destroyLink.hpp>
#include <kestrel/easy_ta2_plugin_t/flushChannel.hpp>
#include <kestrel/easy_ta2_plugin_t/loadLinkAddress.hpp>
#include <kestrel/easy_ta2_plugin_t/loadLinkAddresses.hpp>
#include <kestrel/easy_ta2_plugin_t/onUserAcknowledgementReceived.hpp>
#include <kestrel/easy_ta2_plugin_t/onUserInputReceived.hpp>
#include <kestrel/easy_ta2_plugin_t/openConnection.hpp>
#include <kestrel/easy_ta2_plugin_t/sendPackage.hpp>
#include <kestrel/easy_ta2_plugin_t/shutdown.hpp>
#include <kestrel/easy_ta2_plugin_t/verify_channel_id.hpp>

//----------------------------------------------------------------------

#endif // #ifndef KESTREL_EASY_TA2_PLUGIN_T_HPP
