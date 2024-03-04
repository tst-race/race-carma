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
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_THROW.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/floor_sqrt.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/span.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unsigned_ceil_div.hpp>
#include <sst/catalog/unsigned_le.hpp>
#include <sst/catalog/unsigned_min.hpp>

#include <ClrMsg.h>
#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <PluginResponse.h>
#include <RaceLog.h>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/mailbox_message_type_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_vector_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/client_mb_packet_t.hpp>
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/config.h>
#include <kestrel/corruption_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/guid_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_packet_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/logging.hpp>
#include <kestrel/mb_client_packet_t.hpp>
#include <kestrel/mb_mc_up_packet_t.hpp>
#include <kestrel/mc_leader_init_packet_t.hpp>
#include <kestrel/mc_mb_down_packet_t.hpp>
#include <kestrel/mc_v_packet_t.hpp>
#include <kestrel/old_config_t.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/psn_any_hash_t.hpp>
#include <kestrel/psn_default_hash_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_span_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/secret_sharing.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/span_parents_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

bool plugin_t::mb_server_bootstrappee_link_created(
    tracing_event_t tev,
    process_message_context_t & pmc,
    race_handle_t const & handle,
    link_address_t const & cookie) {
  SST_TEV_TOP(tev);

  auto it = mb_server_bootstrappee_link_creations_.find(handle);
  if (it == mb_server_bootstrappee_link_creations_.end()) {
    return false;
  }

  mb_server_bootstrappee_link_creation_t const & info = it->second;

  std::shared_ptr<phonebook_entry_t const> const bootstrappee =
      phonebook().at(SST_TEV_ARG(tev), info.bootstrappee);

  // TODO: We need to send the bootstrappee our MC leader set, as that's
  //       not publicly deducible in a non-rigid deployment.

  pmc.send_message(
      SST_TEV_ARG(tev),
      serialize({},
                local().psn().value(),
                bootstrappee->auth_encrypt(serialize(
                    {},
                    packet_type_t::registration_complete().value(),
                    info.channel.value(),
                    cookie.value()))),
      bootstrappee->psn());

  mb_server_bootstrappee_link_creations_.erase(it);
  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::mailbox_handle_add_contact_request(
    tracing_event_t tev,
    process_message_context_t & pmc,
    bytes_t const & a_data,
    psn_t const & recver_client_psn) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "mailbox_handle_add_contact_request_called"));

  phonebook_pair_t const * const recver_client_pair =
      phonebook().find(recver_client_psn);

  if (!recver_client_pair) {
    KESTREL_WARN(sdk(),
                 SST_TEV_ARG(tev,
                             "event",
                             "add_contact_request_for_unknown_client"));
    return;
  }

  if (!contains(local().bucket_clients(SST_TEV_ARG(tev)),
                *recver_client_pair)) {
    SST_TEV_THROW(tev, corruption_t());
  }

  std::shared_ptr<phonebook_entry_t const> const recver_client =
      phonebook().at(SST_TEV_ARG(tev), *recver_client_pair);

  pmc.send_message(
      SST_TEV_ARG(tev),
      serialize({},
                local().psn(),
                recver_client->auth_encrypt(serialize(
                    {},
                    packet_type_t::add_contact_request().value(),
                    a_data))),
      recver_client_psn);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "mailbox_handle_add_contact_request_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::handle_new_client(tracing_event_t tev,
                                 bytes_t const & a,
                                 psn_t const & bootstrappee_psn) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(sdk(),
                SST_TEV_ARG(tev, "event", "handle_new_client_called"));

  if (phonebook().find(bootstrappee_psn)) {
    // TODO: This is a client name collision. Output a warning?
    return;
  }

  bytes_t::size_type a_i = 0;

  psn_any_hash_t sender_mb_server_hash;
  {
    bytes_t x;
    deserialize(a, a_i, x);
    x = local().anon_decrypt(x);
    bytes_t::size_type x_i = 0;
    deserialize(x, x_i, sender_mb_server_hash);
  }
  phonebook_pair_t const * const sender_mb_server_pair =
      phonebook().find(sender_mb_server_hash);
  if (!sender_mb_server_pair) {
    throw corruption_t();
  }
  std::shared_ptr<phonebook_entry_t const> const sender_mb_server =
      phonebook().at(SST_TEV_ARG(tev), *sender_mb_server_pair);
  if (sender_mb_server->role() != role_t::mb_server()) {
    throw corruption_t();
  }

  bytes_t chunking_id;
  bytes_t::size_type chunk_index;
  bytes_t::size_type chunk_count;
  bytes_t chunk;
  {
    bytes_t x;
    deserialize(a, a_i, x);
    x = sender_mb_server->auth_decrypt(x);
    bytes_t::size_type x_i;
    deserialize(x, x_i, chunking_id, chunk_index, chunk_count, chunk);
  }
  if (chunk_index >= chunk_count) {
    throw corruption_t();
  }

  sst::optional<bytes_t> const maybe_info =
      chunk_joiner().add(SST_TEV_ARG(tev),
                         chunking_id,
                         chunk_index,
                         chunk_count,
                         std::move(chunk));
  if (maybe_info) {
    bytes_t const info = local().anon_decrypt(*maybe_info);
    bytes_t::size_type info_i = 0;
    psn_t psn;
    bytes_t pk;
    bytes_t nonce;
    bytes_t epoch_nonce;
    std::string channel;
    std::string cookie;
    deserialize(info,
                info_i,
                psn,
                pk,
                nonce,
                epoch_nonce,
                channel,
                cookie);
    if (psn != bootstrappee_psn) {
      throw corruption_t();
    }
    if (epoch_nonce != global().epoch_nonce()) {
      return;
    }

    // TODO: Right now we're just going to believe this endorsement.
    //       This is an agreeance threshold of q=1. Eventually we want
    //       to support setting q>1 by collecting endorsements and
    //       waiting for q to match from different sender mailboxes.

    std::shared_ptr<phonebook_entry_t const> const bootstrappee =
        [&]() {
          phonebook_entry_t x;
          x.set_phonebook(phonebook());
          x.set_psn(bootstrappee_psn);
          x.pk(pk);
          x.set_role(role_t::client());
          return phonebook().add_slow(SST_TEV_ARG(tev), std::move(x));
        }();

    load_link(SST_TEV_ARG(tev),
              channel_id_t(channel),
              link_address_t(cookie),
              bootstrappee->psn());

    auto const channel_it = channels_.find(channel_id_t(channel));
    if (channel_it != channels_.end()) {
      channel_t & channel = channel_it->second;
      if (channel.properties().linkDirection == LD_LOADER_TO_CREATOR) {
        mb_server_bootstrappee_link_creations_.emplace(
            std::piecewise_construct,
            std::make_tuple(create_link(SST_TEV_ARG(tev),
                                        channel,
                                        bootstrappee->psn())),
            std::make_tuple(mb_server_bootstrappee_link_creation_t{
                bootstrappee->psn(),
                channel.id()}));
      }
    }

  } //

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_new_client_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::handle_client_mb_packet(
    tracing_event_t tev,
    process_message_context_t & pmc,
    client_mb_packet_t const & packet) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_client_mb_packet_called"));

  local_config_t const & local = config().local();

  mb_processed_cids_.insert(packet.cid);
  auto const ms = old_config_.mixsize;

  std::vector<sst::bigint> y;
  sst::checked_resize(y, ms);
  y[0] = packet.x;
  for (decltype(y.size()) i = 1; i < y.size(); ++i) {
    y[i] = mul_mod(packet.x, y[i - 1], old_config_.prime);
  }

  phonebook_vector_t const & mc_group =
      local.mc_group(SST_TEV_ARG(tev), packet.mc_group_number);
  auto const mc_count = mc_group.size();

  std::vector<std::vector<sst::bigint>> all_z;
  sst::checked_resize(all_z, ms);
  for (decltype(all_z.size()) i = 0; i < all_z.size(); ++i) {
    sst::checked_resize(all_z[i], mc_count);
    sss_share(all_z[i],
              y[i],
              mc_count,
              old_config_.threshold,
              old_config_.prime);
  }

  auto packets_ptr = acquire<std::vector<mb_mc_up_packet_t>>();
  auto & packets = *packets_ptr;
  sst::checked_resize(packets, mc_count);
  for (decltype(+mc_count) j = 0; j != mc_count; ++j) {
    auto const p = &packets[j];
    p->type = packet_type_t::mb_mc_up_packet();
    p->prime_size = prime_size_;
    p->mixsize = old_config_.mixsize;
    p->cid = packet.cid;
    sst::checked_resize(p->z, ms);
    for (decltype(p->z.size()) i = 0; i < p->z.size(); ++i) {
      p->z[i] = std::move(all_z[i][j]);
    }
  }

  std::vector<unsigned char> combined =
      serialize({}, packet_type_t::mb_mc_up_bundle_packet().value());
  std::vector<unsigned char> buf;
  auto member = mc_group.begin();
  for (decltype(+mc_count) j = 0; j != mc_count; ++j, ++member) {
    std::shared_ptr<phonebook_entry_t const> const pbe =
        config().phonebook().at(SST_TEV_ARG(tev), **member);
    buf.resize(packets[j].to_bytes_size<decltype(buf.size())>());
    packets[j].to_bytes(buf.begin());
    if (j == 0) {
      serialize(combined, buf);
    } else {
      serialize(combined,
                pkc::auth_encrypt(local.sk(), pbe->pk(), buf).first);
    }
  }
  std::shared_ptr<phonebook_entry_t const> const ldr =
      config().phonebook().at(SST_TEV_ARG(tev), **mc_group.begin());
  buf.clear();
  serialize(buf,
            local.psn(),
            pkc::auth_encrypt(local.sk(), ldr->pk(), combined).first);
  pmc.send_message(SST_TEV_ARG(tev), buf, ldr->psn());

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_client_mb_packet_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::handle_endorsement_request(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_endorsement_request_called"));

  SST_ASSERT((dpr.packet_type == packet_type_t::endorsement_request()));

  bytes_t const & packet = *dpr.packet_data;
  bytes_t::size_type packet_i = dpr.body_offset;

  psn_any_hash_t mc_leader_hash;
  deserialize(packet, packet_i, mc_leader_hash);
  phonebook_pair_t const * const mc_leader_pair =
      phonebook().find(mc_leader_hash);
  if (!mc_leader_pair) {
    throw corruption_t();
  }
  if (!contains(local().mc_leaders(SST_TEV_ARG(tev)),
                *mc_leader_pair)) {
    throw corruption_t();
  }
  std::shared_ptr<phonebook_entry_t const> const mc_leader =
      phonebook().at(SST_TEV_ARG(tev), *mc_leader_pair);

  bytes_t a;
  bytes_t b;
  bytes_t c;
  deserialize(packet, packet_i, a, b, c);

  bytes_t::size_type a_i = 0;
  bytes_t::size_type b_i = 0;
  bytes_t::size_type c_i = 0;
  while (a_i < a.size()) {

    // TODO: recver_mb_psn can be changed to recver_mb_hash
    //       (psn_any_hash_t). The sending code is in
    //       onBootstrapPkgReceived.
    psn_t recver_mb_psn;
    bytes_t r;
    bytes_t bb;
    bytes_t cc;
    deserialize(a, a_i, recver_mb_psn, r);
    deserialize(b, b_i, bb);
    deserialize(c, c_i, cc);

    phonebook_pair_t const * const recver_mb_pair =
        phonebook().find(recver_mb_psn);
    if (!recver_mb_pair) {
      throw corruption_t();
      return;
    }
    std::shared_ptr<phonebook_entry_t const> const recver_mb_server =
        phonebook().at(SST_TEV_ARG(tev), *recver_mb_pair);
    if (recver_mb_server->role() != role_t::mb_server()) {
      throw corruption_t();
      return;
    }

    bytes_t const chunking_id = sst::crypto_rng(10);

    // Figure out how many chunks we need to make.
    bytes_t::size_type chunk_size = r.size();
    bytes_t::size_type chunk_count;
    while (true) {
      chunk_count = sst::unsigned_ceil_div(r.size(), chunk_size);
      bytes_t::size_type const n =
          serialize_size(
              serialize_size(global().anon_encrypt_size(
                                 serialize_size(local().psn_hash())),
                             global().auth_encrypt_size(serialize_size(
                                 chunking_id,
                                 chunk_count,
                                 chunk_count,
                                 serialize_size_t{chunk_size}))),
              bb,
              cc)
              .value;
      if (sst::unsigned_le(n, global().prime_space())) {
        break;
      }
      bytes_t::size_type const d =
          n - static_cast<bytes_t::size_type>(global().prime_space());
      if (d >= chunk_size) {
        throw std::runtime_error("Impossible to fit under prime");
      }
      chunk_size -= d;
    }

    unsigned char const * rp = r.data();
    bytes_t::size_type rn = r.size();
    for (bytes_t::size_type i = 0; i < chunk_count; ++i) {
      bytes_t::size_type const rk = sst::unsigned_min(rn, chunk_size);
      client_mb_packet_t packet;
      packet.prime_size = prime_size_;
      packet.cid = guid_t::generate();
      bytes_t const x =
          serialize({},
                    serialize({},
                              recver_mb_server->anon_encrypt(
                                  serialize({}, local().psn_hash())),
                              recver_mb_server->auth_encrypt(
                                  serialize({},
                                            chunking_id,
                                            i,
                                            chunk_count,
                                            sst::span(rp, rk)))),
                    bb,
                    cc);
      SST_ASSERT((sst::unsigned_le(x.size(), global().prime_space())));
      packet.x = sst::bigint(x, sst::integer_rep::pure_unsigned());
      SST_ASSERT((packet.x < global().prime()));
      packet.mc_group_number = mc_leader->group();
      handle_client_mb_packet(SST_TEV_ARG(tev), pmc, packet);
      rp += rk;
      rn -= rk;
    }

  } //

  KESTREL_TRACE(sdk(),
                SST_TEV_ARG(tev,
                            "event",
                            "handle_endorsement_request_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::processEncPkg_mb_server(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  local_config_t const & local = config().local();

  phonebook_entry_t const & remote = *dpr.node_info;
  switch (remote.role()) {
    case role_t::client(): {
      if (!contains(remote.bucket_mb_servers(SST_TEV_ARG(tev)),
                    local.psn())) {
        throw std::runtime_error("I'm not a mailbox for this client");
      }
      if (dpr.packet_type == packet_type_t::link_address_packet()) {
        auto const packet_ptr = acquire<link_address_packet_t>();
        auto & packet = *packet_ptr;
        packet.from_bytes_prep();
        packet_from_bytes_exact(*dpr.packet_data, packet);
        load_link(SST_TEV_ARG(tev),
                  packet.channel_id,
                  packet.link_address,
                  *dpr.psn);
      } else if (dpr.packet_type == packet_type_t::client_mb_packet()) {
        auto p_packet = acquire<client_mb_packet_t>();
        auto & packet = *p_packet;
        packet.from_bytes_prep(pmc.origin_handle(),
                               pmc.origin_span(),
                               prime_size_);
        packet_from_bytes_exact(*dpr.packet_data, packet);
        if (mb_processed_cids_.count(packet.cid) != 0) {
          KESTREL_TRACE(
              sdk(),
              SST_TEV_ARG(tev, "event", "duplicate_client_mb_cid"));
          return;
        }
        handle_client_mb_packet(SST_TEV_ARG(tev), pmc, packet);
      } else if (dpr.packet_type
                 == packet_type_t::endorsement_request()) {
        handle_endorsement_request(SST_TEV_ARG(tev), pmc, dpr);
      } else {
        throw std::runtime_error("unexpected packet type: "
                                 + dpr.packet_type.to_string());
      }

    } break;
    case role_t::mc_leader():
    case role_t::mc_follower(): {

      if (dpr.packet_type != packet_type_t::mc_mb_down_packet()) {
        throw std::runtime_error("unexpected packet type: "
                                 + dpr.packet_type.to_string());
      }
      auto down_packet = acquire<mc_mb_down_packet_t>();
      down_packet->from_bytes_prep(pmc.origin_handle(),
                                   pmc.origin_span());
      packet_from_bytes_exact(*dpr.packet_data, *down_packet);

      {
        auto const it = mb_processed_oids_.find(down_packet->oid);
        if (it != mb_processed_oids_.end()) {
          if_tracer([&](tracer_t & tracer) {
            pmc.send_span = sdk_span_t(
                tracer,
                "carma_mb_server_bonus_oid",
                span_parents_t{pmc.origin_span(), it->second});
          });
          it->second = pmc.send_span;
          KESTREL_TRACE(
              sdk(),
              SST_TEV_ARG(tev, "event", "carma_mb_server_bonus_oid"));
          return;
        }
        if_tracer([&](tracer_t & tracer) {
          pmc.send_span = sdk_span_t(tracer,
                                     "carma_mb_server_first_oid",
                                     span_parents_t{pmc.origin_span()});
        });
        mb_processed_oids_.emplace(down_packet->oid, pmc.send_span);
      }

      std::vector<unsigned char> const buf =
          pkc::anon_decrypt(config().local().pk(),
                            config().local().sk(),
                            down_packet->b)
              .first;
      decltype(buf.size()) buf_i = 0;

      mailbox_message_type_t command;
      psn_t client_psn;
      deserialize(buf, buf_i, command, client_psn);

      switch (command) {
        case mailbox_message_type_t::add_contact_request(): {
          mailbox_handle_add_contact_request(
              SST_TEV_ARG(tev),
              pmc,
              local.anon_decrypt(down_packet->a),
              client_psn);
        } break;
        case mailbox_message_type_t::mail_delivery(): {

          std::shared_ptr<phonebook_entry_t const> const entry_ptr =
              config().phonebook().at(SST_TEV_ARG(tev), client_psn);
          phonebook_entry_t const & entry = *entry_ptr;
          if (!contains(entry.bucket_mb_servers(SST_TEV_ARG(tev)),
                        local.psn())) {
            throw std::runtime_error(
                "I'm not a mailbox for the referenced client");
          }

          auto mb_client_packet = acquire<mb_client_packet_t>();
          mb_client_packet->type = packet_type_t::mb_client_packet();
          mb_client_packet->a = down_packet->a;
          mb_client_packet->oid = down_packet->oid;
          send(SST_TEV_ARG(tev),
               pmc,
               *mb_client_packet,
               *config().phonebook().at(SST_TEV_ARG(tev), client_psn));

        } break;
        case mailbox_message_type_t::new_client(): {
          handle_new_client(SST_TEV_ARG(tev),
                            down_packet->a,
                            client_psn);
        } break;
      }

    } break;
    case role_t::mb_server(): {
      // TODO bad
    } break;
    case role_t::idle_server(): {
      // TODO bad
    } break;
    default: {
      throw std::runtime_error("missing case in switch (remote.role)");
    } break;
  }

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
