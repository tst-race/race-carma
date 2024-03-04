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
#include <kestrel/sst_from_nlohmann_json.hpp>
// Include twice to test idempotence.
#include <kestrel/sst_from_nlohmann_json.hpp>
//

#include <sst/catalog/json/remove_as.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>

#include <ChannelProperties.h>

#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_direction_t.hpp>
#include <kestrel/send_type_t.hpp>
#include <kestrel/transmission_type_t.hpp>

using namespace kestrel;

void sst_from_nlohmann_json(json_t src, ChannelProperties & dst) {
  sst::json::remove_to(src, dst.bootstrap, "bootstrap");
  sst::json::remove_to(src, dst.channelGid, "channelGid");
  dst.channelStatus =
      sst::json::remove_as<channel_status_t>(src, "channelStatus")
          .value();
  dst.connectionType =
      sst::json::remove_as<connection_type_t>(src, "connectionType")
          .value();
  sst::json::remove_to(src, dst.creatorExpected, "creatorExpected");
  sst::json::remove_to(src, dst.creatorsPerLoader, "creatorsPerLoader");
  sst::json::remove_to(src, dst.currentRole, "currentRole");
  src.erase("description");
  sst::json::remove_to(src, dst.duration_s, "duration_s");
  sst::json::remove_to(src, dst.intervalEndTime, "intervalEndTime");
  sst::json::remove_to(src, dst.isFlushable, "isFlushable");
  dst.linkDirection =
      sst::json::remove_as<link_direction_t>(src, "linkDirection")
          .value();
  sst::json::remove_to(src, dst.loaderExpected, "loaderExpected");
  sst::json::remove_to(src, dst.loadersPerCreator, "loadersPerCreator");
  sst::json::remove_to(src, dst.maxLinks, "maxLinks");
  sst::json::remove_to(src,
                       dst.maxSendsPerInterval,
                       "maxSendsPerInterval");
  sst::json::remove_to(src, dst.mtu, "mtu");
  sst::json::remove_to(src, dst.multiAddressable, "multiAddressable");
  sst::json::remove_to(src, dst.period_s, "period_s");
  sst::json::remove_to(src, dst.reliable, "reliable");
  sst::json::remove_to(src, dst.roles, "roles");
  sst::json::remove_to(src,
                       dst.secondsPerInterval,
                       "secondsPerInterval");
  sst::json::remove_to(src,
                       dst.sendsRemainingInInterval,
                       "sendsRemainingInInterval");
  dst.sendType =
      sst::json::remove_as<send_type_t>(src, "sendType").value();
  sst::json::remove_to(src, dst.supported_hints, "supported_hints");
  dst.transmissionType =
      sst::json::remove_as<transmission_type_t>(src, "transmissionType")
          .value();
  sst::json::unknown_key(src);
}
