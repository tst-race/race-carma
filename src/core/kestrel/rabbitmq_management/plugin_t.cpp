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
#include <kestrel/rabbitmq_management/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq_management/plugin_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>

#include <ChannelStatus.h>
#include <IRaceSdkComms.h>

#include <kestrel/common_plugin_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/rabbitmq_management/KESTREL_RABBITMQ_MANAGEMENT_CHANNEL_PROPERTIES.hpp>
#include <kestrel/sst_from_nlohmann_json.hpp>

namespace kestrel {
namespace rabbitmq_management {

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

plugin_t::plugin_t(IRaceSdkComms & sdk)
    : common_plugin_t(sdk, "kestrel-rabbitmq_management"),
      easier_ta2_plugin_t(sdk, "kestrel-rabbitmq_management") {
  sst::json::get_to(sst::json::get_from_string<json_t>(
                        KESTREL_RABBITMQ_MANAGEMENT_CHANNEL_PROPERTIES),
                    this->mutable_properties());
  SST_ASSERT((properties().channelGid == this->sdk().plugin_name()));
  SST_ASSERT((properties().channelStatus == CHANNEL_ENABLED));
}

//----------------------------------------------------------------------

} // namespace rabbitmq_management
} // namespace kestrel
