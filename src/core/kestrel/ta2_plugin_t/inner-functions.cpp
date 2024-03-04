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
#include <kestrel/ta2_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta2_plugin_t.hpp>
//

#include <cstdint>
#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void ta2_plugin_t::inner_activateChannel(tracing_event_t,
                                         race_handle_t const &,
                                         channel_id_t const &,
                                         std::string const &) {
  ;
}

void ta2_plugin_t::inner_closeConnection(tracing_event_t tev,
                                         race_handle_t const &,
                                         connection_id_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_createBootstrapLink(tracing_event_t tev,
                                             race_handle_t const &,
                                             channel_id_t const &,
                                             std::string const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_createLink(tracing_event_t tev,
                                    race_handle_t const &,
                                    channel_id_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_createLinkFromAddress(
    tracing_event_t tev,
    race_handle_t const & handle,
    channel_id_t const & channelGid,
    link_address_t const & linkAddress) {
  SST_TEV_TOP(tev);
  inner_loadLinkAddress(SST_TEV_ARG(tev),
                        handle,
                        channelGid,
                        linkAddress);
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_deactivateChannel(tracing_event_t tev,
                                           race_handle_t const &,
                                           channel_id_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_destroyLink(tracing_event_t tev,
                                     race_handle_t const &,
                                     link_id_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_flushChannel(tracing_event_t tev,
                                      race_handle_t const &,
                                      channel_id_t const &,
                                      std::uint64_t) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_init(tracing_event_t, PluginConfig const &) {
  ;
}

void ta2_plugin_t::inner_loadLinkAddress(tracing_event_t tev,
                                         race_handle_t const &,
                                         channel_id_t const &,
                                         link_address_t const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_loadLinkAddresses(
    tracing_event_t tev,
    race_handle_t const &,
    channel_id_t const &,
    std::vector<std::string> const &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_onUserAcknowledgementReceived(
    tracing_event_t,
    race_handle_t const &) {
  ;
}

void ta2_plugin_t::inner_onUserInputReceived(tracing_event_t,
                                             race_handle_t const &,
                                             bool,
                                             std::string const &) {
  ;
}

void ta2_plugin_t::inner_openConnection(tracing_event_t tev,
                                        race_handle_t const &,
                                        link_type_t const &,
                                        link_id_t const &,
                                        std::string const &,
                                        std::int32_t) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_sendPackage(tracing_event_t tev,
                                     race_handle_t const &,
                                     connection_id_t const &,
                                     RawData &&,
                                     double,
                                     std::uint64_t) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_serveFiles(tracing_event_t,
                                    link_id_t const &,
                                    std::string const &) {
  ;
}

void ta2_plugin_t::inner_xGetChannelGids(tracing_event_t tev,
                                         std::vector<std::string> &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

void ta2_plugin_t::inner_xGetChannelProperties(tracing_event_t tev,
                                               std::string const &,
                                               ChannelProperties &) {
  SST_TEV_TOP(tev);
  SST_THROW_UNIMPLEMENTED();
  SST_TEV_BOT(tev);
}

} // namespace kestrel
