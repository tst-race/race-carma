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
#include <kestrel/race_to_json.hpp>
// Include twice to test idempotence.
#include <kestrel/race_to_json.hpp>
//

#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>

#include <ChannelProperties.h>
#include <LinkProperties.h>
#include <SdkResponse.h>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_status_t.hpp>
#include <kestrel/transmission_type_t.hpp>

namespace kestrel {

nlohmann::json race_to_json(LinkProperties const & x) {
  return {
      {"link_type", link_type_t(x.linkType).to_json()},
      {"transmission_type", transmission_type_t(x.transmissionType)},
      {"connection_type", connection_type_t(x.connectionType)},
  };
}

nlohmann::json race_to_json(SdkResponse const & x) {
  return {
      {"handle", race_handle_t(x.handle)},
      {"queueUtilization", x.queueUtilization},
      {"status", sdk_status_t(x.status)},
  };
}

} // namespace kestrel
