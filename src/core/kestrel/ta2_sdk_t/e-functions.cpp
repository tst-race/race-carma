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
#include <ConnectionStatus.h>
#include <EncPkg.h>
#include <IRaceSdkComms.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_INNER_E_FOO.hpp>

namespace kestrel {

namespace {

CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, displayBootstrapInfoToUser)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, displayInfoToUser)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, generateConnectionId)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, generateLinkId)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, onChannelStatusChanged)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, onConnectionStatusChanged)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, onLinkStatusChanged)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, onPackageStatusChanged)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, receiveEncPkg)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, requestCommonUserInput)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, requestPluginUserInput)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkComms, updateLinkProperties)

} // namespace

SdkResponse ta2_sdk_t::e_displayBootstrapInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType,
    RaceEnums::BootstrapActionType const actionType) {
  return inner_e_displayBootstrapInfoToUser(sdk(),
                                            data,
                                            displayType,
                                            actionType);
}

SdkResponse ta2_sdk_t::e_displayInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType) {
  return inner_e_displayInfoToUser(sdk(), data, displayType);
}

ConnectionID ta2_sdk_t::e_generateConnectionId(LinkID const & linkId) {
  return inner_e_generateConnectionId(sdk(), linkId);
}

LinkID ta2_sdk_t::e_generateLinkId(std::string const & channelGid) {
  return inner_e_generateLinkId(sdk(), channelGid);
}

SdkResponse ta2_sdk_t::e_onChannelStatusChanged(
    RaceHandle const handle,
    std::string const & channelGid,
    ChannelStatus const status,
    ChannelProperties const & properties,
    std::int32_t const timeout) {
  return inner_e_onChannelStatusChanged(sdk(),
                                        handle,
                                        channelGid,
                                        status,
                                        properties,
                                        timeout);
}

SdkResponse ta2_sdk_t::e_onConnectionStatusChanged(
    RaceHandle const handle,
    ConnectionID const & connId,
    ConnectionStatus const status,
    LinkProperties const & properties,
    std::int32_t const timeout) {
  return inner_e_onConnectionStatusChanged(sdk(),
                                           handle,
                                           connId,
                                           status,
                                           properties,
                                           timeout);
}

SdkResponse
ta2_sdk_t::e_onLinkStatusChanged(RaceHandle const handle,
                                 LinkID const & linkId,
                                 LinkStatus const status,
                                 LinkProperties const & properties,
                                 std::int32_t const timeout) {
  return inner_e_onLinkStatusChanged(sdk(),
                                     handle,
                                     linkId,
                                     status,
                                     properties,
                                     timeout);
}

SdkResponse
ta2_sdk_t::e_onPackageStatusChanged(RaceHandle const handle,
                                    PackageStatus const status,
                                    std::int32_t const timeout) {
  return inner_e_onPackageStatusChanged(sdk(), handle, status, timeout);
}

SdkResponse
ta2_sdk_t::e_receiveEncPkg(EncPkg const & pkg,
                           std::vector<ConnectionID> const & connIDs,
                           std::int32_t const timeout) {
  return inner_e_receiveEncPkg(sdk(), pkg, connIDs, timeout);
}

SdkResponse
ta2_sdk_t::e_requestCommonUserInput(std::string const & key) {
  return inner_e_requestCommonUserInput(sdk(), key);
}

SdkResponse
ta2_sdk_t::e_requestPluginUserInput(std::string const & key,
                                    std::string const & prompt,
                                    bool const cache) {
  return inner_e_requestPluginUserInput(sdk(), key, prompt, cache);
}

SdkResponse
ta2_sdk_t::e_updateLinkProperties(LinkID const & linkId,
                                  LinkProperties const & properties,
                                  std::int32_t const timeout) {
  return inner_e_updateLinkProperties(sdk(),
                                      linkId,
                                      properties,
                                      timeout);
}

} // namespace kestrel
