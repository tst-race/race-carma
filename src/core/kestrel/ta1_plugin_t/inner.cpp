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

#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>

#include <ChannelProperties.h>
#include <ClrMsg.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <LinkProperties.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void ta1_plugin_t::inner_notifyEpoch(tracing_event_t tev,
                                     std::string const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onBootstrapPkgReceived(tracing_event_t tev,
                                                psn_t const &,
                                                RawData const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onChannelStatusChanged(
    tracing_event_t tev,
    race_handle_t const &,
    channel_id_t const &,
    channel_status_t const &,
    ChannelProperties const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onConnectionStatusChanged(
    tracing_event_t tev,
    race_handle_t const &,
    connection_id_t const &,
    connection_status_t,
    link_id_t const &,
    LinkProperties const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onLinkPropertiesChanged(
    tracing_event_t tev,
    link_id_t const &,
    LinkProperties const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onLinkStatusChanged(tracing_event_t tev,
                                             race_handle_t const &,
                                             link_id_t const &,
                                             link_status_t const &,
                                             LinkProperties const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onPackageStatusChanged(
    tracing_event_t tev,
    race_handle_t const &,
    package_status_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_onUserAcknowledgementReceived(
    tracing_event_t tev,
    race_handle_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_prepareToBootstrap(tracing_event_t tev,
                                            race_handle_t const &,
                                            link_id_t const &,
                                            std::string const &,
                                            DeviceInfo const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta1_plugin_t::inner_processClrMsg(tracing_event_t tev,
                                       race_handle_t const &,
                                       ClrMsg const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

} // namespace kestrel
