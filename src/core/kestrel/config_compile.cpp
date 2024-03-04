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
#include <kestrel/config_compile.hpp>
// Include twice to test idempotence.
#include <kestrel/config_compile.hpp>
//

#include <algorithm>
#include <fstream>
#include <future>
#include <iterator>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/perfect_gt.hpp>
#include <sst/catalog/perfect_lt.hpp>
#include <sst/catalog/promote_unsigned_t.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/type_max.hpp>
#include <sst/catalog/unique_ptr.hpp>
#include <sst/catalog/unsigned_min.hpp>
#include <sst/checked.h>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/carma/vrf.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/range_config_t.hpp>
#include <kestrel/stack_config_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/vrf_algorand_t.hpp>
#include <kestrel/vrf_core_t.hpp>
#include <kestrel/vrf_shell_t.hpp>

// TODO: This should go away once we're inside the kestrel::carma
//       namespace too.
using namespace kestrel::carma;

namespace kestrel {

namespace {

void do_common(json_t & config,
               std::string const & persona,
               std::set<std::string> const & dynamic_only_nodes) {

  config["pseudonym"] = persona;

  config["dynamic_only"] =
      dynamic_only_nodes.find(persona) != dynamic_only_nodes.end();
}

static std::map<std::string, json_t> config_compile(
    std::string const &,
    config_t & unified,
    stack_config_t const & config,
    std::unordered_set<psn_t> const & all_clients,
    std::unordered_set<psn_t> const & all_servers,
    std::set<std::string> const & dynamic_only_nodes,
    std::map<psn_t, carma::config_t> & configs2,
    std::string const & dir,
    unsigned int const num_threads,
    std::unordered_map<psn_t, std::unordered_set<psn_t>> & tx_nodes) {

  tracing_event_t SST_TEV_DEF(tev);

  // TODO: Remove this variable and write "unified.phonebook()"
  //       everywhere instead. It's more clear.
  phonebook_t & phonebook = unified.phonebook();

  json_t dst = json_t::object();
  using json_int_t = decltype(dst)::number_integer_t;

  sst::bigint const & prime = config.prime;

  std::vector<unsigned char> epoch;
  epoch.resize(16);
  sst::crypto_rng(epoch.data(), epoch.size());

  global_config_t & global = unified.global();

  global.set_shamir_threshold(5);
  global.correctness_test_only(true);
  global.corruption_rate(-1);
  global.epoch_nonce(epoch);
  global.num_clients(sst::checked_cast<decltype(global.num_clients())>(
      config.num_clients()));
  global.set_num_servers(
      sst::checked_cast<node_count_t>(config.num_servers()));
  global.prime(prime);
  global.privacy_failure_rate(-1);
  global.robustness_failure_rate(-1);
  global.vrf(
      vrf_shell_t(sst::unique_ptr<vrf_core_t>(new vrf_algorand_t)));
  global.validate();

  //--------------------------------------------------------------------

  {

    std::vector<unsigned char> vrf_pk = global.vrf().pk_buffer();
    std::vector<unsigned char> vrf_sk = global.vrf().sk_buffer();

    auto const init = [&](psn_t const & psn, bool const server) {
      phonebook.add_fast(SST_TEV_ARG(tev), phonebook_entry_t(psn));
      carma::config_t & config2 =
          configs2
              .emplace(
                  std::piecewise_construct,
                  std::forward_as_tuple(psn),
                  std::forward_as_tuple(SST_TEV_ARG(tev),
                                        dir + "/" + psn.to_path_slug()))
              .first->second;
      config2.global() = global;
      local_config_t & local = config2.local();
      local.set_psn(psn);
      auto const pksk = pkc::generate_keypair();
      local.pk(pksk.first);
      local.sk(pksk.second);
      if (server) {
        global.vrf().keygen(vrf_pk, vrf_sk);
        local.set_vrf_pk(vrf_pk);
        local.set_vrf_sk(vrf_sk);
        (void)local.ticket();
      } else {
        local.set_role(role_t::client());
      }
    };

    for (psn_t const & psn : all_clients) {
      init(psn, false);
    }
    for (psn_t const & psn : all_servers) {
      init(psn, true);
    }

  } //

  //--------------------------------------------------------------------

  std::map<std::string, std::set<std::string>> mc_groups;
  std::map<std::string, std::set<std::string>> mb_groups;
  std::set<psn_t> all_mc_members;
  std::set<psn_t> all_mc_leaders;
  std::set<psn_t> all_mc_servers;
  std::set<psn_t> all_mb_servers;
  std::set<psn_t> all_idle_servers;

  using group_t = decltype(+std::declval<local_config_t>().group());
  std::map<group_t, std::vector<local_config_t *>> all_mc_groups;

  if (global.is_rigid()) {
    std::vector<local_config_t *> ys;
    for (psn_t const & psn : all_servers) {
      ys.push_back(&configs2.at(psn).local());
    }
    std::sort(ys.begin(),
              ys.end(),
              [](local_config_t * const a, local_config_t * const b) {
                return a->ticket().output() < b->ticket().output();
              });
    auto const mc_size = sst::to_unsigned(global.mc_size());
    auto const num_idle = sst::to_unsigned(global.num_idle());
    auto const num_mbs = sst::to_unsigned(global.num_mbs());
    auto const num_servers = sst::to_unsigned(global.num_servers());
    for (decltype(ys.size()) i = 0; i < ys.size(); ++i) {
      local_config_t & local = *ys[i];
      if (i < num_mbs) {
        local.set_role(role_t::mb_server());
        all_mb_servers.emplace(local.psn());
      } else if (i < num_servers - num_idle) {
        group_t const q =
            sst::checked_cast<group_t>((i - num_mbs) / mc_size);
        long long const r =
            sst::checked_cast<long long>((i - num_mbs) % mc_size);
        all_mc_members.emplace(local.psn());
        all_mc_groups[q].emplace_back(&local);
        if (r == 0U) {
          local.set_role(role_t::mc_leader());
          all_mc_leaders.emplace(local.psn());
        } else {
          local.set_role(role_t::mc_follower());
          all_mc_servers.emplace(local.psn());
        }
        local.set_group(q);
        local.set_order(r);
      } else {
        local.set_role(role_t::idle_server());
        all_idle_servers.emplace(local.psn());
      }
    }
  } else {
    throw std::runtime_error(
        "Non-rigid deployments are not supported yet");
  }

  for (auto i = all_mc_groups.begin(); i != all_mc_groups.end();) {
    std::vector<local_config_t *> & group = i->second;
    if (sst::perfect_gt(group.size(), global.shamir_threshold())) {
      if (sst::perfect_gt(group.size(), global.mc_size())) {
        for (auto j = group.begin(); j != group.end();) {
          if (sst::perfect_lt((*j)->order(), global.mc_size())) {
            ++j;
          } else {
            all_mc_members.erase((*j)->psn());
            all_mc_leaders.erase((*j)->psn());
            all_mc_servers.erase((*j)->psn());
            (*j)->set_role(role_t::idle_server());
            all_idle_servers.emplace((*j)->psn());
            j = group.erase(j);
          }
        }
      }
      ++i;
    } else {
      for (local_config_t * const j : group) {
        all_mc_members.erase(j->psn());
        all_mc_leaders.erase(j->psn());
        all_mc_servers.erase(j->psn());
        j->set_role(role_t::idle_server());
        all_idle_servers.emplace(j->psn());
      }
      i = all_mc_groups.erase(i);
    }
  }

  if (all_mc_groups.empty()) {
    throw genesis_rejection();
  }

  if (sst::perfect_lt(all_mc_groups.size(),
                      global.min_good_mc_size())) {
    throw genesis_rejection();
  }

  dst["Prime"] = config.prime.to_string();
  dst["MixSize"] = sst::checked_cast<json_int_t>(config.mix_size);
  dst["Threshold"] = sst::checked_cast<json_int_t>(config.threshold);
  dst["LogLevel"] = sst::checked_cast<json_int_t>(config.log_level);

  dst["allow_multicast"] = config.allow_multicast();
  dst["leader_relay_only"] = config.leader_relay_only;
  dst["inter_server_direct_only"] = config.inter_server_direct_only;
  dst["send_retry_count"] = config.send_retry_count().to_string();
  dst["send_retry_window"] = config.send_retry_window().to_string();
  dst["send_timeout"] = config.send_timeout().to_string();

  //--------------------------------------------------------------------

  {

    for (auto const & psn_pbe : phonebook) {
      psn_t const & psn = psn_pbe.first;

      config_t & config2 = configs2.at(psn);
      local_config_t & local = config2.local();

      // TODO: In the old design, MB servers send stuff to all members
      //       of a committee. In the new design, they only send to the
      //       MC leader. So this should be changed from all_mc_members
      //       to all_mc_leaders.

      // TODO: In the old design, MC servers send stuff to all members
      //       of their committee (I think?). In the new design, they
      //       only send to the MC leader.

      // TODO: In the old design, each MB server was associated with
      //       exactly one MC group. What happens in the new design?

      if (global.is_rigid()) {
      } else {
        throw std::runtime_error(
            "Non-rigid deployments are not supported yet");
      }

      local.set_phonebook(phonebook);
      const_cast<phonebook_entry_t &>(
          *phonebook.at(SST_TEV_ARG(tev), psn)) = local;

    } //

  } //

  //--------------------------------------------------------------------

  for (auto const & psn_pbe : phonebook) {
    std::shared_ptr<phonebook_entry_t const> const pbe =
        phonebook.at(SST_TEV_ARG(tev), psn_pbe);
    if (pbe->role() == role_t::client()) {
      if (pbe->bucket_mb_servers(SST_TEV_ARG(tev)).empty()) {
        throw genesis_rejection();
      }
    }
  }

  //--------------------------------------------------------------------
  // Idle server connections
  //--------------------------------------------------------------------

  for (psn_t const & x : all_servers) {
    (void)configs2.at(x).local().mutable_other_servers();
  }
  for (psn_t const & x : all_idle_servers) {
    phonebook_set_t & xs =
        configs2.at(x).local().mutable_other_servers();
    for (psn_t const & y : all_servers) {
      if (x != y) {
        if (sst::rand_range(1U, sst::to_unsigned(all_servers.size()))
            <= 7U) {
          phonebook_set_t & ys =
              configs2.at(y).local().mutable_other_servers();
          xs.emplace(&unified.phonebook().expect(y));
          ys.emplace(&unified.phonebook().expect(x));
        }
      }
    }
  }

  //--------------------------------------------------------------------

  {
    std::vector<std::pair<psn_t const *, carma::config_t *>> cfgs;
    for (auto & kv : configs2) {
      cfgs.emplace_back(&kv.first, &kv.second);
      (void)tx_nodes[kv.first];
    }
    using idx = sst::promote_unsigned_t<decltype(cfgs.size())>;
    std::queue<std::pair<idx, idx>> tasks;
    {
      idx const n = cfgs.size();
      for (idx i = 0; i < n;) {
        idx const k = sst::unsigned_min(n - i, 10);
        tasks.emplace(i, i + k);
        i += k;
      }
    }
    std::mutex mutex;
    std::vector<std::future<void>> futures;
    for (decltype(+num_threads) t = 0; t < num_threads; ++t) {
      futures.emplace_back(std::async(std::launch::async, [&] {
        while (true) {
          std::pair<idx, idx> task;
          {
            std::lock_guard<std::mutex> const lock(mutex);
            if (tasks.empty()) {
              return;
            }
            task = tasks.front();
            tasks.pop();
          }
          for (idx i = task.first; i < task.second; ++i) {

            config_t & config2 = *cfgs[i].second;

            std::unordered_set<psn_t> & tx =
                tx_nodes.at(config2.local().psn());
            for (phonebook_pair_t const * const & pair :
                 config2.local().tx_nodes(SST_TEV_ARG(tev))) {
              tx.emplace(pair->first);
            }

            config2.global().clear_deducible();
            config2.local().clear_deducible();

            phonebook_t & pb = config2.phonebook();
            for (auto const & psn_pbe : phonebook) {
              pb.add_fast(SST_TEV_ARG(tev),
                          *phonebook.at(SST_TEV_ARG(tev), psn_pbe))
                  ->clear_deducible();
            }

            // TODO: This is where any redactions should be performed on
            //       this phonebook (pb).

            config2.flush(SST_TEV_ARG(tev), true);
            pb.clear();
          }
        }
      }));
    }
    for (auto & future : futures) {
      future.get();
    }
  }

  std::map<std::string, json_t> node_configs;

  for (psn_t const & psn : all_clients) {
    json_t config = dst;
    do_common(config, psn.value(), dynamic_only_nodes);
    node_configs[psn.value()] = std::move(config);
  }
  for (psn_t const & psn : all_servers) {
    json_t config = dst;
    do_common(config, psn.value(), dynamic_only_nodes);
    node_configs[psn.value()] = std::move(config);
  }

  return node_configs;
}

}; // namespace

std::map<std::string, json_t> config_compile(
    std::string const & stack_config_file,
    config_t & unified,
    std::string const &,
    stack_config_t const & stack_config,
    range_config_t const & range_config,
    std::set<std::string> const & dynamic_only_nodes,
    std::map<psn_t, carma::config_t> & configs2,
    std::string const & dir,
    unsigned int const num_threads,
    std::unordered_map<psn_t, std::unordered_set<psn_t>> & tx_nodes) {
  return config_compile(stack_config_file + " (range adjusted)",
                        unified,
                        stack_config,
                        range_config.clients,
                        range_config.servers,
                        dynamic_only_nodes,
                        configs2,
                        dir,
                        num_threads,
                        tx_nodes);
}

} // namespace kestrel
