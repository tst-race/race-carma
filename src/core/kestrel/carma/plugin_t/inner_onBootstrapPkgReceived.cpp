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
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/unsigned_gt.hpp>

#include <EncPkg.h>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/mailbox_message_type_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/logging.hpp>
#include <kestrel/packet_type_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_onBootstrapPkgReceived(
    tracing_event_t tev,
    psn_t const & bootstrappee_psn_1,
    RawData const & package) {
  SST_TEV_TOP(tev);

  process_message_context_t pmc(sdk_, *this);

  bytes_t::size_type package_i = 0;

  psn_t bootstrappee_psn_2;
  bytes_t bootstrappee_pk;
  bytes_t x4;
  deserialize(package,
              package_i,
              bootstrappee_psn_2,
              bootstrappee_pk,
              x4);

  if (bootstrappee_psn_2 != bootstrappee_psn_1) {
    throw std::runtime_error("Corrupt package");
  }

  if (phonebook().find(bootstrappee_psn_1)) {
    return;
  }

  phonebook_entry_t bootstrappee;
  bootstrappee.set_phonebook(phonebook());
  bootstrappee.set_psn(bootstrappee_psn_1);
  bootstrappee.pk(bootstrappee_pk);
  bootstrappee.set_role(role_t::client());

  std::vector<std::shared_ptr<phonebook_entry_t const>>
      recver_mb_servers;

  bytes_t const a = bootstrappee.auth_decrypt(x4);
  {
    bytes_t::size_type a_i = 0;
    bytes_t r;
    while (a_i < a.size()) {
      // TODO: recver_mb_server_psn can be changed to
      //       recver_mb_server_hash (psn_any_hash_t).
      psn_t recver_mb_server_psn;
      deserialize(a, a_i, recver_mb_server_psn, r);
      phonebook_pair_t const * const recver_mb_server_pair =
          phonebook().find(recver_mb_server_psn);
      if (!recver_mb_server_pair) {
        KESTREL_WARN(sdk(),
                     SST_TEV_ARG(tev, "event", "corrupt_bootstrappee"));
        return;
      }
      if (!contains(bootstrappee.bucket_mb_servers(SST_TEV_ARG(tev)),
                    *recver_mb_server_pair)) {
        KESTREL_WARN(sdk(),
                     SST_TEV_ARG(tev, "event", "corrupt_bootstrappee"));
        return;
      }
      recver_mb_servers.emplace_back(
          phonebook().at(SST_TEV_ARG(tev), *recver_mb_server_pair));
    }
  }

  std::vector<phonebook_pair_t const *> sender_mb_server_pairs;
  for (auto const & sender_mb_server_pair :
       local().bucket_mb_servers(SST_TEV_ARG(tev))) {
    sender_mb_server_pairs.emplace_back(sender_mb_server_pair);
  }
  std::shuffle(sender_mb_server_pairs.begin(),
               sender_mb_server_pairs.end(),
               sst::crypto_rng());
  if (sst::unsigned_gt(sender_mb_server_pairs.size(),
                       global().num_parallel_msgs_for_registration())) {
    sender_mb_server_pairs.resize(
        static_cast<decltype(sender_mb_server_pairs.size())>(
            global().num_parallel_msgs_for_registration()));
  }
  std::vector<std::shared_ptr<phonebook_entry_t const>>
      sender_mb_servers;
  for (auto const & sender_mb_server_pair : sender_mb_server_pairs) {
    sender_mb_servers.emplace_back(
        phonebook().at(SST_TEV_ARG(tev), *sender_mb_server_pair));
  }

  // TODO: Send the MC group number instead of the MC leader PSN hash.

  for (auto const & sender_mb_server : sender_mb_servers) {

    // Pick a random MC leader.
    phonebook_set_t const & mc_leaders =
        sender_mb_server->mc_leaders(SST_TEV_ARG(tev));
    auto mc_leader_it = mc_leaders.begin();
    using sz = decltype(mc_leaders.size());
    std::advance(mc_leader_it,
                 sst::rand_range(sz(0), mc_leaders.size() - sz(1)));
    std::shared_ptr<phonebook_entry_t const> const mc_leader =
        phonebook().at(SST_TEV_ARG(tev), **mc_leader_it);

    bytes_t b;
    for (auto const & recver_mb_server : recver_mb_servers) {
      serialize(b,
                recver_mb_server->anon_encrypt(
                    serialize({},
                              mailbox_message_type_t::new_client(),
                              bootstrappee.psn())));
    }

    bytes_t c;
    for (auto const & recver_mb_server : recver_mb_servers) {
      serialize(c,
                mc_leader->anon_encrypt(
                    serialize({}, recver_mb_server->psn_hash())));
    }

    bytes_t const packet = sender_mb_server->auth_encrypt(
        serialize({},
                  packet_type_t::endorsement_request().value(),
                  mc_leader->psn_hash(),
                  a,
                  b,
                  c));

    pmc.send_message(SST_TEV_ARG(tev),
                     serialize({}, local().psn(), packet),
                     sender_mb_server->psn());
  }

  phonebook().add_slow(SST_TEV_ARG(tev), std::move(bootstrappee));

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
