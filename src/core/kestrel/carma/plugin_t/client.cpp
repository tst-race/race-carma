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
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/span.hpp>
#include <sst/catalog/to_unsigned.hpp>
#include <sst/catalog/unsigned_ceil_div.hpp>
#include <sst/catalog/unsigned_max.hpp>
#include <sst/catalog/unsigned_min.hpp>

#include <ChannelProperties.h>
#include <ClrMsg.h>
#include <LinkProperties.h>

#include <kestrel/CARMA_XLOG_INFO.hpp>
#include <kestrel/CARMA_XLOG_TRACE.hpp>
#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/bootstrap_config_t.hpp>
#include <kestrel/carma/client_message_type_t.hpp>
#include <kestrel/carma/clrmsg_store_entry_t.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/mailbox_message_type_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/client_mb_packet_t.hpp>
#include <kestrel/clrmsg_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/detached_clrmsg_store_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/logging.hpp>
#include <kestrel/mb_client_packet_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/psn_default_hash_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------

void plugin_t::inner_processClrMsg2(
    tracing_event_t tev,
    process_message_context_t & pmc,
    phonebook_entry_t const & recver_client,
    bytes_t const & a_data,
    mailbox_message_type_t const recver_mb_cmd) {
  SST_TEV_TOP(tev);

  // TODO: We'll also be adding support for new_client here.
  SST_ASSERT((recver_mb_cmd == mailbox_message_type_t::mail_delivery()
              || recver_mb_cmd
                     == mailbox_message_type_t::add_contact_request()));

  local_config_t const & local = config().local();

  std::vector<psn_t> local_mailboxes;
  for (phonebook_pair_t const * const & psn_pbe :
       local.bucket_mb_servers(SST_TEV_ARG(tev))) {
    if (!config()
             .phonebook()
             .at(SST_TEV_ARG(tev), *psn_pbe)
             ->mc_leaders(SST_TEV_ARG(tev))
             .empty()) {
      local_mailboxes.emplace_back(psn_pbe->first);
    }
  }

  if (local_mailboxes.empty()) {
    throw std::runtime_error("No sender MB servers");
  }

  std::vector<psn_t> target_mailboxes;
  for (phonebook_pair_t const * const & psn_pbe :
       recver_client.bucket_mb_servers(SST_TEV_ARG(tev))) {
    target_mailboxes.emplace_back(psn_pbe->first);
  }

  if (target_mailboxes.empty()) {
    throw std::runtime_error("No recver MB servers");
  }

  std::shuffle(local_mailboxes.begin(),
               local_mailboxes.end(),
               sst::crypto_rng());

  std::shuffle(target_mailboxes.begin(),
               target_mailboxes.end(),
               sst::crypto_rng());

  auto const p =
      sst::unsigned_min(sst::unsigned_max(local_mailboxes.size(),
                                          target_mailboxes.size()),
                        config().global().num_parallel_messages());

  auto local_mailbox = local_mailboxes.end();
  auto target_mailbox = target_mailboxes.end();

  bytes_t const b_data =
      serialize({}, recver_mb_cmd, recver_client.psn());

  for (decltype(+p) i{}; i < p; ++i) {

    if (local_mailbox == local_mailboxes.end()
        || ++local_mailbox == local_mailboxes.end()) {
      local_mailbox = local_mailboxes.begin();
    }

    if (target_mailbox == target_mailboxes.end()
        || ++target_mailbox == target_mailboxes.end()) {
      target_mailbox = target_mailboxes.begin();
    }

    auto const & local_mb_psn = *local_mailbox;
    std::shared_ptr<phonebook_entry_t const> const local_mb_node_ptr =
        config().phonebook().at(SST_TEV_ARG(tev), local_mb_psn);
    auto const & local_mb_node = *local_mb_node_ptr;

    // TODO: We could have .mc_group_numbers() that gives us a vector
    //       of all group numbers. Then we can just pick a random
    //       group from there.
    // TODO: We should have .random_mc_leader() or .random_mc_group()
    //       at some point.
    auto const & leaders = local_mb_node.mc_leaders(SST_TEV_ARG(tev));
    SST_ASSERT((!leaders.empty()));
    auto leader_it = leaders.begin();
    std::advance(
        leader_it,
        sst::rand_range(sst::to_unsigned(leaders.size()) - 1U));
    std::shared_ptr<phonebook_entry_t const> const mc_leader =
        phonebook().at(SST_TEV_ARG(tev), **leader_it);
    node_count_t const mc_group_number = mc_leader->group();

    std::shared_ptr<phonebook_entry_t const> const recver_mb_server =
        config().phonebook().at(SST_TEV_ARG(tev), *target_mailbox);

    bytes_t const c_data = serialize({}, recver_mb_server->psn_hash());

    phonebook_entry_t const & a_dest =
        recver_mb_cmd == mailbox_message_type_t::add_contact_request() ?
            *recver_mb_server :
            recver_client;

    bytes_t const a = a_dest.anon_encrypt(a_data);
    bytes_t const b = recver_mb_server->anon_encrypt(b_data);
    bytes_t const c = mc_leader->anon_encrypt(c_data);

    client_mb_packet_t packet;
    packet.prime_size = prime_size_;
    packet.cid = guid_t::generate();
    packet.x = sst::bigint(serialize({}, a, b, c),
                           sst::integer_rep::pure_unsigned());
    packet.mc_group_number = mc_group_number;
    SST_ASSERT((packet.x <= global().prime()));

    CARMA_XLOG_TRACE(sdk_,
                     0,
                     SST_TEV_ARG(tev,
                                 "event",
                                 "client_mb_packet_created",
                                 "packet_data",
                                 packet.to_json()));

    send(SST_TEV_ARG(tev), pmc, packet, local_mb_node);
  }

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::handle_attached_clrmsg(
    tracing_event_t tev,
    process_message_context_t & pmc,
    phonebook_entry_t const & recver,
    bytes_t const & a_data,
    mailbox_message_type_t const recver_mb_cmd) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_attached_clrmsg_called"));

  // TODO: When we unify bootstrapping with this function, we'll also
  //       support MB server as a recver role here.
  SST_ASSERT((recver.role() == role_t::client()));

  bytes_t const nonce = sst::crypto_rng(16);

  // Figure out how many chunks we need to make.
  bytes_t::size_type chunk_size = a_data.size();
  bytes_t::size_type chunk_count;
  while (true) {
    chunk_count = sst::unsigned_ceil_div(a_data.size(), chunk_size);
    bytes_t::size_type const n =
        serialize_size(
            global().anon_encrypt_size(
                serialize_size(nonce,
                               chunk_count,
                               chunk_count,
                               serialize_size_t{chunk_size})),
            global().anon_encrypt_size(
                serialize_size(recver_mb_cmd, recver.psn())),
            serialize_size(global().anon_encrypt_size(
                serialize_size_t{psn_default_hash_t::size()})))
            .value;
    if (n <= global().prime_space()) {
      break;
    }
    bytes_t::size_type const d = n - global().prime_space();
    if (d >= chunk_size) {
      throw std::runtime_error("Impossible to fit under prime");
    }
    chunk_size -= d;
  }

  unsigned char const * p = a_data.data();
  bytes_t::size_type n = a_data.size();
  for (bytes_t::size_type i = 0; i < chunk_count; ++i) {
    bytes_t::size_type const k = sst::unsigned_min(n, chunk_size);
    inner_processClrMsg2(
        SST_TEV_ARG(tev),
        pmc,
        recver,
        serialize({}, nonce, i, chunk_count, sst::span(p, k)),
        recver_mb_cmd);
    p += k;
    n -= k;
  }

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_attached_clrmsg_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::handle_detached_clrmsg(tracing_event_t tev,
                                      race_handle_t const & handle,
                                      clrmsg_t const & clrmsg,
                                      process_message_context_t & pmc) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_detached_clrmsg_called"));

  {
    sst::unique_ptr<detached_clrmsg_store_t::entry_t> p{sst::in_place};
    p->handle = handle;
    p->clrmsg = clrmsg;
    config()
        .detached_clrmsg_store(SST_TEV_ARG(tev))
        .add(SST_TEV_ARG(tev), std::move(p));
  }

  phonebook_entry_t recver_client;
  recver_client.set_phonebook(phonebook());
  recver_client.set_psn(clrmsg.recver());
  recver_client.set_role(role_t::client());

  // TODO: We need to remember this nonce so we can verify it in the
  //       eventual response from the recver client.
  bytes_t const nonce = sst::crypto_rng(16);

  bytes_t a_data;
  serialize(a_data,
            local().psn(),
            local().pk(),
            nonce,
            clrmsg.recver());

  process_message_context_t pmc2(sdk(), *this);

  handle_attached_clrmsg(SST_TEV_ARG(tev),
                         pmc2,
                         recver_client,
                         a_data,
                         mailbox_message_type_t::add_contact_request());

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "handle_detached_clrmsg_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------
//
// TODO: This function should get split up into
//       {client,mb_server,...}_processClrMsg.
//

void plugin_t::inner_processClrMsg(tracing_event_t tev,
                                   race_handle_t const & handle,
                                   ClrMsg const & msg) {
  SST_TEV_ADD(tev);
  try {

    global_config_t const & global = config().global();
    local_config_t const & local = config().local();

    if (local.role() != role_t::client()) {
      return;
    }

    process_message_context_t pmc(sdk_, *this, handle, msg);

    clrmsg_t const clrmsg(msg);

    SST_TEV_ADD(tev, "incoming_clrmsg", clrmsg.to_json());
    CARMA_XLOG_INFO(sdk_,
                    0,
                    SST_TEV_ARG(tev, "event", "incoming_clrmsg"));

    std::vector<unsigned char> const clrmsg_blob = [&] {
      std::vector<unsigned char> v;
      using sz = decltype(v.size());
      v.resize(clrmsg.to_bytes_size<sz>());
      clrmsg.to_bytes(v.begin());
      return v;
    }();

    //------------------------------------------------------------------
    //
    // If the message's "from" persona doesn't match our persona,
    // something is wrong.
    //

    if (clrmsg.sender() != persona()) {
      throw std::runtime_error(
          "clrmsg.sender() (" + sst::c_quote(clrmsg.sender().string())
          + ") does not match persona() ("
          + sst::c_quote(persona().string()) + ").");
    }

    //------------------------------------------------------------------

    phonebook_pair_t const * const recver_client_pair =
        config().phonebook().find(clrmsg.recver());
    if (recver_client_pair) {
      std::shared_ptr<phonebook_entry_t const> const recver_client =
          phonebook().at(SST_TEV_ARG(tev), *recver_client_pair);
      bytes_t const a_data =
          serialize({}, client_message_type_t::message(), clrmsg_blob);
      handle_attached_clrmsg(SST_TEV_ARG(tev),
                             pmc,
                             *recver_client,
                             a_data,
                             mailbox_message_type_t::mail_delivery());
    } else {
      handle_detached_clrmsg(SST_TEV_ARG(tev), handle, clrmsg, pmc);
    }

    CARMA_XLOG_TRACE(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "processClrMsg_success"));

  } //
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------

void plugin_t::client_processEncPkg(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  phonebook_entry_t const & remote = *dpr.node_info;

  switch (remote.role()) {
    case role_t::mb_server(): {
      if (!contains(local().bucket_mb_servers(SST_TEV_ARG(tev)),
                    *dpr.psn)) {
        throw std::runtime_error("not my mailbox");
      }

      if (dpr.packet_type == packet_type_t::add_contact_request()) {
        client_handle_add_contact_request(SST_TEV_ARG(tev), pmc, dpr);
        return;
      }

      if (dpr.packet_type == packet_type_t::registration_complete()) {
        bootstrappee_on_registration_complete(SST_TEV_ARG(tev), dpr);
        return;
      }

      if (dpr.packet_type != packet_type_t::mb_client_packet()) {
        throw std::runtime_error("unexpected packet type: "
                                 + dpr.packet_type.to_string());
      }
      auto packet = acquire<mb_client_packet_t>();
      packet->from_bytes_prep(pmc.origin_handle(), pmc.origin_span());
      packet_from_bytes_exact(*dpr.packet_data, *packet);

      bytes_t const buf = local().anon_decrypt(packet->a);
      bytes_t::size_type buf_i = 0;

      bytes_t chunk_group;
      bytes_t::size_type chunk_index;
      bytes_t::size_type chunk_count;
      bytes_t chunk_data;
      deserialize(buf,
                  buf_i,
                  chunk_group,
                  chunk_index,
                  chunk_count,
                  chunk_data);

      sst::optional<bytes_t> const combined =
          config().chunk_joiner().add(SST_TEV_ARG(tev),
                                      chunk_group,
                                      chunk_index,
                                      chunk_count,
                                      chunk_data);

      if (combined) {
        bytes_t::size_type combined_i = 0;
        client_message_type_t message_type;
        deserialize(*combined, combined_i, message_type);
        switch (message_type) {

          case client_message_type_t::add_contact_response(): {

            client_handle_add_contact_response(SST_TEV_ARG(tev),
                                               pmc,
                                               *combined,
                                               combined_i);

          } break;

          case client_message_type_t::message(): {

            clrmsg_t clrmsg;
            auto avail = combined->size() - combined_i;
            clrmsg.from_bytes_init();
            {
              bytes_t::size_type unused;
              deserialize(*combined, combined_i, unused);
            }
            clrmsg.from_bytes(combined->data() + combined_i, avail);

            SST_TEV_ADD(tev, "reconstructed_clrmsg", clrmsg.to_json());
            KESTREL_INFO(
                sdk(),
                SST_TEV_ARG(tev, "event", "reconstructed_clrmsg"));

            auto const h = clrmsg.content_hash();
            if (clrmsg_seen_.find(h) != clrmsg_seen_.end()) {
              KESTREL_TRACE(
                  sdk(),
                  SST_TEV_ARG(tev, "event", "duplicate_clrmsg"));
              return;
            }
            clrmsg_seen_.emplace(h);

            sdk().present(SST_TEV_ARG(tev), clrmsg.to_ClrMsg());

          } break;

        } //
      }

    } break;
    default: {
    } break;
  }

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::client_handle_add_contact_request(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "client_handle_add_contact_request_called"));

  bytes_t const & packet = *dpr.packet_data;
  bytes_t::size_type packet_i = dpr.body_offset;
  bytes_t a_data;
  deserialize(packet, packet_i, a_data);

  bytes_t::size_type a_data_i = 0;
  bytes_t chunk_group;
  bytes_t::size_type chunk_index;
  bytes_t::size_type chunk_count;
  bytes_t chunk_data;
  deserialize(a_data,
              a_data_i,
              chunk_group,
              chunk_index,
              chunk_count,
              chunk_data);

  sst::optional<bytes_t> const combined =
      config().chunk_joiner().add(SST_TEV_ARG(tev),
                                  chunk_group,
                                  chunk_index,
                                  chunk_count,
                                  chunk_data);

  if (combined) {

    bytes_t::size_type combined_i = 0;
    psn_t sender_client_psn;
    bytes_t sender_client_pk;
    bytes_t nonce;
    psn_t recver_client_psn;
    deserialize(*combined,
                combined_i,
                sender_client_psn,
                sender_client_pk,
                nonce,
                recver_client_psn);

    if (recver_client_psn != local().psn()) {
      throw corruption_t();
    }

    phonebook_pair_t const * const sender_client_pair =
        phonebook().find(sender_client_psn);
    std::shared_ptr<phonebook_entry_t const> sender_client;
    if (sender_client_pair) {
      sender_client =
          phonebook().at(SST_TEV_ARG(tev), *sender_client_pair);
      // TODO: Verify sender_client_pk against sender_client->pk()?
    } else {
      // TODO: Need to validate sender_client_pk.size()
      phonebook_entry_t e;
      e.set_phonebook(phonebook());
      e.set_psn(sender_client_psn);
      e.pk(sender_client_pk);
      e.set_role(role_t::client());
      sender_client =
          phonebook().add_slow(SST_TEV_ARG(tev), std::move(e));
    }

    process_message_context_t pmc2(sdk(), *this);

    handle_attached_clrmsg(
        SST_TEV_ARG(tev),
        pmc2,
        *sender_client,
        serialize({},
                  client_message_type_t::add_contact_response(),
                  local().psn(),
                  local().pk(),
                  nonce),
        mailbox_message_type_t::mail_delivery());

  } //

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "client_handle_add_contact_request_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::client_handle_add_contact_response(
    tracing_event_t tev,
    process_message_context_t & pmc,
    bytes_t const & a_data,
    bytes_t::size_type a_data_i) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "client_handle_add_contact_response_called"));

  psn_t sender_client_psn;
  bytes_t sender_client_pk;
  bytes_t nonce;
  deserialize(a_data,
              a_data_i,
              sender_client_psn,
              sender_client_pk,
              nonce);

  // TODO: Check the pk size.
  // TODO: Check that the nonce is one we sent earlier.

  phonebook_pair_t const * const sender_client_pair =
      phonebook().find(sender_client_psn);
  std::shared_ptr<phonebook_entry_t const> sender_client;
  if (sender_client_pair) {
    sender_client =
        phonebook().at(SST_TEV_ARG(tev), *sender_client_pair);
  } else {
    phonebook_entry_t entry;
    entry.set_phonebook(phonebook());
    entry.set_psn(sender_client_psn);
    entry.pk(sender_client_pk);
    entry.set_role(role_t::client());
    sender_client =
        phonebook().add_slow(SST_TEV_ARG(tev), std::move(entry));
  }

  while (true) {
    sst::unique_ptr<detached_clrmsg_store_t::entry_t> * const p =
        config()
            .detached_clrmsg_store(SST_TEV_ARG(tev))
            .get(SST_TEV_ARG(tev), sender_client->psn());
    if (!p) {
      break;
    }
    // TODO: This code is duplicated between here and the usual
    //       processClrMsg handler. They can surely be unified somehow.
    std::vector<unsigned char> const clrmsg_blob = [&] {
      std::vector<unsigned char> v;
      using sz = decltype(v.size());
      v.resize((*p)->clrmsg.to_bytes_size<sz>());
      (*p)->clrmsg.to_bytes(v.begin());
      return v;
    }();
    bytes_t const a_data =
        serialize({}, client_message_type_t::message(), clrmsg_blob);
    // TODO: Using to_ClrMsg() here to help get the original pmc context
    //       isn't very nice, but it works. Can we do better?
    process_message_context_t pmc2(sdk(),
                                   *this,
                                   (*p)->handle,
                                   (*p)->clrmsg.to_ClrMsg());
    handle_attached_clrmsg(SST_TEV_ARG(tev),
                           pmc2,
                           *sender_client,
                           a_data,
                           mailbox_message_type_t::mail_delivery());
    p->reset();
  }

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "client_handle_add_contact_response_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

void plugin_t::bootstrappee_on_registration_complete(
    tracing_event_t tev,
    deserialize_packet_result const & dpr) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "bootstrappee_on_registration_complete_called"));

  bytes_t const & packet = *dpr.packet_data;
  bytes_t::size_type packet_i = dpr.body_offset;
  std::string channel;
  std::string cookie;
  deserialize(packet, packet_i, channel, cookie);
  load_link(SST_TEV_ARG(tev),
            channel_id_t(channel),
            link_address_t(cookie),
            *dpr.psn);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev,
                  "event",
                  "bootstrappee_on_registration_complete_succeeded"));

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_1(tracing_event_t tev) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_1_called"));

  if (bootstrappee_step_1_done_) {
    return false;
  }

  if (!config().bootstrap().exists()) {
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_1_started"));

  config().local().set_psn(persona());
  auto const pksk = pkc::generate_keypair();
  config().local().pk(pksk.first);
  config().local().sk(pksk.second);
  config().phonebook().add_slow(SST_TEV_ARG(tev), config().local());

  auto const it = channels_.find(config().bootstrap().channel_id());
  if (it == channels_.end()) {
    throw std::runtime_error("Missing bootstrap channel");
  }
  bootstrappee_reply_channel_ = &it->second;

  for (auto const & kv : channels_) {
    channel_t const & channel = kv.second;
    if (channel.properties().connectionType
            == connection_type_t::indirect().value()
        && !channel.properties().bootstrap) {
      bootstrappee_mb_channels_.emplace(&channel, false);
    }
  }

  bootstrappee_step_1_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_1_finished"));

  // The reply channel might already be available, in which case we need
  // to make this call here because onChannelStatusChanged() might never
  // make this call.
  bootstrappee_step_2(SST_TEV_ARG(tev), *bootstrappee_reply_channel_);

  // Some MB channels might already be available, in which case we need
  // to make these calls here because onChannelStatusChanged() might
  // never make these calls.
  for (auto const & kv : bootstrappee_mb_channels_) {
    bootstrappee_step_5(SST_TEV_ARG(tev), *kv.first);
  }

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_2(tracing_event_t tev,
                                   channel_t const & channel) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_2_called"));

  if (!bootstrappee_step_1_done_) {
    return false;
  }

  if (bootstrappee_step_2_done_) {
    return false;
  }

  SST_ASSERT((config().bootstrap().exists()));

  if (&channel != bootstrappee_reply_channel_) {
    return false;
  }

  if (channel.properties().channelStatus != CHANNEL_AVAILABLE) {
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_2_started"));

  bootstrappee_reply_link_load_handle_ =
      race_handle_t(sdk().loadLinkAddress(
          SST_TEV_ARG(tev),
          config().bootstrap().channel_id().value(),
          config().bootstrap().link_address().value(),
          std::vector<std::string>{
              config().bootstrap().bootstrapper().value(),
          },
          0));

  bootstrappee_step_2_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_2_finished"));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_3(tracing_event_t tev,
                                   race_handle_t const & handle,
                                   link_id_t const & link_id,
                                   link_status_t const & link_status,
                                   LinkProperties const &) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_3_called"));

  if (!bootstrappee_step_2_done_) {
    return false;
  }

  if (bootstrappee_step_3_done_) {
    return false;
  }

  if (handle != bootstrappee_reply_link_load_handle_) {
    return false;
  }

  if (link_status != link_status_t::loaded()) {
    // TODO: This might mean something went wrong. What can we do?
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_3_started"));

  bootstrappee_reply_connection_open_handle_ =
      openConnection(SST_TEV_ARG(tev), link_id, link_type_t::send());

  bootstrappee_step_3_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_3_finished"));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_4(
    tracing_event_t tev,
    race_handle_t const & handle,
    connection_id_t const & connection_id,
    connection_status_t const & connection_status,
    link_id_t const &,
    LinkProperties const &) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_4_called"));

  if (!bootstrappee_step_3_done_) {
    return false;
  }

  if (bootstrappee_step_4_done_) {
    return false;
  }

  if (handle != bootstrappee_reply_connection_open_handle_) {
    return false;
  }

  if (connection_status != connection_status_t::open()) {
    // TODO: This might mean something went wrong. What can we do?
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_4_started"));

  bootstrappee_reply_connection_id_ = connection_id;

  bootstrappee_step_4_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_4_finished"));

  bootstrappee_step_7(SST_TEV_ARG(tev));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_5(tracing_event_t tev,
                                   channel_t const & channel) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_5_called"));

  if (!bootstrappee_step_1_done_) {
    return false;
  }

  if (bootstrappee_step_5_done_) {
    return false;
  }

  auto const it = bootstrappee_mb_channels_.find(&channel);

  if (it == bootstrappee_mb_channels_.end()) {
    return false;
  }

  if (it->second) {
    return false;
  }

  if (channel.status() == channel_status_t::enabled()
      || channel.status() == channel_status_t::starting()) {
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_5_started"));

  if (channel.status() == channel_status_t::available()) {
    for (phonebook_pair_t const * const & kv :
         config().local().bucket_mb_servers(SST_TEV_ARG(tev))) {
      psn_t const & psn = kv->first;
      auto const r = bootstrappee_cookies_.emplace(
          std::piecewise_construct,
          std::make_tuple(
              sdk().createLink(SST_TEV_ARG(tev),
                               channel.id().value(),
                               std::vector<std::string>{psn.value()},
                               0)),
          std::make_tuple(std::cref(channel), std::cref(*kv)));
      SST_ASSERT((r.second));
    }
    it->second = true;
  } else {
    // TODO: Print a warning?
    bootstrappee_mb_channels_.erase(it);
  }

  if (bootstrappee_cookies_.size() < bootstrappee_mb_channels_.size()) {
    return false;
  }

  bootstrappee_step_5_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_5_finished"));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_6(
    tracing_event_t tev,
    race_handle_t const & handle,
    link_id_t const & link_id,
    link_status_t const & link_status,
    LinkProperties const & link_properties) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_6_called"));

  // Steps 5 and 6 can progress at the same time.
  if (!bootstrappee_step_1_done_) {
    return false;
  }

  if (bootstrappee_step_6_done_) {
    return false;
  }

  auto const it = bootstrappee_cookies_.find(handle);

  if (it == bootstrappee_cookies_.end()) {
    return false;
  }

  if (it->second.cookie) {
    KESTREL_WARN(sdk(),
                 SST_TEV_ARG(tev,
                             "event",
                             "redundant_onLinkStatusChanged_call"));
    return false;
  }

  if (link_status != link_status_t::created()) {
    // TODO: This means something went wrong. What can we do?
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_6_started"));

  add_link(SST_TEV_ARG(tev),
           link_id,
           link_properties,
           std::unordered_set<psn_t>{it->second.mb_server->first});

  it->second.cookie.emplace(link_properties.linkAddress);

  ++bootstrappee_cookie_count_;

  if (bootstrappee_cookie_count_ < bootstrappee_cookies_.size()) {
    return false;
  }

  bootstrappee_step_6_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_6_finished"));

  bootstrappee_step_7(SST_TEV_ARG(tev));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

bool plugin_t::bootstrappee_step_7(tracing_event_t tev) {
  SST_TEV_TOP(tev);

  KESTREL_TRACE(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_7_called"));

  if (!bootstrappee_step_4_done_) {
    return false;
  }

  if (!bootstrappee_step_6_done_) {
    return false;
  }

  if (bootstrappee_step_7_done_) {
    return false;
  }

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_7_started"));

  std::shared_ptr<phonebook_entry_t const> const bootstrapper =
      config().phonebook().at(SST_TEV_ARG(tev),
                              config().bootstrap().bootstrapper());

  std::vector<unsigned char> x1;
  for (auto const & kv : bootstrappee_cookies_) {
    bootstrappee_cookie_t const & v = kv.second;
    std::shared_ptr<phonebook_entry_t const> const mb_server =
        config().phonebook().at(SST_TEV_ARG(tev), *v.mb_server);
    std::vector<unsigned char> const nonce = sst::crypto_rng(16);
    std::vector<unsigned char> const x2 =
        serialize({},
                  config().local().psn(),
                  config().local().pk(),
                  nonce,
                  config().global().epoch_nonce(),
                  v.channel->id().value(),
                  v.cookie->value());
    std::vector<unsigned char> const x3 =
        pkc::anon_encrypt(mb_server->pk(), x2).first;
    serialize(x1, mb_server->psn(), x3);
  }
  std::vector<unsigned char> const x4 =
      pkc::auth_encrypt(config().local().sk(), bootstrapper->pk(), x1)
          .first;
  std::vector<unsigned char> const package =
      serialize({}, config().local().psn(), config().local().pk(), x4);

  sdk().sendBootstrapPkg(SST_TEV_ARG(tev),
                         bootstrappee_reply_connection_id_.value(),
                         config().local().psn().value(),
                         package,
                         0);

  bootstrappee_step_7_done_ = true;

  KESTREL_INFO(
      sdk(),
      SST_TEV_ARG(tev, "event", "bootstrappee_step_7_finished"));

  return true;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
