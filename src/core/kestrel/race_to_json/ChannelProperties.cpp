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

#include <sst/catalog/to_string.hpp>

#include <ChannelProperties.h>

#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_direction_t.hpp>
#include <kestrel/send_type_t.hpp>
#include <kestrel/transmission_type_t.hpp>

namespace kestrel {

json_t race_to_json(ChannelProperties const & x) {
  return {
      {"bootstrap", x.bootstrap},
      {"channelGid", x.channelGid},
      {"channelStatus", channel_status_t(x.channelStatus)},
      {"connectionType", connection_type_t(x.connectionType)},
      {"creatorsPerLoader", sst::to_string(x.creatorsPerLoader)},
      {"duration_s", sst::to_string(x.duration_s)},
      {"intervalEndTime", sst::to_string(x.intervalEndTime)},
      {"isFlushable", x.isFlushable},
      {"linkDirection", link_direction_t(x.linkDirection)},
      {"loadersPerCreator", sst::to_string(x.loadersPerCreator)},
      {"maxLinks", sst::to_string(x.maxLinks)},
      {"maxSendsPerInterval", sst::to_string(x.maxSendsPerInterval)},
      {"mtu", sst::to_string(x.mtu)},
      {"multiAddressable", x.multiAddressable},
      {"period_s", sst::to_string(x.period_s)},
      {"reliable", x.reliable},
      {"secondsPerInterval", sst::to_string(x.secondsPerInterval)},
      {"sendType", send_type_t(x.sendType)},
      {"sendsRemainingInInterval",
       sst::to_string(x.sendsRemainingInInterval)},
      {"transmissionType", transmission_type_t(x.transmissionType)},
  };
};

} // namespace kestrel
