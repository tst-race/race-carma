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
#include <kestrel/carma/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/plugin_t.hpp>
//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/make_unique.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/what.hpp>
#include <sst/time.h>

#include <ConnectionStatus.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>

#include <kestrel/CARMA_XLOG_INFO.hpp>
#include <kestrel/carma/bootstrap_config_t.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/config.h>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/connection_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/goodbox_entry_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_packet_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_profile_t.hpp>
#include <kestrel/open_connection_call_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

#define LOG_EXCEPTION(logging_macro, tev, event, exception)            \
  (logging_macro(                                                      \
      sdk_,                                                            \
      0,                                                               \
      SST_TEV_ARG((tev), "event", (event), "exception", (exception))))

namespace kestrel {
namespace carma {

void plugin_t::inner_init(tracing_event_t tev,
                          PluginConfig const & pluginConfig) {
  SST_TEV_TOP(tev);

  plugin_config_ = pluginConfig;

#if !CARMA_WITH_MOCK_SDK
  try {
    auto const jaegerConfigPath =
        pluginConfig.etcDirectory + "/jaeger-config.yml";
    std::atomic_store(&tracer_.ptr,
                      sdk_.createTracer(SST_TEV_ARG(tev),
                                        jaegerConfigPath,
                                        persona_->string()));
  } catch (...) {
  }
#endif

  old_config_ =
      sst::json::get_from_string<decltype(old_config_), json_t>(
          sdk_.readFile(SST_TEV_ARG(tev), "config.json"));

  //--------------------------------------------------------------------
  // Parse link-profiles.json
  //--------------------------------------------------------------------

  auto const link_profiles_data =
      sdk().readFile(SST_TEV_ARG(tev), "link-profiles.json");

  if (!link_profiles_data.empty()) {

    CARMA_XLOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "link_profiles_file_found"));

    CARMA_XLOG_INFO(sdk_,
                    0,
                    SST_TEV_ARG(tev, "event", "parsing_link_profiles"));

    link_profiles_ =
        sst::json::get_from_string<decltype(link_profiles_), json_t>(
            link_profiles_data);

    CARMA_XLOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "done_parsing_link_profiles"));

    // TODO: !!!!! This is a temporary hack to treat kestrel-rabbitmq
    //       addresses as kestrel-rabbitmq_management addresses when
    //       we're compiling for Emscripten.
#ifdef __EMSCRIPTEN__
    link_profiles_[channel_id_t("kestrel-rabbitmq_management")] =
        std::move(link_profiles_.at(channel_id_t("kestrel-rabbitmq")));
#endif

  } else {

    CARMA_XLOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "link_profiles_file_not_found"));

  } //

  //--------------------------------------------------------------------

  update_channels(SST_TEV_ARG(tev));

  bootstrappee_step_1(SST_TEV_ARG(tev));

  for (auto & kv : channels_) {
    auto & channel = kv.second;
    if (channel.properties().channelStatus == CHANNEL_ENABLED) {
      try {
        channel.activate(SST_TEV_ARG(tev));
      } catch (...) {
      }
    }
  }

  //--------------------------------------------------------------------

  link_discovery_cooldown_mutex_.cooldown(
      std::chrono::seconds(sst::checked_cast<std::chrono::seconds::rep>(
          old_config_.link_discovery_cooldown())));

  prime_size_ = old_config_.prime.num_bytes<decltype(prime_size_)>(
      sst::integer_rep::pure_unsigned());

  sdk_.set_config(SST_TEV_ARG(tev), old_config_);

  //--------------------------------------------------------------------

  CARMA_LOG_INFO(
      sdk_,
      0,
      SST_TEV_ARG(tev, "event", "starting_network_maintenance_thread"));

  network_maintenance_thread_ = std::thread([this]() {
    tracing_event_t SST_TEV_DEF(tev);
    try {
      SST_TEV_ADD(tev, "my_psn", persona().value());
      auto const & cooldown = network_maintenance_cooldown_;
      auto & mutex = network_maintenance_mutex_;
      auto & cond = network_maintenance_cond_;
      auto const & stop = network_maintenance_stop_;
      auto const pred = [&]() { return stop.load(); };
      std::unique_lock<std::mutex> lock(mutex);
      auto next = std::chrono::steady_clock::now();
      while (!cond.wait_until(lock, next, pred)) {
        [&](tracing_event_t tev) {
          try {
            SST_TEV_ADD(tev,
                        "network_maintenance_call_id",
                        sst::to_string(network_maintenance_call_id_++));
            primary_lock_t const primary_lock(*this->primary_mutex());
            do_network_maintenance(SST_TEV_ARG(tev));
          } catch (tracing_exception_t const & e) {
            LOG_EXCEPTION(CARMA_LOG_ERROR,
                          e.tev(),
                          "network_maintenance_failed",
                          sst::what());
          } catch (...) {
            LOG_EXCEPTION(CARMA_LOG_ERROR,
                          tev,
                          "network_maintenance_failed",
                          sst::what());
          }
        }(SST_TEV_ARG(tev));
        next = std::chrono::steady_clock::now() + cooldown;
      }
    } catch (tracing_exception_t const & e) {
      LOG_EXCEPTION(CARMA_LOG_FATAL,
                    e.tev(),
                    "network_maintenance_thread_failed",
                    sst::what());
      primary_lock_t const primary_lock(*this->primary_mutex());
      plugin_state_ = plugin_state_t::broken;
    } catch (...) {
      LOG_EXCEPTION(CARMA_LOG_FATAL,
                    tev,
                    "network_maintenance_thread_failed",
                    sst::what());
      primary_lock_t const primary_lock(*this->primary_mutex());
      plugin_state_ = plugin_state_t::broken;
    }
  });

  //--------------------------------------------------------------------

  try {
    sdk_.onPluginStatusChanged(SST_TEV_ARG(tev),
                               plugin_status_t::ready().value());
  } catch (...) {
  }

  //--------------------------------------------------------------------

  CARMA_LOG_INFO(
      sdk_,
      0,
      SST_TEV_ARG(tev, "event", "network_maintenance_thread_started"));

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
