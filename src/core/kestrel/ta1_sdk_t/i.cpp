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

#include <sst/catalog/to_string.hpp>

#include <EncPkg.h>
#include <LinkProperties.h>
#include <RaceEnums.h>

#include <kestrel/json_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/user_display_type_t.hpp>

namespace kestrel {

json_t ta1_sdk_t::i_bootstrapDevice(
    RaceHandle handle,
    std::vector<std::string> const & ta2Channels) {
  return {
      {"handle", race_handle_t(handle)},
      {"ta2Channels", ta2Channels},
  };
}

json_t ta1_sdk_t::i_displayInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType const displayType) {
  return {
      {"data", data},
      {"displayType", user_display_type_t(displayType)},
  };
}

json_t ta1_sdk_t::i_sendBootstrapPkg(ConnectionID const & connectionId,
                                     std::string const & persona,
                                     RawData const & pkg,
                                     std::int32_t const timeout) {
  return {
      {"connectionId", connectionId},
      {"persona", persona},
      {"timeout", sst::to_string(timeout)},
  };
}

} // namespace kestrel
