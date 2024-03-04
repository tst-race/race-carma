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
#include <kestrel/carma/generate_configs.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/generate_configs.hpp>
//

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <regex>
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
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/main.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/mkdir_p_new.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/perfect_lt.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/to_integer.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unknown_opt.hpp>
#include <sst/catalog/what.hpp>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/channel_list_entry_t.hpp>
#include <kestrel/config_compile.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_profile_t.hpp>
#include <kestrel/old_config_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/range_config_t.hpp>
#include <kestrel/stack_config_t.hpp>
#include <kestrel/ta1_request_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

namespace {

ta1_request_t create_request(
    config_t const & unified,
    std::map<std::string, old_config_t> const & all_node_configs,
    std::map<psn_t, config_t> const & configs2,
    channel_list_t const & channel_list,
    std::unordered_map<psn_t, std::unordered_set<psn_t>> const &
        tx_nodes,
    ta1_request_t const * const filter = nullptr) {
  (void)configs2;
  tracing_event_t SST_TEV_DEF(tev);
  ta1_request_t request;
  for (auto const & all_node_configs_kv : all_node_configs) {
    auto const & config = all_node_configs_kv.second;
    auto const & x = psn_t(all_node_configs_kv.first);
    std::shared_ptr<phonebook_entry_t const> const my_entry_ptr =
        unified.phonebook().at(SST_TEV_ARG(tev), x);
    phonebook_entry_t const & my_entry = *my_entry_ptr;
    auto direct = {connection_type_t::direct()};
    auto indirect = {connection_type_t::indirect()};
    for (psn_t const & y : tx_nodes.at(x)) {
      bool const b =
          my_entry.role() == role_t::client()
          || unified.phonebook().at(SST_TEV_ARG(tev), y)->role()
                 == role_t::client();
      if (b || !config.inter_server_direct_only) {
        request.add(x, y, channel_list, indirect, filter);
      }
      if (!b) {
        request.add(x, y, channel_list, direct, filter);
      }
    }
  }
  return request;
}

// TODO: This would probably be used by generate_configs for all TA1
//       plugins. It could be defined once, in the kestrel namespace.
void write_status_file(std::string const & config_dir,
                       std::string const & status,
                       std::string const & reason) {
  nlohmann::json json{
      {"attempt", 1},
      {"reason", reason},
      {"status", status},
  };
  sst::json::get_as_file(json,
                         config_dir + "/network-manager-config-gen-status.json");
}

} // namespace

void generate_configs(std::list<std::string> args) {
  std::string exception_config_dir;
  try {
    tracing_event_t SST_TEV_DEF(tev);

    if (args.empty()) {
      throw std::runtime_error("argc == 0");
    }

    //------------------------------------------------------------------
    // RACE options data
    //------------------------------------------------------------------

    // --channel-list
    std::string channel_list_file;
    bool have_channel_list_file = false;

    // --config-dir
    std::string config_dir = "configs";

    // --fulfilled-requests
    std::string fulfilled_requests_file;
    bool have_fulfilled_requests_file = false;

    // --local
    bool have_local = false;
    (void)have_local;

    // --odd-factor
    int odd_factor = 101;

    // --overwrite
    bool overwrite = false;

    // --range
    std::string range_file;
    bool have_range_file = false;

    // --two-exp
    int two_exp = 3767;

    //------------------------------------------------------------------
    // CARMA options data
    //------------------------------------------------------------------

    bool leader_relay_only = true;
    bool inter_server_direct_only = true;
    auto num_clients = stack_config_t::default_num_clients();
    auto num_servers = stack_config_t::default_num_servers();
    std::vector<std::regex> dynamic_only_regexes;

    //------------------------------------------------------------------
    // Stack JSON
    //------------------------------------------------------------------
    //
    // Some command-line options will simply modify this JSON object,
    // deferring some of the parsing work to stack_config_t's from_json
    // function.
    //

    nlohmann::json stack_json = {
        /*
        {"prime",
         "0x32800000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000000000000000000000000000000000000"
         "0000000000000000000000000000001"},
*/
        {"mix_size", 4},
        {"secret_key_length", 128},
        {"threshold", 2},
        {"leader_relay_only", leader_relay_only},
        {"log_level", 0},
        {"inter_server_direct_only", inter_server_direct_only},
    };

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
      num_threads = 1;
    }

    //------------------------------------------------------------------

    bool parse_options = true;
    while (args.pop_front(), !args.empty()) {
      if (parse_options) {

        bool has_arg;

        //--------------------------------------------------------------
        // Options terminator
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--", sst::opt_arg::forbidden)) {
          parse_options = false;
          continue;
        }

        //--------------------------------------------------------------
        // --[no-]allow-multicast
        //--------------------------------------------------------------

        if (sst::parse_opt(args,
                           "--allow-multicast",
                           sst::opt_arg::forbidden)) {
          stack_json["allow_multicast"] = true;
          continue;
        }

        if (sst::parse_opt(args,
                           "--no-allow-multicast",
                           sst::opt_arg::forbidden)) {
          stack_json["allow_multicast"] = false;
          continue;
        }

        //--------------------------------------------------------------
        // --channel-list
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--channel-list")) {
          channel_list_file = std::move(args.front());
          have_channel_list_file = true;
          continue;
        }

        //--------------------------------------------------------------
        // --config-dir
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--config-dir")) {
          config_dir = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --fulfilled-requests
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--fulfilled-requests")) {
          fulfilled_requests_file = std::move(args.front());
          have_fulfilled_requests_file = true;
          continue;
        }

        //--------------------------------------------------------------
        // --[no-]leader-relay-only
        //--------------------------------------------------------------

        if (sst::parse_opt(args,
                           "--leader-relay-only",
                           sst::opt_arg::forbidden)) {
          leader_relay_only = true;
          stack_json["leader_relay_only"] = true;
          continue;
        }

        if (sst::parse_opt(args,
                           "--no-leader-relay-only",
                           sst::opt_arg::forbidden)) {
          leader_relay_only = false;
          stack_json["leader_relay_only"] = false;
          continue;
        }

        //--------------------------------------------------------------
        // --local
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--local", sst::opt_arg::forbidden)) {
          have_local = true;
          continue;
        }

        //--------------------------------------------------------------
        // --mix-size
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--mix-size")) {
          stack_json["mix_size"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --[no-]inter-server-direct-only
        //--------------------------------------------------------------

        if (sst::parse_opt(args,
                           "--inter-server-direct-only",
                           sst::opt_arg::forbidden)) {
          inter_server_direct_only = true;
          stack_json["inter_server_direct_only"] = true;
          continue;
        }

        if (sst::parse_opt(args,
                           "--no-inter-server-direct-only",
                           sst::opt_arg::forbidden)) {
          inter_server_direct_only = false;
          stack_json["inter_server_direct_only"] = false;
          continue;
        }

        //--------------------------------------------------------------
        // --num-clients
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--num-clients")) {
          sst::json::get_to(nlohmann::json(args.front()), num_clients);
          if (sst::perfect_lt(num_clients, 1)) {
            throw std::runtime_error("--num-clients must be positive: "
                                     + sst::to_string(num_clients));
          }
          stack_json["num_clients"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --num-mc-groups
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--num-mc-groups")) {
          stack_json["num_mc_groups"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --num-servers
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--num-servers")) {
          sst::json::get_to(nlohmann::json(args.front()), num_servers);
          if (sst::perfect_lt(num_servers, 1)) {
            throw std::runtime_error("--num-servers must be positive: "
                                     + sst::to_string(num_servers));
          }
          stack_json["num_servers"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --num-threads
        //--------------------------------------------------------------
        //
        // A suggested number of threads to use for CPU intensive parts
        // of config generation.
        //

        if (sst::parse_opt(args, "--num-threads")) {
          num_threads =
              sst::to_integer<decltype(num_threads)>(args.front());
          if (num_threads < 1) {
            throw std::runtime_error("--num-threads must be positive: "
                                     + sst::to_string(num_threads));
          }
          continue;
        }

        //--------------------------------------------------------------
        // --odd-factor
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--odd-factor")) {
          sst::to_integer<decltype(odd_factor)>(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --overwrite
        //--------------------------------------------------------------

        if (sst::parse_opt(args,
                           "--overwrite",
                           sst::opt_arg::forbidden)) {
          overwrite = true;
          continue;
        }

        //--------------------------------------------------------------
        // --range
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--range")) {
          range_file = std::move(args.front());
          have_range_file = true;
          continue;
        }

        //--------------------------------------------------------------
        // --send-retry-count
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--send-retry-count")) {
          stack_json["send_retry_count"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --send-retry-window
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--send-retry-window")) {
          stack_json["send_retry_window"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --send-timeout
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--send-timeout")) {
          stack_json["send_timeout"] = std::move(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --two-exp
        //--------------------------------------------------------------

        if (sst::parse_opt(args, "--two-exp")) {
          sst::to_integer<decltype(two_exp)>(args.front());
          continue;
        }

        //--------------------------------------------------------------
        // --dynamic-only
        //--------------------------------------------------------------

        if (sst::parse_opt(args,
                           "--dynamic-only",
                           sst::opt_arg::permitted,
                           &has_arg)) {
          if (has_arg) {
            dynamic_only_regexes.emplace_back(args.front());
          } else {
            dynamic_only_regexes.emplace_back(".*");
          }
          continue;
        }

        //--------------------------------------------------------------
        // Unknown options
        //--------------------------------------------------------------

        sst::unknown_opt(args);

        //--------------------------------------------------------------
      }

      throw std::runtime_error("operands are forbidden: "
                               + args.front());
    }

    exception_config_dir = config_dir;

    if (!have_range_file) {
      throw std::runtime_error("missing option: --range");
    }

    bool const mock = range_file == "mock";

    if (!mock && !have_channel_list_file) {
      throw std::runtime_error("missing option: --channel-list");
    }

    channel_list_t const channel_list =
        mock ? sst::json::get_as<channel_list_t>(nlohmann::json{{
            {"channelGid", "carmaMock"},
            {"connectionType", "CT_INDIRECT"},
            {"description", "carmaMock"},
            {"reliable", false},
            {"transmissionType", "TT_MULTICAST"},
        }}) :
               sst::json::get_from_file<channel_list_t, nlohmann::json>(
                   channel_list_file);

    range_config_t const range_config =
        mock ?
            [&]() {
              range_config_t x;
              for (decltype(+num_clients) i = 0; i != num_clients;
                   ++i) {
                x.clients.emplace("Armadillo" + sst::to_string(i));
              }
              for (decltype(+num_servers) i = 0; i != num_servers;
                   ++i) {
                x.servers.emplace("WhiteWhale" + sst::to_string(i));
              }
              return x;
            }() :
            sst::json::get_from_file<range_config_t, nlohmann::json>(
                range_file);

    std::set<psn_t> const all_personas = [&] {
      std::set<psn_t> xs;
      for (auto const & x : range_config.clients) {
        xs.emplace(x);
      }
      for (auto const & x : range_config.servers) {
        xs.emplace(x);
      }
      return xs;
    }();

    stack_json["prime"] =
        sst::to_string((sst::bigint(1) << two_exp) * odd_factor + 1);

    if (!mock) {
      stack_json["num_clients"] =
          sst::to_string(range_config.clients.size());
      stack_json["num_servers"] =
          sst::to_string(range_config.servers.size());
    }

    auto const stack_config =
        sst::json::get_as<stack_config_t>(stack_json);

    std::string retrying_reason;

    if (have_fulfilled_requests_file) {

      write_status_file(config_dir, "complete", "success");

      return;
    }

    if (overwrite) {
      sst::rm_f_r(config_dir);
    }
    sst::mkdir_p_new(config_dir);

    sst::json::get_as_file(nlohmann::json(channel_list),
                           config_dir + "/channel_list.json");

    std::set<std::string> const dynamic_only_nodes = [&]() {
      std::set<std::string> dynamic_only_nodes;
      for (auto const nodes :
           {&range_config.clients, &range_config.servers}) {
        for (auto const & node : *nodes) {
          for (auto const & regex : dynamic_only_regexes) {
            if (std::regex_search(node.value(), regex)) {
              dynamic_only_nodes.emplace(node.value());
            }
          }
        }
      }
      return dynamic_only_nodes;
    }();

    int const max_genesis_attempts = 100;
    int genesis_attempt = 0;

retry_genesis:

    ++genesis_attempt;
    if (genesis_attempt > max_genesis_attempts) {
      throw std::runtime_error("All genesis attempts were rejected");
    }

    // This is the union of all nodes.
    config_t unified(SST_TEV_ARG(tev), config_dir + "/nonexistent");

    std::map<psn_t, carma::config_t> configs2;

    std::map<std::string, nlohmann::json> all_node_configs_json;
    std::unordered_map<psn_t, std::unordered_set<psn_t>> tx_nodes;

    try {
      all_node_configs_json = config_compile("stack.json",
                                             unified,
                                             range_file,
                                             stack_config,
                                             range_config,
                                             dynamic_only_nodes,
                                             configs2,
                                             config_dir,
                                             num_threads,
                                             tx_nodes);
    } catch (genesis_rejection const &) {
      goto retry_genesis;
    }

    // At this point, the configs have been fully written to disk, but
    // for memory reasons, each configs2[x].phonebook() will have been
    // destroyed. To retrieve a local config, use configs2[x].local().
    // To retrieve a phonebook entry, use unified.phonebook().at(x).
    // Using configs2[x].phonebook() will cause assertion failures.

    std::map<std::string, old_config_t> const all_node_configs = [&]() {
      std::map<std::string, old_config_t> all_node_configs;
      for (auto const & x : all_node_configs_json) {
        all_node_configs.insert(
            {x.second.at("pseudonym"),
             sst::json::get_as<old_config_t>(x.second)});
      }
      return all_node_configs;
    }();

    for (auto const & x : all_node_configs_json) {
      auto const persona =
          sst::json::get_as<psn_t>(x.second.at("pseudonym"));
      auto const slug = persona.to_path_slug();
      auto const d1 = config_dir + "/" + slug;
      sst::mkdir_p(d1);
      sst::json::get_as_file(x.second, d1 + "/config.json");
    }

    ta1_request_t const request = create_request(unified,
                                                 all_node_configs,
                                                 configs2,
                                                 channel_list,
                                                 tx_nodes);

    sst::json::get_as_file(nlohmann::json(request),
                           config_dir + "/network-manager-request.json");

    write_status_file(config_dir, "starting", "");
  } catch (...) {
    try {
      if (!exception_config_dir.empty()) {
        write_status_file(exception_config_dir,
                          "complete",
                          std::string("error: ") + sst::what());
      }
    } catch (...) {
    }
    throw;
  }
}

void generate_configs(int const argc, char ** const argv) {
  generate_configs(std::list<std::string>(argv, argv + argc));
}

} // namespace carma
} // namespace kestrel
