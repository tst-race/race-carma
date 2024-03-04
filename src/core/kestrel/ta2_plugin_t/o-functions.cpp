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
#include <kestrel/ta2_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta2_plugin_t.hpp>
//

#include <cstdint>
#include <string>
#include <vector>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkType.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>
#include <kestrel/race_to_json.hpp>

namespace kestrel {

json_t ta2_plugin_t::o_activateChannel(RaceHandle,
                                       std::string const &,
                                       std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_closeConnection(RaceHandle,
                                       ConnectionID const &) {
  return {};
}

json_t ta2_plugin_t::o_createBootstrapLink(RaceHandle,
                                           std::string const &,
                                           std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_createLink(RaceHandle, std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_createLinkFromAddress(RaceHandle,
                                             std::string const &,
                                             std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_deactivateChannel(RaceHandle,
                                         std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_destroyLink(RaceHandle, LinkID const &) {
  return {};
}

json_t ta2_plugin_t::o_flushChannel(RaceHandle,
                                    std::string const &,
                                    std::uint64_t) {
  return {};
}

json_t ta2_plugin_t::o_loadLinkAddress(RaceHandle,
                                       std::string const &,
                                       std::string const &) {
  return {};
}

json_t
ta2_plugin_t::o_loadLinkAddresses(RaceHandle,
                                  std::string const &,
                                  std::vector<std::string> const &) {
  return {};
}

json_t ta2_plugin_t::o_onUserAcknowledgementReceived(RaceHandle) {
  return {};
}

json_t ta2_plugin_t::o_onUserInputReceived(RaceHandle,
                                           bool,
                                           std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_openConnection(RaceHandle,
                                      LinkType,
                                      LinkID const &,
                                      std::string const &,
                                      std::int32_t) {
  return {};
}

json_t ta2_plugin_t::o_sendPackage(RaceHandle,
                                   ConnectionID const &,
                                   EncPkg const &,
                                   double,
                                   std::uint64_t) {
  return {};
}

json_t ta2_plugin_t::o_serveFiles(LinkID const &, std::string const &) {
  return {};
}

json_t ta2_plugin_t::o_xGetChannelGids(
    std::vector<std::string> const & channelGids) {
  return {
      {"channelGids", channelGids},
  };
}

json_t ta2_plugin_t::o_xGetChannelProperties(
    std::string const &,
    ChannelProperties const & channelProperties) {
  return {
      {"channelProperties", race_to_json(channelProperties)},
  };
}

} // namespace kestrel
