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

#ifndef KESTREL_TA1_PLUGIN_T_HPP
#define KESTREL_TA1_PLUGIN_T_HPP

#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ConnectionStatus.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <IRacePluginNM.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <PackageStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

//
// TODO: This should eventually derive from common_plugin_t<ta1_sdk_t>.
//       Right now it's using sdk_wrapper_t, which is essentially the
//       old version of ta1_sdk_t.
//

class ta1_plugin_t : public virtual common_plugin_t<sdk_wrapper_t>,
                     public IRacePluginNM {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  explicit ta1_plugin_t(IRaceSdkNM & sdk,
                        std::string const & plugin_name)
      : common_plugin_t(sdk, plugin_name) {
  }

  ta1_plugin_t(ta1_plugin_t const &) = delete;
  ta1_plugin_t & operator=(ta1_plugin_t const &) = delete;
  ta1_plugin_t(ta1_plugin_t &&) = delete;
  ta1_plugin_t & operator=(ta1_plugin_t &&) = delete;
  virtual ~ta1_plugin_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each plugin function foo, we define an input description
  // function i_foo that returns a JSON object that describes the input
  // to foo.
  //

protected:

  static json_t i_notifyEpoch(std::string const & data);

  static json_t i_onBootstrapPkgReceived(std::string const & persona,
                                         RawData const & pkg);

  static json_t
  i_onChannelStatusChanged(RaceHandle const & handle,
                           std::string const & channelGid,
                           ChannelStatus const & status,
                           ChannelProperties const & properties);

  static json_t
  i_onConnectionStatusChanged(RaceHandle handle,
                              ConnectionID const & connId,
                              ConnectionStatus status,
                              LinkID const & linkId,
                              LinkProperties const & properties);

  static json_t
  i_onLinkPropertiesChanged(LinkID const & linkId,
                            LinkProperties const & linkProperties);

  static json_t
  i_onLinkStatusChanged(RaceHandle handle,
                        LinkID const & linkId,
                        LinkStatus status,
                        LinkProperties const & properties);

  static json_t i_onPackageStatusChanged(RaceHandle const & handle,
                                         PackageStatus const & status);

  static json_t i_onUserAcknowledgementReceived(RaceHandle handle);

  static json_t i_prepareToBootstrap(RaceHandle handle,
                                     LinkID const & linkId,
                                     std::string const & configPath,
                                     DeviceInfo const & deviceInfo);

  static json_t i_processClrMsg(RaceHandle handle, ClrMsg const & msg);

  static json_t i_shutdown();

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each plugin function foo, we define an output description
  // function o_foo that returns a JSON object that describes the output
  // of foo.
  //

protected:

  static json_t o_notifyEpoch(std::string const & data);

  static json_t o_onBootstrapPkgReceived(std::string const & persona,
                                         RawData const & pkg);

  static json_t
  o_onChannelStatusChanged(RaceHandle const & handle,
                           std::string const & channelGid,
                           ChannelStatus const & status,
                           ChannelProperties const & properties);

  static json_t
  o_onConnectionStatusChanged(RaceHandle handle,
                              ConnectionID const & connId,
                              ConnectionStatus status,
                              LinkID const & linkId,
                              LinkProperties const & properties);

  static json_t
  o_onLinkPropertiesChanged(LinkID const & linkId,
                            LinkProperties const & linkProperties);

  static json_t
  o_onLinkStatusChanged(RaceHandle handle,
                        LinkID const & linkId,
                        LinkStatus status,
                        LinkProperties const & properties);

  static json_t o_onPackageStatusChanged(RaceHandle const & handle,
                                         PackageStatus const & status);

  static json_t o_onUserAcknowledgementReceived(RaceHandle handle);

  static json_t o_prepareToBootstrap(RaceHandle handle,
                                     LinkID const & linkId,
                                     std::string const & configPath,
                                     DeviceInfo const & deviceInfo);

  static json_t o_processClrMsg(RaceHandle handle, ClrMsg const & msg);

  static json_t o_shutdown();

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------
  //
  // If you're inheriting from this class to implement a TA1 plugin,
  // these are the functions you need to implement. You also need to
  // implement the inner_init and inner_shutdown functions, which are
  // declared in common_plugin_t.
  //
  // Each of these functions has a default implementation that simply
  // throws an exception.
  //

protected:

  virtual void inner_notifyEpoch(tracing_event_t tev,
                                 std::string const & data);

  virtual void inner_onBootstrapPkgReceived(tracing_event_t tev,
                                            psn_t const & persona,
                                            RawData const & pkg);

  virtual void
  inner_onChannelStatusChanged(tracing_event_t tev,
                               race_handle_t const & handle,
                               channel_id_t const & channelGid,
                               channel_status_t const & status,
                               ChannelProperties const & properties);

  virtual void
  inner_onConnectionStatusChanged(tracing_event_t tev,
                                  race_handle_t const & handle,
                                  connection_id_t const & connId,
                                  connection_status_t status,
                                  link_id_t const & linkId,
                                  LinkProperties const & properties);

  virtual void
  inner_onLinkPropertiesChanged(tracing_event_t tev,
                                link_id_t const & linkId,
                                LinkProperties const & linkProperties);

  virtual void
  inner_onLinkStatusChanged(tracing_event_t tev,
                            race_handle_t const & handle,
                            link_id_t const & linkId,
                            link_status_t const & status,
                            LinkProperties const & properties);

  virtual void
  inner_onPackageStatusChanged(tracing_event_t tev,
                               race_handle_t const & handle,
                               package_status_t const & status);

  virtual void
  inner_onUserAcknowledgementReceived(tracing_event_t tev,
                                      race_handle_t const & handle);

  virtual void inner_prepareToBootstrap(tracing_event_t tev,
                                        race_handle_t const & handle,
                                        link_id_t const & linkId,
                                        std::string const & configPath,
                                        DeviceInfo const & deviceInfo);

  virtual void inner_processClrMsg(tracing_event_t tev,
                                   race_handle_t const & handle,
                                   ClrMsg const & msg);

  //--------------------------------------------------------------------
  // Outer plugin functions
  //--------------------------------------------------------------------

public:

  PluginResponse init(PluginConfig const & pluginConfig) override;

  PluginResponse notifyEpoch(std::string const & data) override;

  PluginResponse onBootstrapPkgReceived(std::string persona,
                                        RawData pkg) override;

  PluginResponse
  onChannelStatusChanged(RaceHandle handle,
                         std::string channelGid,
                         ChannelStatus status,
                         ChannelProperties properties) override;

  PluginResponse
  onConnectionStatusChanged(RaceHandle handle,
                            ConnectionID connId,
                            ConnectionStatus status,
                            LinkID linkId,
                            LinkProperties properties) override;

  PluginResponse
  onLinkPropertiesChanged(LinkID linkId,
                          LinkProperties linkProperties) override;

  PluginResponse
  onLinkStatusChanged(RaceHandle handle,
                      LinkID linkId,
                      LinkStatus status,
                      LinkProperties properties) override;

  PluginResponse onPackageStatusChanged(RaceHandle handle,
                                        PackageStatus status) override;

  PluginResponse
  onUserAcknowledgementReceived(RaceHandle handle) override;

  PluginResponse prepareToBootstrap(RaceHandle handle,
                                    LinkID linkId,
                                    std::string configPath,
                                    DeviceInfo deviceInfo) override;

  PluginResponse processClrMsg(RaceHandle handle,
                               ClrMsg const & msg) override;

  PluginResponse shutdown() override;

  //--------------------------------------------------------------------
  // Outer plugin function names
  //--------------------------------------------------------------------

protected:

  static char const func_init[];
  static char const func_notifyEpoch[];
  static char const func_onBootstrapPkgReceived[];
  static char const func_onChannelStatusChanged[];
  static char const func_onConnectionStatusChanged[];
  static char const func_onLinkPropertiesChanged[];
  static char const func_onLinkStatusChanged[];
  static char const func_onPackageStatusChanged[];
  static char const func_onUserAcknowledgementReceived[];
  static char const func_prepareToBootstrap[];
  static char const func_processClrMsg[];
  static char const func_shutdown[];

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA1_PLUGIN_T_HPP
