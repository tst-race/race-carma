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

#ifndef KESTREL_TA2_PLUGIN_T_HPP
#define KESTREL_TA2_PLUGIN_T_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <IRacePluginComms.h>
#include <IRaceSdkComms.h>
#include <LinkType.h>
#include <PluginConfig.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class ta2_plugin_t : public virtual common_plugin_t<ta2_sdk_t>,
                     public IRacePluginComms {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  explicit ta2_plugin_t(IRaceSdkComms & sdk,
                        std::string const & plugin_name)
      : common_plugin_t(sdk, plugin_name) {
  }

  ta2_plugin_t(ta2_plugin_t const &) = delete;
  ta2_plugin_t & operator=(ta2_plugin_t const &) = delete;
  ta2_plugin_t(ta2_plugin_t &&) = delete;
  ta2_plugin_t & operator=(ta2_plugin_t &&) = delete;
  virtual ~ta2_plugin_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each plugin function foo, we define an input description
  // function i_foo that returns a JSON object that describes the input
  // to foo.
  //

protected:

  static json_t i_activateChannel(RaceHandle handle,
                                  std::string const & channelGid,
                                  std::string const & roleName);

  static json_t i_closeConnection(RaceHandle handle,
                                  ConnectionID const & connectionId);

  static json_t i_createBootstrapLink(RaceHandle handle,
                                      std::string const & channelGid,
                                      std::string const & passphrase);

  static json_t i_createLink(RaceHandle handle,
                             std::string const & channelGid);

  static json_t
  i_createLinkFromAddress(RaceHandle handle,
                          std::string const & channelGid,
                          std::string const & linkAddress);

  static json_t i_deactivateChannel(RaceHandle handle,
                                    std::string const & channelGid);

  static json_t i_destroyLink(RaceHandle handle, LinkID const & linkId);

  static json_t i_flushChannel(RaceHandle handle,
                               std::string const & channelGid,
                               std::uint64_t batchId);

  static json_t i_loadLinkAddress(RaceHandle handle,
                                  std::string const & channelGid,
                                  std::string const & linkAddress);

  static json_t
  i_loadLinkAddresses(RaceHandle handle,
                      std::string const & channelGid,
                      std::vector<std::string> const & linkAddresses);

  static json_t i_onUserAcknowledgementReceived(RaceHandle handle);

  static json_t i_onUserInputReceived(RaceHandle handle,
                                      bool answered,
                                      std::string const & response);

  static json_t i_openConnection(RaceHandle handle,
                                 LinkType linkType,
                                 LinkID const & linkId,
                                 std::string const & linkHints,
                                 std::int32_t sendTimeout);

  static json_t i_sendPackage(RaceHandle handle,
                              ConnectionID const & connectionId,
                              EncPkg const & pkg,
                              double timeoutTimestamp,
                              std::uint64_t batchId);

  static json_t i_serveFiles(LinkID const & linkId,
                             std::string const & path);

  static json_t
  i_xGetChannelGids(std::vector<std::string> const & channelGids);

  static json_t
  i_xGetChannelProperties(std::string const & channelGid,
                          ChannelProperties const & channelProperties);

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each plugin function foo, we define an output description
  // function o_foo that returns a JSON object that describes the output
  // of foo.
  //

protected:

  static json_t o_activateChannel(RaceHandle handle,
                                  std::string const & channelGid,
                                  std::string const & roleName);

  static json_t o_closeConnection(RaceHandle handle,
                                  ConnectionID const & connectionId);

  static json_t o_createBootstrapLink(RaceHandle handle,
                                      std::string const & channelGid,
                                      std::string const & passphrase);

  static json_t o_createLink(RaceHandle handle,
                             std::string const & channelGid);

  static json_t
  o_createLinkFromAddress(RaceHandle handle,
                          std::string const & channelGid,
                          std::string const & linkAddress);

  static json_t o_deactivateChannel(RaceHandle handle,
                                    std::string const & channelGid);

  static json_t o_destroyLink(RaceHandle handle, LinkID const & linkId);

  static json_t o_flushChannel(RaceHandle handle,
                               std::string const & channelGid,
                               std::uint64_t batchId);

  static json_t o_loadLinkAddress(RaceHandle handle,
                                  std::string const & channelGid,
                                  std::string const & linkAddress);

  static json_t
  o_loadLinkAddresses(RaceHandle handle,
                      std::string const & channelGid,
                      std::vector<std::string> const & linkAddresses);

  static json_t o_onUserAcknowledgementReceived(RaceHandle handle);

  static json_t o_onUserInputReceived(RaceHandle handle,
                                      bool answered,
                                      std::string const & response);

  static json_t o_openConnection(RaceHandle handle,
                                 LinkType linkType,
                                 LinkID const & linkId,
                                 std::string const & linkHints,
                                 std::int32_t sendTimeout);

  static json_t o_sendPackage(RaceHandle handle,
                              ConnectionID const & connectionId,
                              EncPkg const & pkg,
                              double timeoutTimestamp,
                              std::uint64_t batchId);

  static json_t o_serveFiles(LinkID const & linkId,
                             std::string const & path);

  static json_t
  o_xGetChannelGids(std::vector<std::string> const & channelGids);

  static json_t
  o_xGetChannelProperties(std::string const & channelGid,
                          ChannelProperties const & channelProperties);

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------
  //
  // If you're inheriting from this class to implement a TA2 plugin,
  // these are the functions you need to implement. You also need to
  // implement the inner_shutdown function, which is declared in the
  // common_plugin_t class.
  //

protected:

  virtual void inner_activateChannel(tracing_event_t tev,
                                     race_handle_t const & handle,
                                     channel_id_t const & channelGid,
                                     std::string const & roleName);

  virtual void
  inner_closeConnection(tracing_event_t tev,
                        race_handle_t const & handle,
                        connection_id_t const & connectionId);

  virtual void
  inner_createBootstrapLink(tracing_event_t tev,
                            race_handle_t const & handle,
                            channel_id_t const & channelGid,
                            std::string const & passphrase);

  virtual void inner_createLink(tracing_event_t tev,
                                race_handle_t const & handle,
                                channel_id_t const & channelGid);

  virtual void
  inner_createLinkFromAddress(tracing_event_t tev,
                              race_handle_t const & handle,
                              channel_id_t const & channelGid,
                              link_address_t const & linkAddress);

  virtual void inner_deactivateChannel(tracing_event_t tev,
                                       race_handle_t const & handle,
                                       channel_id_t const & channelGid);

  virtual void inner_destroyLink(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 link_id_t const & linkId);

  virtual void inner_flushChannel(tracing_event_t tev,
                                  race_handle_t const & handle,
                                  channel_id_t const & channelGid,
                                  std::uint64_t batchId);

  void inner_init(tracing_event_t tev,
                  PluginConfig const & pluginConfig) override;

  virtual void
  inner_loadLinkAddress(tracing_event_t tev,
                        race_handle_t const & handle,
                        channel_id_t const & channelGid,
                        link_address_t const & linkAddress);

  virtual void inner_loadLinkAddresses(
      tracing_event_t tev,
      race_handle_t const & handle,
      channel_id_t const & channelGid,
      std::vector<std::string> const & linkAddresses);

  virtual void
  inner_onUserAcknowledgementReceived(tracing_event_t tev,
                                      race_handle_t const & handle);

  virtual void inner_onUserInputReceived(tracing_event_t tev,
                                         race_handle_t const & handle,
                                         bool answered,
                                         std::string const & response);

  virtual void inner_openConnection(tracing_event_t tev,
                                    race_handle_t const & handle,
                                    link_type_t const & linkType,
                                    link_id_t const & linkId,
                                    std::string const & linkHints,
                                    std::int32_t sendTimeout);

  virtual void inner_sendPackage(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 connection_id_t const & connectionId,
                                 RawData && pkg,
                                 double timeoutTimestamp,
                                 std::uint64_t batchId);

  virtual void inner_serveFiles(tracing_event_t tev,
                                link_id_t const & linkId,
                                std::string const & path);

  //
  // Sets channelGids to the list of all channel GIDs provided by this
  // plugin.
  //

  virtual void
  inner_xGetChannelGids(tracing_event_t tev,
                        std::vector<std::string> & channelGids);

  //
  // Sets channelProperties to the properties of the channel identified
  // by channelGid.
  //

  virtual void
  inner_xGetChannelProperties(tracing_event_t tev,
                              std::string const & channelGid,
                              ChannelProperties & channelProperties);

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

  PluginResponse init(PluginConfig const & pluginConfig) override;

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

  PluginResponse serveFiles(LinkID linkId, std::string path) override;

  PluginResponse shutdown() override;

  //--------------------------------------------------------------------
  // Additional outer plugin functions
  //--------------------------------------------------------------------

public:

  virtual PluginResponse
  xGetChannelGids(std::vector<std::string> & channelGids);

  virtual PluginResponse
  xGetChannelProperties(std::string const & channelGid,
                        ChannelProperties & channelProperties);

  //--------------------------------------------------------------------
  // Outer plugin function names
  //--------------------------------------------------------------------

protected:

  static char const func_activateChannel[];
  static char const func_closeConnection[];
  static char const func_createBootstrapLink[];
  static char const func_createLink[];
  static char const func_createLinkFromAddress[];
  static char const func_deactivateChannel[];
  static char const func_destroyLink[];
  static char const func_flushChannel[];
  static char const func_init[];
  static char const func_loadLinkAddress[];
  static char const func_loadLinkAddresses[];
  static char const func_onUserAcknowledgementReceived[];
  static char const func_onUserInputReceived[];
  static char const func_openConnection[];
  static char const func_sendPackage[];
  static char const func_serveFiles[];
  static char const func_shutdown[];
  static char const func_xGetChannelGids[];
  static char const func_xGetChannelProperties[];

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA2_PLUGIN_T_HPP
