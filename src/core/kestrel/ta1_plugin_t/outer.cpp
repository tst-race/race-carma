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
#include <utility>

#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

PluginResponse ta1_plugin_t::init(PluginConfig const & pluginConfig) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev, "plugin_function_input", this->i_init(pluginConfig));
  return this->template inner_call<func_init, PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->common_init(SST_TEV_ARG(tev), pluginConfig);
        this->inner_init(SST_TEV_ARG(tev), pluginConfig);
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_init(pluginConfig));
      });
}

PluginResponse ta1_plugin_t::notifyEpoch(std::string const & data) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev, "plugin_function_input", this->i_notifyEpoch(data));
  return this->template inner_call<func_notifyEpoch, PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->inner_notifyEpoch(SST_TEV_ARG(tev), std::move(data));
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_notifyEpoch(data));
      });
}

PluginResponse ta1_plugin_t::onBootstrapPkgReceived(std::string persona,
                                                    RawData pkg) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onBootstrapPkgReceived(persona, pkg));
  return this
      ->template inner_call<func_onBootstrapPkgReceived, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_onBootstrapPkgReceived(
                SST_TEV_ARG(tev),
                psn_t(std::move(persona)),
                std::move(pkg));
            SST_TEV_ADD(tev,
                        "plugin_function_output",
                        this->o_onBootstrapPkgReceived(persona, pkg));
          });
}

PluginResponse
ta1_plugin_t::onChannelStatusChanged(RaceHandle handle,
                                     std::string channelGid,
                                     ChannelStatus status,
                                     ChannelProperties properties) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onChannelStatusChanged(handle,
                                             channelGid,
                                             status,
                                             properties));
  return this
      ->template inner_call<func_onChannelStatusChanged, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_onChannelStatusChanged(
                SST_TEV_ARG(tev),
                race_handle_t(std::move(handle)),
                channel_id_t(std::move(channelGid)),
                channel_status_t(std::move(status)),
                std::move(properties));
            SST_TEV_ADD(tev,
                        "plugin_function_output",
                        this->o_onChannelStatusChanged(handle,
                                                       channelGid,
                                                       status,
                                                       properties));
          });
}

PluginResponse
ta1_plugin_t::onConnectionStatusChanged(RaceHandle handle,
                                        ConnectionID connId,
                                        ConnectionStatus status,
                                        LinkID linkId,
                                        LinkProperties properties) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onConnectionStatusChanged(handle,
                                                connId,
                                                status,
                                                linkId,
                                                properties));
  return this->template inner_call<func_onConnectionStatusChanged,
                                   PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->inner_onConnectionStatusChanged(
            SST_TEV_ARG(tev),
            race_handle_t(std::move(handle)),
            connection_id_t(std::move(connId)),
            connection_status_t(std::move(status)),
            link_id_t(std::move(linkId)),
            std::move(properties));
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_onConnectionStatusChanged(handle,
                                                      connId,
                                                      status,
                                                      linkId,
                                                      properties));
      });
}

PluginResponse
ta1_plugin_t::onLinkPropertiesChanged(LinkID linkId,
                                      LinkProperties linkProperties) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onLinkPropertiesChanged(linkId, linkProperties));
  return this
      ->template inner_call<func_onLinkPropertiesChanged, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_onLinkPropertiesChanged(
                SST_TEV_ARG(tev),
                link_id_t(std::move(linkId)),
                std::move(linkProperties));
            SST_TEV_ADD(
                tev,
                "plugin_function_output",
                this->o_onLinkPropertiesChanged(linkId,
                                                linkProperties));
          });
}

PluginResponse
ta1_plugin_t::onLinkStatusChanged(RaceHandle handle,
                                  LinkID linkId,
                                  LinkStatus status,
                                  LinkProperties properties) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(
      tev,
      "plugin_function_input",
      this->i_onLinkStatusChanged(handle, linkId, status, properties));
  return this
      ->template inner_call<func_onLinkStatusChanged, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_onLinkStatusChanged(
                SST_TEV_ARG(tev),
                race_handle_t(std::move(handle)),
                link_id_t(std::move(linkId)),
                link_status_t(std::move(status)),
                std::move(properties));
            SST_TEV_ADD(tev,
                        "plugin_function_output",
                        this->o_onLinkStatusChanged(handle,
                                                    linkId,
                                                    status,
                                                    properties));
          });
}

PluginResponse
ta1_plugin_t::onPackageStatusChanged(RaceHandle handle,
                                     PackageStatus status) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onPackageStatusChanged(handle, status));
  return this
      ->template inner_call<func_onPackageStatusChanged, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_onPackageStatusChanged(
                SST_TEV_ARG(tev),
                race_handle_t(std::move(handle)),
                package_status_t(std::move(status)));
            SST_TEV_ADD(tev,
                        "plugin_function_output",
                        this->o_onPackageStatusChanged(handle, status));
          });
}

PluginResponse
ta1_plugin_t::onUserAcknowledgementReceived(RaceHandle handle) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_onUserAcknowledgementReceived(handle));
  return this->template inner_call<func_onUserAcknowledgementReceived,
                                   PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->inner_onUserAcknowledgementReceived(
            SST_TEV_ARG(tev),
            race_handle_t(std::move(handle)));
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_onUserAcknowledgementReceived(handle));
      });
}

PluginResponse ta1_plugin_t::prepareToBootstrap(RaceHandle handle,
                                                LinkID linkId,
                                                std::string configPath,
                                                DeviceInfo deviceInfo) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_prepareToBootstrap(handle,
                                         linkId,
                                         configPath,
                                         deviceInfo));
  return this
      ->template inner_call<func_prepareToBootstrap, PLUGIN_ERROR>(
          tev,
          [&](tracing_event_t & tev) {
            this->inner_prepareToBootstrap(
                SST_TEV_ARG(tev),
                race_handle_t(std::move(handle)),
                link_id_t(std::move(linkId)),
                std::move(configPath),
                std::move(deviceInfo));
            SST_TEV_ADD(tev,
                        "plugin_function_output",
                        this->o_prepareToBootstrap(handle,
                                                   linkId,
                                                   configPath,
                                                   deviceInfo));
          });
}

PluginResponse ta1_plugin_t::processClrMsg(RaceHandle const handle,
                                           ClrMsg const & msg) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_processClrMsg(handle, msg));
  return this->template inner_call<func_processClrMsg, PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->inner_processClrMsg(SST_TEV_ARG(tev),
                                  race_handle_t(std::move(handle)),
                                  std::move(msg));
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_processClrMsg(handle, msg));
      });
}

PluginResponse ta1_plugin_t::shutdown() {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev, "plugin_function_input", this->i_shutdown());
  return this->template inner_call<func_shutdown, PLUGIN_FATAL>(
      tev,
      [&](tracing_event_t & tev) {
        this->common_shutdown(SST_TEV_ARG(tev));
        this->inner_shutdown(SST_TEV_ARG(tev));
        SST_TEV_ADD(tev, "plugin_function_output", this->o_shutdown());
      });
}

} // namespace kestrel
