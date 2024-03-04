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

#ifndef KESTREL_CARMA_PLUGIN_T_HPP
#define KESTREL_CARMA_PLUGIN_T_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/cooldown_mutex.hpp>
#include <sst/catalog/from_varint.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/perfect_gt.hpp>
#include <sst/catalog/remove_cvref_t.hpp>
#include <sst/catalog/span.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_varint.hpp>
#include <sst/catalog/track_it.hpp>
#include <sst/catalog/unix_time_s.hpp>
#include <sst/catalog/unix_time_s_t.hpp>
#include <sst/catalog/varint_size.hpp>

// RACE headers
#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>

#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/vec_ZZ_p.h>

// CARMA headers
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/mailbox_message_type_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/corruption_t.hpp>
#include <kestrel/bytes_t.hpp>
#include <kestrel/config.h>
#include <kestrel/generic_hash_t.hpp>
#include <kestrel/old_config_t.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/client_mb_packet_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/create_link_call_t.hpp>
#include <kestrel/create_link_from_address_call_t.hpp>
#include <kestrel/link_profile_t.hpp>
#include <kestrel/goodbox_entry_t.hpp>
#include <kestrel/graeffe_transform.hpp>
#include <kestrel/guid_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/link_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/load_link_call_t.hpp>
#include <kestrel/mb_mc_up_packet_t.hpp>
#include <kestrel/mc_leader_init_packet_t.hpp>
#include <kestrel/mc_mb_down_packet_t.hpp>
#include <kestrel/mc_v_packet_t.hpp>
#include <kestrel/open_connection_call_t.hpp>
#include <kestrel/outbox_entry_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/pooled.hpp>
#include <kestrel/prime_size_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/sdk_span_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/secret_sharing.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/span_parents_t.hpp>
#include <kestrel/easy_ta1_plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

#if !CARMA_WITH_MOCK_SDK
#include <opentracing/tracer.h>
#endif

namespace kestrel {
namespace carma {

class plugin_t : public easy_ta1_plugin_t<config_t> {

protected:

  struct deserialize_packet_result;

private:

  //--------------------------------------------------------------------

  PluginConfig plugin_config_;

  //--------------------------------------------------------------------

  void processEncPkg(tracing_event_t tev,
                     race_handle_t const & handle,
                     EncPkg const & ePkg,
                     std::vector<ConnectionID> const & connIDs);

  void onPersonaLinksChanged(tracing_event_t tev,
                             psn_t const & persona,
                             link_type_t const & link_type,
                             std::vector<LinkID> const & link_ids);

  void onUserInputReceived(tracing_event_t tev,
                           race_handle_t const & handle,
                           bool answered,
                           std::string const & response);

  //--------------------------------------------------------------------
  // Config traversal
  //--------------------------------------------------------------------

  global_config_t & global() noexcept {
    return config().global();
  }

  global_config_t const & global() const noexcept {
    return config().global();
  }

  local_config_t & local() noexcept {
    return config().local();
  }

  local_config_t const & local() const noexcept {
    return config().local();
  }

  phonebook_t & phonebook() noexcept {
    return config().phonebook();
  }

  phonebook_t const & phonebook() const noexcept {
    return config().phonebook();
  }

  chunk_joiner_t & chunk_joiner() noexcept {
    return config().chunk_joiner();
  }

  chunk_joiner_t const & chunk_joiner() const noexcept {
    return config().chunk_joiner();
  }

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------

  void inner_init(tracing_event_t tev,
                  PluginConfig const & pluginConfig) final;

  void inner_onBootstrapPkgReceived(tracing_event_t tev,
                                    psn_t const & persona,
                                    RawData const & pkg) final;

  void inner_onChannelStatusChanged(
      tracing_event_t tev,
      race_handle_t const & handle,
      channel_id_t const & channelGid,
      channel_status_t const & status,
      ChannelProperties const & properties) final;

  void inner_onConnectionStatusChanged(
      tracing_event_t tev,
      race_handle_t const & handle,
      connection_id_t const & connId,
      connection_status_t status,
      link_id_t const & linkId,
      LinkProperties const & properties) final;

  void inner_onLinkPropertiesChanged(
      tracing_event_t tev,
      link_id_t const & linkId,
      LinkProperties const & linkProperties) final;

  void inner_onLinkStatusChanged(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 link_id_t const & linkId,
                                 link_status_t const & status,
                                 LinkProperties const & properties)
      final;

  void inner_onPackageStatusChanged(tracing_event_t tev,
                                    race_handle_t const & handle,
                                    package_status_t const & status)
      final;

  void inner_onUserAcknowledgementReceived(tracing_event_t tev,
                                           race_handle_t const & handle)
      final;

  void inner_prepareToBootstrap(tracing_event_t tev,
                                race_handle_t const & handle,
                                link_id_t const & linkId,
                                std::string const & configPath,
                                DeviceInfo const & deviceInfo) final;

  void inner_processClrMsg(tracing_event_t tev,
                           race_handle_t const & handle,
                           ClrMsg const & msg) final;

  void inner_shutdown(tracing_event_t tev) noexcept final;

  //--------------------------------------------------------------------
  // OpenTracing tracer
  //--------------------------------------------------------------------

protected:
#if CARMA_WITH_MOCK_SDK

  using tracer_t = int;

  template<class Function>
  void if_tracer(Function &&) {
  }

#else

  using tracer_t = opentracing::Tracer;

  template<class Function>
  void if_tracer(Function && f) {
    try {
      auto const tracer = std::atomic_load(&tracer_.ptr).get();
      if (tracer) {
        f(*tracer);
      }
    } catch (...) {
      std::atomic_store(&tracer_.ptr, std::shared_ptr<tracer_t>());
    }
  }

  class {
    std::shared_ptr<tracer_t> ptr;

    template<class Function>
    friend void plugin_t::if_tracer(Function && f);

    friend void plugin_t::inner_init(tracing_event_t tev,
                                     PluginConfig const & pluginConfig);
  } tracer_;

#endif

  //--------------------------------------------------------------------

  std::unordered_map<channel_id_t, std::vector<link_profile_t>>
      link_profiles_;

  //--------------------------------------------------------------------

protected:
  class process_message_context_t final : public origin_handle_t,
                                          public origin_span_t {

  public:

    sdk_span_t send_span;

    sdk_wrapper_t & sdk;
    plugin_t & plugin;

    explicit process_message_context_t(sdk_wrapper_t & sdk,
                                       plugin_t & plugin)
        : origin_handle_t(),
          origin_span_t(),
          send_span(),
          sdk(sdk),
          plugin(plugin) {
    }

    explicit process_message_context_t(sdk_wrapper_t & sdk,
                                       plugin_t & plugin,
                                       race_handle_t const & handle,
                                       ClrMsg const & message)
        : origin_handle_t(handle),
          origin_span_t(message),
          send_span(origin_span()),
          sdk(sdk),
          plugin(plugin) {
    }

    explicit process_message_context_t(sdk_wrapper_t & sdk,
                                       plugin_t & plugin,
                                       race_handle_t const & handle,
                                       EncPkg const & message)
        : origin_handle_t(handle),
          origin_span_t(message),
          send_span(origin_span()),
          sdk(sdk),
          plugin(plugin) {
      static_cast<void>(message);
    }

    race_handle_t send_message(tracing_event_t tev,
                               std::vector<unsigned char> const & msg,
                               psn_t const & psn) {
      SST_TEV_ADD(tev);
      try {
        auto entry_ptr = plugin.acquire<outbox_entry_t>();
        auto & entry = *entry_ptr;
        entry.init(
            origin_handle(),
            psn,
            EncPkg(send_span.trace_id(), send_span.span_id(), msg));
        plugin.outbox_.emplace_back(std::move(entry_ptr));
        return plugin.send(SST_TEV_ARG(tev), entry);
      }
      SST_TEV_RETHROW(tev);
    }
  };

  struct server_session_t final {

    plugin_t * plugin;
    old_config_t * config;

    guid_t mpcid;

    enum class state_t {
      T0_wait_init,
      T1_wait_mb_mc_up,
      T2_wait_v_packet,
      T3_end,
    } state;

    static std::string to_string(state_t const state) {
      // clang-format off
      switch (state) {
#define F(x) case state_t::x: return #x;
        F(T0_wait_init)
        F(T1_wait_mb_mc_up)
        F(T2_wait_v_packet)
        F(T3_end)
#undef F
      }
      // clang-format on
      SST_ASSERT(false);
      throw std::runtime_error("missing server session state");
    }

    pooled<mc_leader_init_packet_t> p_init;
    std::vector<pooled<mb_mc_up_packet_t>> p_mb_mc_up; // mixsize
    std::vector<pooled<mc_v_packet_t>> p_v_packet; // mcsize

    std::vector<sst::bigint> T2_z;
    std::vector<sst::bigint> T2_shrs;
    std::vector<decltype(+T2_shrs.size())> T2_pts;
    std::vector<sst::bigint> T2_x;

    void deduce(tracing_event_t tev,
                plugin_t & plugin,
                guid_t const & mpcid) {
      this->plugin = &plugin;
      config = &plugin.old_config_;
      this->mpcid = mpcid;
      state = state_t::T0_wait_init;
      p_init.reset();
      p_mb_mc_up.clear();
      sst::checked_resize(p_mb_mc_up, config->mixsize);
      p_v_packet.clear();
      sst::checked_resize(
          p_v_packet,
          plugin.local().mc_group(SST_TEV_ARG(tev)).size());
    }

    void create(tracing_event_t tev, plugin_t & plugin) {
      deduce(SST_TEV_ARG(tev), plugin, guid_t::generate());
    }

    bool tick(tracing_event_t tev, process_message_context_t & pmc) {
      SST_TEV_ADD(tev);
      try {
        global_config_t const & global =
            plugin->config().global();
        local_config_t const & local =
            plugin->config().local();

        while (true) {
          CARMA_LOG_INFO(plugin->sdk(),
                         0,
                         SST_TEV_ARG(tev,
                                       "event",
                                       "session_tick_loop",
                                       "session_state",
                                       to_string(state)));
          switch (state) {
            case state_t::T0_wait_init: {
              if (p_init == nullptr) {
                return false;
              }
              state = state_t::T1_wait_mb_mc_up;
            } break;
            case state_t::T1_wait_mb_mc_up: {
              for (auto const & p : p_mb_mc_up) {
                if (p == nullptr) {
                  return false;
                }
              }

              plugin->if_tracer([&](tracer_t & tracer) {
                if (local.role() != role_t::mc_leader()) {
                  span_parents_t parents = {p_init->origin_span()};
                  for (auto const & p : p_mb_mc_up) {
                    parents.push_back(p->origin_span());
                  }
                  pmc.send_span = sdk_span_t(tracer,
                                             "carma_mc_server_mix",
                                             parents);
                }
              });

              auto pooled_v =
                  plugin->template acquire<std::vector<sst::bigint>>();
              auto & v = *pooled_v;
              sst::checked_resize(v, config->mixsize);
              for (auto & vv : v) {
                vv = 0;
              }
              for (auto const & p : p_mb_mc_up) {
                for (decltype(+v.size()) i = 0; i < v.size(); ++i) {
                  v[i] = add_mod(v[i], p->z[i], config->prime);
                }
              }
              auto v_packet = plugin->template acquire<mc_v_packet_t>();
              v_packet->type = packet_type_t::mc_v_packet();
              v_packet->prime_size = plugin->prime_size_;
              v_packet->mixsize = config->mixsize;
              v_packet->mpcid = mpcid;
              v_packet->v = std::move(v);
              if (local.role() == role_t::mc_follower()) {
                plugin->send(SST_TEV_ARG(tev),
                             pmc,
                             *v_packet,
                             *plugin->config().phonebook().at(
                                 SST_TEV_ARG(tev),
                                 local.mc_leader(SST_TEV_ARG(tev))));
              }
              p_v_packet[local.order()] = std::move(v_packet);
              if (local.role() == role_t::mc_leader()
                  || !config->leader_relay_only) {
                state = state_t::T2_wait_v_packet;
              } else {
                state = state_t::T3_end;
              }
            } break;

            case state_t::T2_wait_v_packet: {
              auto const mixsize = config->mixsize;
              auto const mcsize = p_v_packet.size();
              auto const recon_threshold = config->threshold + 1;
              {
                decltype(+p_v_packet.size()) recon_n = 0;
                for (auto const & p : p_v_packet) {
                  if (p != nullptr) {
                    if (++recon_n >= recon_threshold) {
                      break;
                    }
                  }
                }
                if (recon_n < recon_threshold) {
                  return false;
                }
              }
              auto & z = T2_z;
              auto & shrs = T2_shrs;
              auto & pts = T2_pts;
              auto & x = T2_x;
              sst::checked_resize(z, mixsize);
              sst::checked_resize(shrs, mcsize);
              sst::checked_resize(pts, recon_threshold);
              sst::checked_resize(x, mixsize);
              {
                decltype(+mcsize) recon_n = 0;
                for (decltype(+mcsize) i = 0; i < mcsize; ++i) {
                  if (p_v_packet[i] != nullptr) {
                    pts[recon_n] = i;
                    if (++recon_n >= recon_threshold) {
                      break;
                    }
                  }
                }
              }
              for (decltype(+mixsize) j = 0; j < mixsize; ++j) {
                decltype(+mcsize) recon_n = 0;
                for (decltype(+mcsize) i = 0; i < mcsize; ++i) {
                  if (p_v_packet[i] != nullptr) {
                    shrs[i] = std::move(p_v_packet[i]->v[j]);
                    if (++recon_n >= recon_threshold) {
                      break;
                    }
                  }
                }
                sss_recon(z[j], shrs, pts, config->prime);
              }

#if 0
              CARMA_LOG_TRACE(
                  plugin->sdk(),
                  0,
                  SST_TEV_ARG(tev,
                                "event",
                                "mc_server_calculated_root_sum_powers",
                                "mc_server_calculated_root_sum_powers",
                                [&]() {
                                  auto xs = nlohmann::json::array();
                                  for (decltype(+mixsize) i = 0;
                                       i < mixsize;
                                       ++i) {
                                    xs += z[i].to_string();
                                  }
                                  return xs;
                                }()));
#endif

              // zeta must be a primitive root
              SST_ASSERT(config->prime
                         == (sst::bigint(1) << config->two_exponent)
                                    * config->odd_factor
                                + 1);
              SST_ASSERT((sst::perfect_gt(config->odd_factor, mixsize)));
              {
                NumX poly(config->prime);
                newton_to_polynomial(
                    poly,
                    z,
                    sst::checked_cast<unsigned int>(mixsize));
                x = find_roots(
                    poly,
                    config->zeta,
                    sst::checked_cast<long>(config->two_exponent),
                    sst::checked_cast<long>(config->odd_factor));
              }

              // All servers must have the roots in the same order.
              std::sort(x.begin(),
                        x.end(),
                        [](sst::bigint const & a,
                           sst::bigint const & b) { return a < b; });

              for (decltype(+mixsize) i = 0; i < mixsize; ++i) {
                auto packet =
                    plugin->template acquire<mc_mb_down_packet_t>();
                bytes_t buf =
                    x[i].to_bytes(sst::integer_rep::pure_unsigned());
                buf.resize(global.prime_space());
                bytes_t::size_type buf_i = 0;
                bytes_t c;
                bytes_t::size_type cc_i = 0;
                deserialize(buf, buf_i, packet->a, packet->b, c);
                bytes_t const cc = plugin->local().anon_decrypt(c);
                psn_any_hash_t mailbox_hash;
                deserialize(cc, cc_i, mailbox_hash);
                std::shared_ptr<phonebook_entry_t const> const mailbox =
                    plugin->phonebook().at(SST_TEV_ARG(tev),
                                           mailbox_hash);
                if (mailbox->role() != role_t::mb_server()) {
                  throw corruption_t();
                }
                packet->type = packet_type_t::mc_mb_down_packet();
                packet->oid = mpcid + (i + 1);
                plugin->send(SST_TEV_ARG(tev), pmc, *packet, *mailbox);
              }

#if 0
              CARMA_LOG_TRACE(
                  plugin->sdk(),
                  0,
                  SST_TEV_ARG(tev,
                                "event",
                                "mc_server_calculated_roots",
                                "mc_server_calculated_roots",
                                [&]() {
                                  auto xs = nlohmann::json::array();
                                  for (decltype(+mixsize) i = 0;
                                       i < mixsize;
                                       ++i) {
                                    xs += x[i].to_string();
                                  }
                                  return xs;
                                }()));
#endif

              CARMA_LOG_TRACE(
                  plugin->sdk(),
                  0,
                  SST_TEV_ARG(tev,
                                "event",
                                "mc_server_picked_oids",
                                "mc_server_picked_oids",
                                [&]() {
                                  auto xs = nlohmann::json::array();
                                  for (decltype(+mixsize) i = 0;
                                       i < mixsize;
                                       ++i) {
                                    xs += (mpcid + (i + 1)).to_json();
                                  }
                                  return xs;
                                }()));

              state = state_t::T3_end;
            } break;

            case state_t::T3_end: {
              return true;
            } break;
            default: {
              throw std::runtime_error("missing server session state");
            } break;
          }
        }
      }
      SST_TEV_RETHROW(tev);
    }
  };

  //--------------------------------------------------------------------

  void inner_processClrMsg2(tracing_event_t tev,
                            process_message_context_t & pmc,
                            phonebook_entry_t const & recver_client,
                            bytes_t const & a_data,
                            mailbox_message_type_t recver_mb_cmd);

  //--------------------------------------------------------------------

  std::set<std::vector<unsigned char>> processed_packet_ids_;

  //--------------------------------------------------------------------

  std::set<guid_t> mb_processed_cids_;
  std::map<guid_t, sdk_span_t> mb_processed_oids_;

  //--------------------------------------------------------------------

  std::set<sst::remove_cvref_t<decltype(std::declval<generic_hash_t>().finish())>> clrmsg_seen_;

  //--------------------------------------------------------------------

  std::map<guid_t, pooled<server_session_t>> sessions_;

  server_session_t & get_or_deduce_session(tracing_event_t tev,
                                           guid_t const & mpcid) {
    auto const it = sessions_.find(mpcid);
    if (it != sessions_.end()) {
      return *it->second;
    }
    auto session = acquire<server_session_t>();
    session->deduce(SST_TEV_ARG(tev), *this, mpcid);
    auto & s = *session;
    sessions_.emplace(mpcid, std::move(session));
    return s;
  }

  std::map<guid_t, pooled<mb_mc_up_packet_t>> loose_ups;
  std::map<guid_t, std::reference_wrapper<server_session_t>> wanted_ups;

  //--------------------------------------------------------------------

  prime_size_t prime_size_;

  //--------------------------------------------------------------------

  struct prepareToBootstrap_info_1_t {
    sst::unix_time_s_t creation_time = sst::unix_time_s();
    bool garbage = true;

    race_handle_t handle{};
    link_id_t linkId{};
    std::string configPath{};
    DeviceInfo deviceInfo{};

    race_handle_t openConnection_handle{};
  };

  std::unordered_map<race_handle_t, prepareToBootstrap_info_1_t>
      prepareToBootstrap_infos_1_;

  struct prepareToBootstrap_info_2_t final {
    sst::unix_time_s_t creation_time = sst::unix_time_s();
    bool garbage = true;

    connection_id_t connection_id{};
  };

  // TODO: Use unordered_map here after guid_t supports hashing.
  std::map<guid_t, prepareToBootstrap_info_2_t>
      prepareToBootstrap_infos_2_;

  //--------------------------------------------------------------------
  // Client
  //--------------------------------------------------------------------
  //
  // For the bootstrappee, the bootstrap channel is tracked normally in
  // the channels_ map, but the link is not tracked in the links_ map
  // because we don't want the link to be used normally. Instead, the
  // link information is only stored in these variables.
  //

  void client_handle_add_contact_request(
      tracing_event_t tev,
      process_message_context_t & pmc,
      deserialize_packet_result const & dpr);

  void client_handle_add_contact_response(tracing_event_t tev,
                                          process_message_context_t
                                              & pmc,
                                          bytes_t const & a_data,
                                          bytes_t::size_type a_data_i);

  void handle_attached_clrmsg(tracing_event_t tev,
                              process_message_context_t & pmc,
                              phonebook_entry_t const & recver_client,
                              bytes_t const & a_data,
                              mailbox_message_type_t recver_mb_cmd);

  void handle_detached_clrmsg(tracing_event_t tev,
                              race_handle_t const & handle,
                              clrmsg_t const & clrmsg,
                              process_message_context_t & pmc);

  void bootstrappee_on_registration_complete(
      tracing_event_t tev,
      deserialize_packet_result const & dpr);

  channel_t const * bootstrappee_reply_channel_{};
  race_handle_t bootstrappee_reply_link_load_handle_{};
  link_id_t bootstrappee_reply_link_id_{};
  race_handle_t bootstrappee_reply_connection_open_handle_{};
  connection_id_t bootstrappee_reply_connection_id_{};

  struct bootstrappee_cookie_t {
    channel_t const * channel{};
    phonebook_pair_t const * mb_server{};
    sst::optional<link_address_t> cookie{};
    explicit bootstrappee_cookie_t(channel_t const & channel,
                                   phonebook_pair_t const & mb_server)
        : channel(&channel),
          mb_server(&mb_server) {
    }
  };

  std::unordered_map<channel_t const *, bool> bootstrappee_mb_channels_;
  std::unordered_map<race_handle_t, bootstrappee_cookie_t>
      bootstrappee_cookies_{};
  decltype(bootstrappee_cookies_.size()) bootstrappee_cookie_count_{};

  bool bootstrappee_step_1(tracing_event_t tev);
  bool bootstrappee_step_1_done_ = false;

  bool bootstrappee_step_2(tracing_event_t tev,
                           channel_t const & channel);
  bool bootstrappee_step_2_done_ = false;

  bool bootstrappee_step_3(tracing_event_t tev,
                           race_handle_t const & handle,
                           link_id_t const & link_id,
                           link_status_t const & link_status,
                           LinkProperties const & link_properties);
  bool bootstrappee_step_3_done_ = false;

  bool bootstrappee_step_4(
      tracing_event_t tev,
      race_handle_t const & handle,
      connection_id_t const & connection_id,
      connection_status_t const & connection_status,
      link_id_t const & link_id,
      LinkProperties const & link_properties);
  bool bootstrappee_step_4_done_ = false;

  bool bootstrappee_step_5(tracing_event_t tev,
                           channel_t const & channel);
  bool bootstrappee_step_5_done_ = false;

  bool bootstrappee_step_6(tracing_event_t tev,
                           race_handle_t const & handle,
                           link_id_t const & link_id,
                           link_status_t const & link_status,
                           LinkProperties const & link_properties);
  bool bootstrappee_step_6_done_ = false;

  bool bootstrappee_step_7(tracing_event_t tev);
  bool bootstrappee_step_7_done_ = false;

  void client_processEncPkg(tracing_event_t tev,
                            process_message_context_t & pmc,
                            deserialize_packet_result const & dpr);

  //--------------------------------------------------------------------
  // Object pooling
  //--------------------------------------------------------------------
  //
  // TODO: Actually pool. Be careful of thread safety once we start
  //       allowing plugin calls to execute concurrently.
  //
  // TODO: acquire() should perhaps be given Args&&... to forward to T's
  //       constructor (if allocating) or to its init() function (if
  //       reusing). All of our own classes that are poolable should
  //       always have an init() function. For pooling more fundamental
  //       types, we can support a narrow set of them that we explicitly
  //       support and do the right thing for them.
  //

  template<class T>
  pooled<T> acquire() {
    return pooled<T>(new T);
  }

  //--------------------------------------------------------------------
  // Channel tracking
  //--------------------------------------------------------------------
  //
  // channels_ keeps track of all known channels. It should only be
  // accessed under primary_mutex_. Entries should never be deleted.
  //
  // update_channels can be called by any thread at any time to update
  // channels_.
  //

  std::unordered_map<channel_id_t, channel_t> channels_;

  void update_channels(tracing_event_t tev);

  sst::cooldown_mutex channel_update_cooldown_mutex_{
      std::chrono::minutes(1)};
  std::future<void> channel_update_cooldown_future_;

  //--------------------------------------------------------------------
  // Link tracking
  //--------------------------------------------------------------------

  std::unordered_map<link_id_t, link_t> links_;

  void add_link(tracing_event_t tev,
                link_id_t const & id,
                LinkProperties const & properties,
                std::unordered_set<psn_t> const & personas);

  void add_link(tracing_event_t tev,
                link_id_t const & id,
                LinkProperties const & properties,
                psn_t const & persona);

  //--------------------------------------------------------------------
  // createLink call tracking
  //--------------------------------------------------------------------
  //
  // Everything in this section should only be accessed under
  // primary_mutex_.
  //

  std::unordered_map<race_handle_t, create_link_call_t>
      create_link_calls_;

  race_handle_t create_link(tracing_event_t tev,
                            channel_t & channel,
                            psn_t const & persona);

  //--------------------------------------------------------------------
  // createLinkFromAddress call tracking
  //--------------------------------------------------------------------
  //
  // Everything in this section should only be accessed under
  // primary_mutex_.
  //

  std::unordered_map<race_handle_t, create_link_from_address_call_t>
      create_link_from_address_calls_;

  void create_link_from_address(tracing_event_t tev,
                                channel_t & channel,
                                link_profile_t const & link_profile);

  //--------------------------------------------------------------------
  // loadLinkAddress call tracking
  //--------------------------------------------------------------------
  //
  // Everything in this section should only be accessed under
  // primary_mutex_.
  //

  std::unordered_map<race_handle_t, load_link_call_t> load_link_calls_;

  void load_link(tracing_event_t tev,
                 channel_id_t const & channel_id,
                 link_address_t const & link_address,
                 psn_t const & persona);

  void load_link(tracing_event_t tev,
                 channel_id_t const & channel_id,
                 link_profile_t const & link_profile);

  //--------------------------------------------------------------------
  // openConnection call tracking
  //--------------------------------------------------------------------

  std::unordered_map<race_handle_t, open_connection_call_t>
      open_connection_calls_;

  void open_connection(tracing_event_t tev, link_t & link);

  void ensure_connection(tracing_event_t tev, link_t & link);

  // TODO: close_connection(connection_id)

  //--------------------------------------------------------------------
  // Link discovery
  //--------------------------------------------------------------------

  void run_link_discovery(tracing_event_t tev);

  std::future<void> link_discovery_cooldown_future_;
  sst::cooldown_mutex link_discovery_cooldown_mutex_{true};

  //--------------------------------------------------------------------
  // Packet serialization
  //--------------------------------------------------------------------

  template<class Size, class Packet>
  Size get_serialized_packet_size(Packet const & src) {
    Size const k = pkc::auth_ciphertext_size<Size>(
        src.template to_bytes_size<Size>());
    sst::checked_t<Size> n = 0;
    n += persona().to_bytes_size<Size>();
    n += sst::varint_size(k);
    n += k;
    return n.value();
  }

  template<class Packet, class ByteIt1, class ByteIt2>
  ByteIt1 serialize_packet(tracing_event_t tev,
                           Packet const & src,
                           ByteIt1 dst,
                           ByteIt2 const id,
                           phonebook_entry_t const & remote) {
    dst = persona().to_bytes(dst);

    auto buf1_ptr = acquire<std::vector<unsigned char>>();
    auto & buf1 = *buf1_ptr;
    using size_type = decltype(+buf1.size());
    buf1.resize(src.template to_bytes_size<size_type>());
    src.to_bytes(&buf1[0]);

    auto pair = pkc::auth_encrypt(
        config().local().sk(),
        config().phonebook().at(SST_TEV_ARG(tev), remote.psn())->pk(),
        buf1);
    auto & buf2 = pair.first;
    auto const ct_size = buf2.size();
    unsigned char const * const p_id = pair.second;

    dst = sst::to_varint(ct_size, dst);

    dst = std::copy_n(&buf2[0], buf2.size(), dst);
    std::copy_n(p_id, pkc::auth_id_size<size_type>(), id);
    return dst;
  }

  struct deserialize_packet_result {
    pooled<psn_t> psn;

    // The bytes of the packet, including those at the front that were
    // already deserialized as the packet type.
    pooled<bytes_t> packet_data;

    // The offset just past the packet type in packet_data.
    bytes_t::size_type body_offset{};

    std::shared_ptr<phonebook_entry_t const> node_info;
    pooled<bytes_t> packet_id;
    packet_type_t packet_type;

    deserialize_packet_result(plugin_t & plugin)
        : psn(plugin.acquire<psn_t>()),
          packet_data(plugin.acquire<bytes_t>()),
          node_info(nullptr),
          packet_id(plugin.acquire<bytes_t>()) {
    }
  };

  deserialize_packet_result
  deserialize_packet(tracing_event_t tev,
                     process_message_context_t &,
                     unsigned char const *& src,
                     std::size_t & avail) {
    SST_TEV_ADD(tev);
    try {
      SST_ASSERT((src != nullptr));

      deserialize_packet_result dpr(*this);

      //
      // We don't want to update src and avail until we're sure that
      // we're going to consume a complete packet, so we use src2 and
      // avail2 until then. This way, when an exception is thrown, the
      // caller can distinguish between two cases: if src and avail are
      // unchanged then something is very wrong, but if not then there
      // may be another packet at the updated position despite an
      // exception being thrown while processing the current one.
      //

      auto src2 = src;
      auto avail2 = avail;

      src2 = dpr.psn->from_bytes(src2, avail2);

      CARMA_LOG_TRACE(sdk_,
                      0,
                      SST_TEV_ARG(tev,
                                    "event",
                                    "parsed_psn_from_incoming_packet",
                                    "parsed_psn_from_incoming_packet",
                                    nlohmann::json{*dpr.psn}));

      auto const ct_size =
          sst::from_varint<std::size_t>(sst::track_it(src2, &avail2));

      if (ct_size > avail2) {
        throw std::runtime_error(
            "A PKC ciphertext size parsed from an EncPkg incorrectly "
            "indicates that the ciphertext is larger than the rest of "
            "the actual EncPkg");
      }

      auto const ct = src2;
      src2 += ct_size;
      avail2 -= ct_size;

      src = src2;
      avail = avail2;

      // Look up the sender's PSN. If we don't recognize it, stop. We
      // don't accept messages from unknown parties, even if they're
      // correctly encrypted for us.
      auto const * const p_entry = config().phonebook().find(*dpr.psn);
      if (!p_entry) {
        return dpr;
      }
      dpr.node_info =
          config().phonebook().at(SST_TEV_ARG(tev), p_entry->first);
      SST_ASSERT((dpr.node_info));

      // Decrypt the packet data.
      {
        unsigned char const * const id = pkc::auth_decrypt(
            config()
                .phonebook()
                .at(SST_TEV_ARG(tev), dpr.node_info->psn())
                ->pk(),
            config().local().sk(),
            sst::span(ct, ct_size),
            *dpr.packet_data);
        sst::checked_resize(*dpr.packet_id,
                            pkc::auth_id_size<
                                decltype(dpr.packet_id->size())>());
        std::copy_n(id, dpr.packet_id->size(), dpr.packet_id->data());
      }

      // Parse the packet type.
      {
        auto n = dpr.packet_data->size();
        auto x = n;
        dpr.packet_type.from_bytes(dpr.packet_data->begin(), n);
        dpr.body_offset = x - n;
      }

      CARMA_LOG_TRACE(
          sdk_,
          0,
          SST_TEV_ARG(tev,
                        "event",
                        "parsed_packet_type_from_incoming_packet",
                        "parsed_packet_type_from_incoming_packet",
                        dpr.packet_type.to_json()));

      return dpr;
    }
    SST_TEV_RETHROW(tev);
  }

  template<class ByteSequence, class Packet>
  static void packet_from_bytes_exact(ByteSequence const & src,
                                      Packet & dst) {
    auto avail = src.size();
    dst.from_bytes(src.begin(), avail);
    if (avail != 0) {
      throw std::runtime_error("corrupt packet");
    }
  }

  //--------------------------------------------------------------------

  server_session_t * handle_mb_mc_up_packet(
      tracing_event_t tev,
      process_message_context_t & pmc,
      pooled<mb_mc_up_packet_t> up_packet);

  server_session_t * handle_mb_mc_up_bundle_packet(
      tracing_event_t tev,
      process_message_context_t & pmc,
      deserialize_packet_result const & dpr);

  void mc_server_processEncPkg(tracing_event_t tev,
                               process_message_context_t & pmc,
                               deserialize_packet_result const & dpr);

  //--------------------------------------------------------------------
  // Role-specific processEncPkg functions
  //--------------------------------------------------------------------

  void processEncPkg_idle_server(tracing_event_t tev,
                                 process_message_context_t & pmc,
                                 deserialize_packet_result const & dpr);

  //--------------------------------------------------------------------
  // MB server
  //--------------------------------------------------------------------

  void mailbox_handle_add_contact_request(
      tracing_event_t tev,
      process_message_context_t & pmc,
      bytes_t const & a_data,
      psn_t const & recver_client_psn);

  struct mb_server_bootstrappee_link_creation_t final {
    psn_t bootstrappee;
    channel_id_t channel;
  };

  std::unordered_map<race_handle_t,
                     mb_server_bootstrappee_link_creation_t>
      mb_server_bootstrappee_link_creations_;

  bool mb_server_bootstrappee_link_created(
      tracing_event_t tev,
      process_message_context_t & pmc,
      race_handle_t const & handle,
      link_address_t const & cookie);

  void handle_new_client(tracing_event_t tev,
                         bytes_t const & a,
                         psn_t const & bootstrappee_psn);

  void handle_client_mb_packet(tracing_event_t tev,
                               process_message_context_t & pmc,
                               client_mb_packet_t const & packet);

  void handle_endorsement_request(
      tracing_event_t tev,
      process_message_context_t & pmc,
      deserialize_packet_result const & dpr);

  void processEncPkg_mb_server(tracing_event_t tev,
                               process_message_context_t & pmc,
                               deserialize_packet_result const & dpr);

  //--------------------------------------------------------------------

  // TODO: process_message_context_t should eventually go away.
  friend class process_message_context_t;

  //--------------------------------------------------------------------

  std::map<link_type_t, std::map<psn_t, std::set<link_id_t>>>
      link_sets_;

  std::list<pooled<outbox_entry_t>> outbox_;
  std::map<race_handle_t, goodbox_entry_t> goodbox_;

  //--------------------------------------------------------------------

  std::uintmax_t network_maintenance_call_id_{0};
  std::chrono::seconds network_maintenance_cooldown_{5};

  std::chrono::seconds link_polling_maintenance_cooldown_{120};
  decltype(sst::mono_time_ns()) link_polling_maintenance_previous_ns_{
      0};

  std::chrono::seconds send_retrying_maintenance_cooldown_{20};
  decltype(sst::mono_time_ns()) send_retrying_maintenance_previous_ns_{
      0};

  std::thread network_maintenance_thread_;
  std::mutex network_maintenance_mutex_;
  std::condition_variable network_maintenance_cond_;
  std::atomic_bool network_maintenance_stop_{false};

  void process_raw_link_ids(
      tracing_event_t tev,
      std::vector<std::string> const & personas,
      std::vector<std::reference_wrapper<std::set<link_id_t>>> const &
          link_sets,
      std::vector<LinkID> const & raw_link_ids);
  void do_network_maintenance(tracing_event_t tev);

  //--------------------------------------------------------------------

  old_config_t old_config_;

  //--------------------------------------------------------------------

  race_handle_t send(tracing_event_t tev,
                     outbox_entry_t & outbox_entry);

  template<class Packet>
  race_handle_t send(tracing_event_t tev,
                     process_message_context_t & pmc,
                     Packet const & packet,
                     phonebook_entry_t const & remote) {
    SST_TEV_ADD(tev);
    try {
      auto p_buf = acquire<std::vector<unsigned char>>();
      auto & buf = *p_buf;
      buf.resize(
          get_serialized_packet_size<decltype(+buf.size())>(packet));

      auto p_id = acquire<std::vector<unsigned char>>();
      auto & id = *p_id;
      id.resize(pkc::auth_id_size<decltype(+id.size())>());

      serialize_packet(SST_TEV_ARG(tev),
                       packet,
                       &buf[0],
                       &id[0],
                       remote);

      return pmc.send_message(SST_TEV_ARG(tev), buf, remote.psn());
    }
    SST_TEV_RETHROW(tev);
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  explicit plugin_t(IRaceSdkNM & sdk);

  ~plugin_t() SST_NOEXCEPT(true) override = default;

  plugin_t(plugin_t const &) = delete;

  plugin_t & operator=(plugin_t const &) = delete;

  plugin_t(plugin_t &&) = delete;

  plugin_t & operator=(plugin_t &&) = delete;

  //--------------------------------------------------------------------
  // IRacePluginNM function overrides
  //--------------------------------------------------------------------
  //
  // How to update these function declarations:
  //
  //    1. Delete all of the function declarations.
  //
  //    2. Copy all pure virtual function declarations from the
  //       IRacePluginNM class.
  //
  //    3. Delete "virtual" everywhere.
  //
  //    4. Replace "= 0" with "final" everywhere.
  //
  //    5. Replace "const T" with "T const" everywhere for all T.
  //

  PluginResponse
  processEncPkg(RaceHandle handle,
                EncPkg const & ePkg,
                std::vector<ConnectionID> const & connIDs) final;

  PluginResponse onPersonaLinksChanged(std::string recipientPersona,
                                       LinkType linkType,
                                       std::vector<LinkID> links) final;

  PluginResponse
  onUserInputReceived(RaceHandle handle,
                      bool answered,
                      std::string const & response) final;

  //--------------------------------------------------------------------
  // start_client_lookup
  //--------------------------------------------------------------------

  void start_client_lookup(tracing_event_t tev,
                           psn_t const & persona);

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_PLUGIN_T_HPP
