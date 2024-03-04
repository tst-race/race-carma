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
#include <kestrel/ta2_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta2_sdk_t.hpp>
//

#include <cstdint>
#include <string>
#include <vector>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <PackageStatus.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/bootstrap_action_type_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/user_display_type_t.hpp>

namespace kestrel {

json_t ta2_sdk_t::i_displayBootstrapInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType,
    RaceEnums::BootstrapActionType const actionType) {
  return {
      {"data", data},
      {"displayType", user_display_type_t(displayType)},
      {"actionType", bootstrap_action_type_t(actionType)},
  };
}

json_t ta2_sdk_t::i_displayInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType) {
  return {
      {"data", data},
      {"displayType", user_display_type_t(displayType)},
  };
}

json_t ta2_sdk_t::i_generateConnectionId(LinkID const & linkId) {
  return {
      {"linkId", linkId},
  };
}

json_t ta2_sdk_t::i_generateLinkId(std::string const & channelGid) {
  return {
      {"channelGid", channelGid},
  };
}

json_t ta2_sdk_t::i_onChannelStatusChanged(
    RaceHandle const handle,
    std::string const & channelGid,
    ChannelStatus const status,
    ChannelProperties const & properties,
    std::int32_t const timeout) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"status", channel_status_t(status)},
      {"properties", race_to_json(properties)},
      {"timeout", timeout},
  };
}

json_t ta2_sdk_t::i_onConnectionStatusChanged(
    RaceHandle const handle,
    ConnectionID const & connId,
    ConnectionStatus const status,
    LinkProperties const & properties,
    std::int32_t const timeout) {
  return {
      {"handle", race_handle_t(handle)},
      {"connId", connId},
      {"status", connection_status_t(status)},
      {"properties", race_to_json(properties)},
      {"timeout", timeout},
  };
}

json_t
ta2_sdk_t::i_onLinkStatusChanged(RaceHandle const handle,
                                 LinkID const & linkId,
                                 LinkStatus const status,
                                 LinkProperties const & properties,
                                 std::int32_t const timeout) {
  return {
      {"handle", race_handle_t(handle)},
      {"linkId", linkId},
      {"status", link_status_t(status)},
      {"properties", race_to_json(properties)},
      {"timeout", timeout},
  };
}

json_t ta2_sdk_t::i_onPackageStatusChanged(RaceHandle const handle,
                                           PackageStatus const status,
                                           std::int32_t const timeout) {
  return {
      {"handle", race_handle_t(handle)},
      {"status", package_status_t(status)},
      {"timeout", timeout},
  };
}

json_t
ta2_sdk_t::i_receiveEncPkg(EncPkg const & pkg,
                           std::vector<ConnectionID> const & connIDs,
                           std::int32_t const timeout) {
  return {
      {"pkg", race_to_json(pkg)},
      {"connIDs", connIDs},
      {"timeout", timeout},
  };
}

json_t ta2_sdk_t::i_requestCommonUserInput(std::string const & key) {
  return {
      {"key", key},
  };
}

json_t ta2_sdk_t::i_requestPluginUserInput(std::string const & key,
                                           std::string const & prompt,
                                           bool const cache) {
  return {
      {"key", key},
      {"prompt", prompt},
      {"cache", cache},
  };
}

json_t
ta2_sdk_t::i_updateLinkProperties(LinkID const & linkId,
                                  LinkProperties const & properties,
                                  std::int32_t const timeout) {
  return {
      {"linkId", linkId},
      {"properties", race_to_json(properties)},
      {"timeout", timeout},
  };
}

} // namespace kestrel
