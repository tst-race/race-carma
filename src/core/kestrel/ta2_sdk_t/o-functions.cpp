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
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>
#include <kestrel/race_to_json.hpp>

namespace kestrel {

json_t ta2_sdk_t::o_displayBootstrapInfoToUser(
    SdkResponse const & return_value,
    std::string const &,
    RaceEnums::UserDisplayType,
    RaceEnums::BootstrapActionType) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t ta2_sdk_t::o_displayInfoToUser(SdkResponse const & return_value,
                                      std::string const &,
                                      RaceEnums::UserDisplayType) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_generateConnectionId(ConnectionID const & return_value,
                                  LinkID const &) {
  return {
      {"return_value", return_value},
  };
}

json_t ta2_sdk_t::o_generateLinkId(LinkID const & return_value,
                                   std::string const &) {
  return {
      {"return_value", return_value},
  };
}

json_t
ta2_sdk_t::o_onChannelStatusChanged(SdkResponse const & return_value,
                                    RaceHandle,
                                    std::string const &,
                                    ChannelStatus,
                                    ChannelProperties const &,
                                    std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_onConnectionStatusChanged(SdkResponse const & return_value,
                                       RaceHandle,
                                       ConnectionID const &,
                                       ConnectionStatus,
                                       LinkProperties const &,
                                       std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_onLinkStatusChanged(SdkResponse const & return_value,
                                 RaceHandle,
                                 LinkID const &,
                                 LinkStatus,
                                 LinkProperties const &,
                                 std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_onPackageStatusChanged(SdkResponse const & return_value,
                                    RaceHandle,
                                    PackageStatus,
                                    std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t ta2_sdk_t::o_receiveEncPkg(SdkResponse const & return_value,
                                  EncPkg const &,
                                  std::vector<ConnectionID> const &,
                                  std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_requestCommonUserInput(SdkResponse const & return_value,
                                    std::string const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_requestPluginUserInput(SdkResponse const & return_value,
                                    std::string const &,
                                    std::string const &,
                                    bool) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
ta2_sdk_t::o_updateLinkProperties(SdkResponse const & return_value,
                                  LinkID const &,
                                  LinkProperties const &,
                                  std::int32_t) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

} // namespace kestrel
