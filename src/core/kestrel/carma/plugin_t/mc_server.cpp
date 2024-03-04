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
#include <chrono>
#include <exception>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/SST_UNREACHABLE.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/floor_sqrt.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_string.hpp>

#include <ClrMsg.h>
#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <PluginResponse.h>
#include <RaceLog.h>

#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_vector_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/client_mb_packet_t.hpp>
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/config.h>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_packet_t.hpp>
#include <kestrel/mb_client_packet_t.hpp>
#include <kestrel/mb_mc_up_packet_t.hpp>
#include <kestrel/mc_leader_init_packet_t.hpp>
#include <kestrel/mc_mb_down_packet_t.hpp>
#include <kestrel/mc_v_packet_t.hpp>
#include <kestrel/old_config_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/pooled.hpp>
#include <kestrel/sdk_span_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/secret_sharing.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/span_parents_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

plugin_t::server_session_t *
plugin_t::handle_mb_mc_up_packet(tracing_event_t tev,
                                 process_message_context_t & pmc,
                                 pooled<mb_mc_up_packet_t> up_packet) {
  SST_TEV_TOP(tev);

  server_session_t * session = nullptr;

  auto const & cid = up_packet->cid;
  auto const wanted_it = wanted_ups.find(cid);
  if (wanted_it != wanted_ups.end()) {
    session = &wanted_it->second.get();
    wanted_ups.erase(cid);
    auto const b = session->p_init->cids.cbegin();
    auto const e = session->p_init->cids.cend();
    auto const i = std::find(b, e, cid);
    if (i == e) {
      throw std::runtime_error("cid not found");
    }
    auto const slot = std::distance(b, i);
    session->p_mb_mc_up[slot] = std::move(up_packet);
  } else {
    loose_ups.emplace(cid, std::move(up_packet));
  }

  if (config().local().role() == role_t::mc_leader()) {
    auto const mixsize = old_config_.mixsize;
    if (loose_ups.size() >= mixsize) {

      SST_ASSERT(session == nullptr);
      auto new_session = acquire<server_session_t>();
      new_session->create(SST_TEV_ARG(tev), *this);

      auto init = acquire<mc_leader_init_packet_t>();
      init->type = packet_type_t::mc_leader_init_packet();
      init->mpcid = new_session->mpcid;
      init->mixsize = mixsize;
      sst::checked_resize(init->cids, mixsize);

      if_tracer([&](tracer_t & tracer) {
        auto it = loose_ups.begin();
        sdk_span_t const mix_root(tracer, "carma_mc_leader_mix_root");
        span_parents_t parents = {mix_root};
        for (decltype(+mixsize) i = 0; i != mixsize; ++i) {
          parents.push_back(it->second->origin_span());
          ++it;
        }
        pmc.send_span =
            sdk_span_t(tracer, "carma_mc_leader_mix", parents);
      });

      {
        auto it = loose_ups.begin();
        for (decltype(+mixsize) i = 0; i != mixsize; ++i) {
          init->cids[i] = it->first;
          new_session->p_mb_mc_up[i] = std::move(it->second);
          it = loose_ups.erase(it);
        }
        KESTREL_INFO(sdk(),
                     SST_TEV_ARG(tev,
                                 "event",
                                 "mc_leader_picked_cids",
                                 "mc_leader_picked_cids",
                                 [&]() {
                                   auto xs = nlohmann::json::array();
                                   for (auto const & cid : init->cids) {
                                     xs += cid.to_json();
                                   }
                                   return xs;
                                 }()));
      }

      bool first_iteration = true;
      for (phonebook_pair_t const * const & entry :
           config().local().mc_group(SST_TEV_ARG(tev))) {
        if (!first_iteration) {
          send(SST_TEV_ARG(tev),
               pmc,
               *init,
               *config().phonebook().at(SST_TEV_ARG(tev), *entry));
        }
        first_iteration = false;
      }

      new_session->p_init = std::move(init);
      session = new_session.get();
      sessions_.emplace(new_session->mpcid, std::move(new_session));
    }
  }

  return session;

  SST_TEV_BOT(tev);
}

plugin_t::server_session_t * plugin_t::handle_mb_mc_up_bundle_packet(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  SST_ASSERT(
      (dpr.packet_type == packet_type_t::mb_mc_up_bundle_packet()));

  if (config().local().role() != role_t::mc_leader()) {
    // TODO: Output a warning?
    return nullptr;
  }

  phonebook_pair_t const * const p_mb_server =
      config().phonebook().find(*dpr.psn);
  if (!p_mb_server) {
    // TODO: Output a warning?
    return nullptr;
  }

  std::shared_ptr<phonebook_entry_t const> const mb_server =
      config().phonebook().at(SST_TEV_ARG(tev), *p_mb_server);

  server_session_t * session = nullptr;

  std::vector<unsigned char> const & data = *dpr.packet_data;
  std::vector<unsigned char>::size_type data_i = 0;

  {
    int packet_type;
    deserialize(data, data_i, packet_type);
  }

  node_count_t i = 0U;
  phonebook_vector_t const & mc_group =
      config().local().mc_group(SST_TEV_ARG(tev),
                                config().local().group());
  while (data_i < data.size()) {
    std::vector<unsigned char> buf;
    deserialize(data, data_i, buf);
    if (i == 0U) {
      auto up_packet = acquire<mb_mc_up_packet_t>();
      up_packet->from_bytes_prep(pmc.origin_handle(),
                                 pmc.origin_span(),
                                 prime_size_,
                                 old_config_.mixsize);
      packet_from_bytes_exact(buf, *up_packet);
      session = handle_mb_mc_up_packet(SST_TEV_ARG(tev),
                                       pmc,
                                       std::move(up_packet));
    } else {
      std::shared_ptr<phonebook_entry_t const> const follower =
          config().phonebook().at(SST_TEV_ARG(tev), *mc_group[i]);
      pmc.send_message(SST_TEV_ARG(tev),
                       serialize({}, mb_server->psn(), buf),
                       follower->psn());
    }
    ++i;
  }

  return session;

  SST_TEV_BOT(tev);
}

void plugin_t::mc_server_processEncPkg(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_ADD(tev);
  try {
    local_config_t & local = config().local();

    if (local.role() == role_t::mb_server()) {
      SST_UNREACHABLE();
    } else if (local.role() == role_t::mc_follower()
               || local.role() == role_t::mc_leader()) {
      //----------------------------------------------------------------
      server_session_t * session = nullptr;

      if (dpr.packet_type == packet_type_t::mb_mc_up_bundle_packet()) {
        session =
            handle_mb_mc_up_bundle_packet(SST_TEV_ARG(tev), pmc, dpr);
      } else if (dpr.packet_type == packet_type_t::mb_mc_up_packet()) {

        if (config().phonebook().at(SST_TEV_ARG(tev), *dpr.psn)->role()
            != role_t::mb_server()) {
          throw std::runtime_error("up packet from non-mailbox");
        }

        auto up_packet = acquire<mb_mc_up_packet_t>();
        up_packet->from_bytes_prep(pmc.origin_handle(),
                                   pmc.origin_span(),
                                   prime_size_,
                                   old_config_.mixsize);
        packet_from_bytes_exact(*dpr.packet_data, *up_packet);

        session = handle_mb_mc_up_packet(SST_TEV_ARG(tev),
                                         pmc,
                                         std::move(up_packet));

      } else if (dpr.packet_type
                 == packet_type_t::mc_leader_init_packet()) {

        if (config().phonebook().at(SST_TEV_ARG(tev), *dpr.psn)->role()
            != role_t::mc_leader()) {
          throw std::runtime_error("init packet from non-leader");
        }

        auto packet = acquire<mc_leader_init_packet_t>();
        packet->from_bytes_prep(pmc.origin_handle(),
                                pmc.origin_span(),
                                old_config_.mixsize);
        packet_from_bytes_exact(*dpr.packet_data, *packet);

        session =
            &get_or_deduce_session(SST_TEV_ARG(tev), packet->mpcid);
        auto zzz = session->p_mb_mc_up.begin();
        for (auto const & cid : packet->cids) {
          auto const it = loose_ups.find(cid);
          if (it != loose_ups.end()) {
            *zzz = std::move(it->second);
            loose_ups.erase(it);
          } else {
            wanted_ups.emplace(cid, *session);
          }
          ++zzz;
        }
        session->p_init = std::move(packet);

      } else if (dpr.packet_type == packet_type_t::mc_v_packet()) {

        std::shared_ptr<phonebook_entry_t const> const entry_ptr =
            config().phonebook().at(SST_TEV_ARG(tev), *dpr.psn);
        phonebook_entry_t const & entry = *entry_ptr;

        if (entry.role() != role_t::mc_leader()
            && entry.role() != role_t::mc_follower()) {
          throw std::runtime_error("mc_v_packet from non-mc");
        }

        auto packet = acquire<mc_v_packet_t>();
        packet->from_bytes_prep(pmc.origin_handle(),
                                pmc.origin_span(),
                                prime_size_,
                                old_config_.mixsize);
        packet_from_bytes_exact(*dpr.packet_data, *packet);

        session =
            &get_or_deduce_session(SST_TEV_ARG(tev), packet->mpcid);
        session->p_v_packet[entry.order()] = std::move(packet);

      } else {
        throw std::runtime_error("unexpected packet type: "
                                 + dpr.packet_type.to_string());
      }

      if (session != nullptr) {
        if (session->tick(SST_TEV_ARG(tev), pmc)) {
          sessions_.erase(session->mpcid);
        }
      }

      //----------------------------------------------------------------
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace carma
} // namespace kestrel
