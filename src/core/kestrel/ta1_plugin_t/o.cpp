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

#include <kestrel/json_t.hpp>

namespace kestrel {

json_t ta1_plugin_t::o_notifyEpoch(std::string const &) {
  return {};
}

json_t ta1_plugin_t::o_onBootstrapPkgReceived(std::string const &,
                                              RawData const &) {
  return {};
}

json_t
ta1_plugin_t::o_onChannelStatusChanged(RaceHandle const &,
                                       std::string const &,
                                       ChannelStatus const &,
                                       ChannelProperties const &) {
  return {};
}

json_t
ta1_plugin_t::o_onConnectionStatusChanged(RaceHandle,
                                          ConnectionID const &,
                                          ConnectionStatus,
                                          LinkID const &,
                                          LinkProperties const &) {
  return {};
}

json_t ta1_plugin_t::o_onLinkPropertiesChanged(LinkID const &,
                                               LinkProperties const &) {
  return {};
}

json_t ta1_plugin_t::o_onLinkStatusChanged(RaceHandle,
                                           LinkID const &,
                                           LinkStatus,
                                           LinkProperties const &) {
  return {};
}

json_t ta1_plugin_t::o_onPackageStatusChanged(RaceHandle const &,
                                              PackageStatus const &) {
  return {};
}

json_t ta1_plugin_t::o_onUserAcknowledgementReceived(RaceHandle) {
  return {};
}

json_t ta1_plugin_t::o_prepareToBootstrap(RaceHandle,
                                          LinkID const &,
                                          std::string const &,
                                          DeviceInfo const &) {
  return {};
}

json_t ta1_plugin_t::o_processClrMsg(RaceHandle, ClrMsg const &) {
  return {};
}

json_t ta1_plugin_t::o_shutdown() {
  return {};
}

} // namespace kestrel
