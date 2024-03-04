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

#include <sst/catalog/SST_ASSERT.h>

#include <ChannelStatus.h>
#include <IRaceSdkComms.h>

#include <kestrel/common_plugin_t.hpp>

namespace kestrel {
namespace rabbitmq {

plugin_t::plugin_t(IRaceSdkComms & sdk)
    : common_plugin_t(sdk, "kestrel-rabbitmq"),
      easy_ta2_plugin_t(sdk, "kestrel-rabbitmq") {
  this->mutable_properties() = plugin_t::initial_properties();
  SST_ASSERT(properties().channelGid == this->sdk().plugin_name());
  SST_ASSERT(properties().channelStatus == CHANNEL_ENABLED);
} //

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
