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
#include <kestrel/kestrel_genesis.hpp>
// Include twice to test idempotence.
#include <kestrel/kestrel_genesis.hpp>
//

#include <algorithm>
#include <iostream>
#include <list>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/json/get_from_cli.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/mkdir_p_new.hpp>
#include <sst/catalog/mv.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/perfect_lt.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/to_integer.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unknown_oper.hpp>
#include <sst/catalog/unknown_opt.hpp>

#include <ChannelProperties.h>

#include <kestrel/carma/generate_configs.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/engine_config_t.hpp>
#include <kestrel/genesis_spec_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/kestrel_cli_args_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/rabbitmq/generate_configs.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/role_t.hpp>
#include <kestrel/slugify.hpp>
#include <kestrel/transmission_type_t.hpp>

namespace kestrel {

void kestrel_genesis(kestrel_cli_args_t const & kestrel_cli_args,
                     genesis_spec_t const & genesis_spec,
                     std::ostream & cout) {

  std::string const & work_dir = kestrel_cli_args.work_dir();

  sst::mkdir_p(work_dir);

  std::string const nodes_dir = work_dir + "/nodes";
  sst::rm_f_r(nodes_dir);
  sst::mkdir_p_new(nodes_dir);

  std::string const misc_dir = nodes_dir + "/%misc";
  sst::mkdir_p_new(misc_dir);

  std::string const channels_file = misc_dir + "/channels.json";
  {
    json_t j0 = json_t::array();
    ChannelProperties const p =
        rabbitmq::plugin_t::initial_properties();
    j0.push_back({
        {"channelGid", p.channelGid},
        {"connectionType", connection_type_t(p.connectionType)},
        {"description", p.channelGid},
        {"reliable", p.reliable},
        {"transmissionType", transmission_type_t(p.transmissionType)},
    });
    sst::json::get_as_file(j0, channels_file);
  }

  std::vector<psn_t> clients;

  std::string const range_file = misc_dir + "/range.json";
  {
    json_t j0 = json_t::object();
    json_t & j1 = j0["range"] = json_t::object();
    json_t & j2 = j1["RACE_nodes"] = json_t::array();
    for (decltype(+genesis_spec.num_clients()) i = 0;
         i < genesis_spec.num_clients();
         ++i) {
      clients.emplace_back("Armadillo" + sst::to_string(i));
      j2.push_back({
          {"genesis", true},
          {"name", clients.back().value()},
          {"type", "client"},
      });
    }
    for (decltype(+genesis_spec.num_servers()) i = 0;
         i < genesis_spec.num_servers();
         ++i) {
      j2.push_back({
          {"genesis", true},
          {"name", "WhiteWhale" + sst::to_string(i)},
          {"type", "server"},
      });
    }
    sst::json::get_as_file(j0, range_file);
  }

  std::list<std::string> argv1;
  std::list<std::string> argv2;

  argv1.emplace_back("generate_configs");
  argv2.emplace_back("generate_configs");

  argv1.emplace_back("--overwrite");
  argv2.emplace_back("--overwrite");

  argv1.emplace_back("--no-inter-server-direct-only");

  argv1.emplace_back("--range");
  argv1.emplace_back(range_file);

  argv2.emplace_back("--range");
  argv2.emplace_back(range_file);

  argv1.emplace_back("--channel-list");
  argv1.emplace_back(channels_file);

  std::string const dir1 = nodes_dir + "/%1";
  std::string const dir2 = nodes_dir + "/%2";

  argv1.emplace_back("--config-dir=" + dir1);
  argv2.emplace_back("--config-dir=" + dir2);

  argv2.emplace_back("--ta1-request");
  argv2.emplace_back(dir1 + "/network-manager-request.json");

  sst::mkdir_p_new(dir1);
  sst::mkdir_p_new(dir2);

  carma::generate_configs(argv1);

  rabbitmq::generate_configs(argv2);

  for (auto it = sst::dir_it(dir1); it != it.end(); ++it) {
    if (it->test_d()) {
      std::string const x = it->str();
      std::string const key = x.substr(it.prefix());
      if (key != "shared") {
        std::string const psn = unslugify(key);
        std::string const psn_slug = slugify(psn);
        std::string const carma_dir =
            nodes_dir + "/" + psn_slug + "/plugins/carma";
        std::string const y = carma_dir + "/fs";
        sst::mkdir_p_new(carma_dir);
        sst::mv(x, y);
      }
    }
  }

  {
    json_t const src = sst::json::get_from_file<json_t>(
                           dir2 + "/genesis-link-addresses.json")
                           .at("kestrel-rabbitmq");
    for (auto const & kv : src.items()) {
      json_t::string_t const & key = kv.key();
      json_t const & links = kv.value();
      std::string const psn = unslugify(key);
      std::string const psn_slug = slugify(psn);
      std::string const d = nodes_dir + "/" + psn_slug;
      engine_config_t engine_config;
      engine_config.set_clients(clients);
      engine_config.set_psn(psn_t(psn));
      {
        auto const p = engine_config.clients();
        engine_config.set_role(
            std::binary_search(p->begin(),
                               p->end(),
                               *engine_config.psn()) ?
                role_t::client() :
                role_t::server());
      }
      sst::json::get_as_file(json_t(engine_config), d + "/engine.json");
      sst::json::get_as_file(
          json_t{{"kestrel-rabbitmq", links}},
          d + "/plugins/carma/fs/link-profiles.json");
    }
  }

  // TODO: Copy "shared", if it exists, into every directory.

  sst::rm_f_r(misc_dir);
  sst::rm_f_r(dir1);
  sst::rm_f_r(dir2);

  //--------------------------------------------------------------------
}

void kestrel_genesis(kestrel_cli_args_t const & kestrel_cli_args,
                     std::list<std::string> argv,
                     std::ostream & cout) {

  if (argv.empty()) {
    argv.push_back("kestrel genesis");
  }

  std::string const argv0 = std::move(argv.front());

  sst::optional<std::string> genesis_spec_file;

  bool parse_options = true;
  while (argv.pop_front(), !argv.empty()) {
    if (parse_options) {

      //----------------------------------------------------------------
      // Options terminator
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--", sst::opt_arg::forbidden)) {
        parse_options = false;
        continue;
      }

      //----------------------------------------------------------------
      // Unknown options
      //----------------------------------------------------------------

      sst::unknown_opt(argv);

      //----------------------------------------------------------------
    }

    //------------------------------------------------------------------
    // Operands
    //------------------------------------------------------------------

    if (!genesis_spec_file) {
      genesis_spec_file.emplace(std::move(argv.front()));
    } else {
      sst::unknown_oper(argv);
    }

    //------------------------------------------------------------------
  }

  kestrel_genesis(kestrel_cli_args,
                  sst::json::get_from_cli<genesis_spec_t, json_t>(
                      *genesis_spec_file),
                  cout);
}

} // namespace kestrel
