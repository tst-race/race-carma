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

// Include first to test independence.
#include <kestrel/ta1_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta1_plugin_t.hpp>
//

#include <string>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <SdkResponse.h>

#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>

namespace kestrel {

json_t ta1_plugin_t::i_notifyEpoch(std::string const & data) {
  return {
      {"data", data},
  };
}

json_t
ta1_plugin_t::i_onBootstrapPkgReceived(std::string const & persona,
                                       RawData const & pkg) {
  return {
      {"persona", persona},
  };
}

json_t ta1_plugin_t::i_onChannelStatusChanged(
    RaceHandle const & handle,
    std::string const & channelGid,
    ChannelStatus const & status,
    ChannelProperties const & properties) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"status", channel_status_t(status)},
      {"properties", race_to_json(properties)},
  };
}

json_t ta1_plugin_t::i_onConnectionStatusChanged(
    RaceHandle const handle,
    ConnectionID const & connId,
    ConnectionStatus const status,
    LinkID const & linkId,
    LinkProperties const & properties) {
  return {
      {"handle", race_handle_t(handle)},
      {"connId", connId},
      {"status", connection_status_t(status)},
      {"linkId", linkId},
      {"properties", race_to_json(properties)},
  };
}

json_t ta1_plugin_t::i_onLinkPropertiesChanged(
    LinkID const & linkId,
    LinkProperties const & linkProperties) {
  return {
      {"linkId", linkId},
      {"linkProperties", race_to_json(linkProperties)},
  };
}

json_t
ta1_plugin_t::i_onLinkStatusChanged(RaceHandle const handle,
                                    LinkID const & linkId,
                                    LinkStatus const status,
                                    LinkProperties const & properties) {
  return {
      {"handle", race_handle_t(handle)},
      {"linkId", linkId},
      {"status", link_status_t(status)},
      {"properties", race_to_json(properties)},
  };
}

json_t
ta1_plugin_t::i_onPackageStatusChanged(RaceHandle const & handle,
                                       PackageStatus const & status) {
  return {
      {"handle", race_handle_t(handle)},
      {"status", package_status_t(status)},
  };
}

json_t
ta1_plugin_t::i_onUserAcknowledgementReceived(RaceHandle const handle) {
  return {
      {"handle", race_handle_t(handle)},
  };
}

json_t
ta1_plugin_t::i_prepareToBootstrap(RaceHandle const handle,
                                   LinkID const & linkId,
                                   std::string const & configPath,
                                   DeviceInfo const & deviceInfo) {
  return {
      {"handle", race_handle_t(handle)},
      {"linkId", linkId},
      {"configPath", configPath},
      {"deviceInfo", race_to_json(deviceInfo)},
  };
}

json_t ta1_plugin_t::i_processClrMsg(RaceHandle const handle,
                                     ClrMsg const & msg) {
  return {
      {"handle", race_handle_t(handle)},
      {"msg", race_to_json(msg)},
  };
}

json_t ta1_plugin_t::i_shutdown() {
  return {};
}

} // namespace kestrel
