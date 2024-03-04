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
#include <kestrel/rabbitmq/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/plugin_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <sst/catalog/json/get_from_string.hpp>

#include <ChannelProperties.h>

#include <kestrel/json_t.hpp>
#include <kestrel/rabbitmq/KESTREL_RABBITMQ_CHANNEL_PROPERTIES.hpp>
#include <kestrel/sst_from_nlohmann_json.hpp>

namespace kestrel {
namespace rabbitmq {

//----------------------------------------------------------------------
// Channel properties
//----------------------------------------------------------------------

ChannelProperties plugin_t::initial_properties() {
  return sst::json::get_from_string<ChannelProperties, json_t>(
      KESTREL_RABBITMQ_CHANNEL_PROPERTIES);
}

//----------------------------------------------------------------------

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
