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
#include <kestrel/ta1_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta1_sdk_t.hpp>
//

#include <cstdint>
#include <string>
#include <vector>

#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_INNER_E_FOO.hpp>

namespace kestrel {

CARMA_DEFINE_INNER_E_FOO(IRaceSdkNM, bootstrapDevice)

SdkResponse ta1_sdk_t::e_bootstrapDevice(
    RaceHandle const handle,
    std::vector<std::string> const & ta2Channels) {
  return inner_e_bootstrapDevice(sdk(), handle, ta2Channels);
}

CARMA_DEFINE_INNER_E_FOO(IRaceSdkNM, displayInfoToUser)

SdkResponse ta1_sdk_t::e_displayInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType) {
  return inner_e_displayInfoToUser(sdk(), data, displayType);
}

CARMA_DEFINE_INNER_E_FOO(IRaceSdkNM, sendBootstrapPkg)

SdkResponse
ta1_sdk_t::e_sendBootstrapPkg(ConnectionID const & connectionId,
                              std::string const & persona,
                              RawData const & pkg,
                              std::int32_t const timeout) {
  return inner_e_sendBootstrapPkg(sdk(),
                                  connectionId,
                                  persona,
                                  pkg,
                                  timeout);
}

} // namespace kestrel
