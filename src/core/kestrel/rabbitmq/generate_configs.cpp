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
#include <kestrel/rabbitmq/generate_configs.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/generate_configs.hpp>
//

#include <list>
#include <stdexcept>
#include <string>

#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/main.hpp>
#include <sst/catalog/mkdir_p_new.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/unknown_opt.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/link_role_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/ta1_request_t.hpp>

namespace kestrel {
namespace rabbitmq {

void generate_configs(std::list<std::string> args) {

  std::string config_dir;
  bool have_config_dir = false;

  bool overwrite = false;

  std::string range_file;
  bool have_range_file = false;

  std::string ta1_request_file;
  bool have_ta1_request_file = false;

  if (!args.empty()) {
    bool parse_options = true;
    while (args.pop_front(), !args.empty()) {
      if (parse_options) {

        if (sst::parse_opt(args, "--", sst::opt_arg::forbidden)) {
          parse_options = false;
          continue;
        }

        if (sst::parse_opt(args, "--config-dir")) {
          config_dir = args.front();
          have_config_dir = true;
          continue;
        }

        if (sst::parse_opt(args, "--local", sst::opt_arg::forbidden)) {
          continue;
        }

        if (sst::parse_opt(args,
                           "--overwrite",
                           sst::opt_arg::forbidden)) {
          overwrite = true;
          continue;
        }

        if (sst::parse_opt(args, "--range")) {
          range_file = args.front();
          have_range_file = true;
          continue;
        }

        if (sst::parse_opt(args, "--ta1-request")) {
          ta1_request_file = args.front();
          have_ta1_request_file = true;
          continue;
        }

        sst::unknown_opt(args);
      }
      throw std::runtime_error("Operands are forbidden.");
    }
  }

  if (!have_config_dir) {
    throw std::runtime_error("The --config-dir option must be given.");
  }

  if (!have_range_file) {
    throw std::runtime_error("The --range option must be given.");
  }

  if (!have_ta1_request_file) {
    throw std::runtime_error("The --ta1-request option must be given.");
  }

  if (overwrite) {
    sst::rm_f_r(config_dir);
  }
  sst::mkdir_p_new(config_dir);

  ta1_request_t const request =
      sst::json::get_from_file<ta1_request_t, json_t>(ta1_request_file);

  {
    ta1_request_t fulfilled;
    for (auto const & kv1 : request.entries()) {
      psn_t const & sender = kv1.first;
      for (auto const & kv2 : kv1.second) {
        std::string const & channel = kv2.first;
        for (auto const & recipients : kv2.second) {
          if (recipients.size() == 1) {
            fulfilled.add(sender, channel, recipients);
          }
        }
      }
    }
    sst::json::get_as_file(json_t(fulfilled),
                           config_dir + "/fulfilled-network-manager-request.json");
  }

  {
    json_t genesis = json_t::object();
    json_t & gmap = genesis["kestrel-rabbitmq"];
    gmap = json_t::object();
    for (auto const & kv1 : request.entries()) {
      psn_t const & sender = kv1.first;
      for (auto const & kv2 : kv1.second) {
        std::string const & channel = kv2.first;
        (void)channel;
        for (auto const & recipients : kv2.second) {
          if (recipients.size() == 1) {
            psn_t const & recipient = *recipients.cbegin();
            link_addrinfo_t ai;

            // TODO: This is hardcoded for now. Eventually we should be
            //       passed in a list of servers to randomly pick from.
            if (sst::rand_range(0, 1)) {
              ai.host("rabbitmq.kestrel.stealthsoftwareinc.com");
            } else {
              ai.host("rabbitmq2.kestrel.stealthsoftwareinc.com");
            }

            gmap[recipient.string()].push_back({
                {"address", ai.to_link_address().string()},
                {"description", ""},
                {"personas", json_t::array({sender.string()})},
                {"role", ai.role()},
            });
            ai.role(link_role_t::loader());
            gmap[sender.string()].push_back({
                {"address", ai.to_link_address().string()},
                {"description", ""},
                {"personas", json_t::array({recipient.string()})},
                {"role", ai.role()},
            });
          }
        }
      }
    }
    sst::json::get_as_file(genesis,
                           config_dir + "/genesis-link-addresses.json");
  }
}

void generate_configs(int const argc, char ** const argv) {
  generate_configs(std::list<std::string>(argv, argv + argc));
}

} // namespace rabbitmq
} // namespace kestrel
