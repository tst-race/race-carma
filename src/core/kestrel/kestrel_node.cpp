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
#include <kestrel/kestrel_node.hpp>
// Include twice to test idempotence.
#include <kestrel/kestrel_node.hpp>
//

#include <algorithm>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/perfect_eq.hpp>
#include <sst/catalog/precise_sleep_until_unix_time_ns.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/unix_time_ns.hpp>
#include <sst/catalog/unknown_oper.hpp>
#include <sst/catalog/unknown_opt.hpp>

#include <kestrel/engine_t.hpp>
#include <kestrel/kestrel_cli_args_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/replay_message_spec_t.hpp>
#include <kestrel/replay_spec_t.hpp>
#include <kestrel/slugify.hpp>

namespace kestrel {

void kestrel_node(kestrel_cli_args_t const & kestrel_cli_args,
                  std::list<std::string> argv,
                  std::ostream & cout) {

  if (argv.empty()) {
    argv.push_back("kestrel node");
  }

  std::string const argv0 = std::move(argv.front());

  sst::optional<replay_spec_t> replay;

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
      // --replay
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--replay", sst::opt_arg::required)) {
        // TODO: Use sst::json::get_from_file_or_inline
        replay = sst::json::get_from_file<replay_spec_t, json_t>(
            argv.front());
        continue;
      }

      //----------------------------------------------------------------
      // Unknown options
      //----------------------------------------------------------------

      sst::unknown_opt(argv);

      //----------------------------------------------------------------
    }

    //------------------------------------------------------------------
    // Unknown operands
    //------------------------------------------------------------------

    sst::unknown_oper(argv);

    //------------------------------------------------------------------
  }

  engine_t engine(kestrel_cli_args.work_dir());

  std::promise<void> on_start_promise;
  std::future<void> on_start_future(on_start_promise.get_future());

  engine.on_start([&](engine_t::on_start_info_t const &) {
    on_start_promise.set_value();
  });

  engine.on_stop([&](char const * const error) {
    if (error) {
      std::cerr << "Engine stopped: Error: " << error << "\n";
    } else {
      std::cout << "Engine stopped.\n";
    }
  });

  engine.on_recv_clrmsg(
      [&](engine_t::on_recv_clrmsg_info_t const & info) {
        std::cout << info.psn << ": " << info.msg << "\n";
      });

  engine.start();

  on_start_future.wait();

  if (replay) {
    sst::unix_time_ns_t const dawn = sst::is_positive(replay->dawn()) ?
                                         replay->dawn() :
                                         sst::unix_time_ns();
    std::multiset<replay_message_spec_t> & messages =
        replay->messages();
    while (!messages.empty()) {
      auto const it = messages.begin();
      replay_message_spec_t const & message = *it;
      std::shared_ptr<psn_t const> const my_psn_p =
          engine.config().psn();
      psn_t const & my_psn = *my_psn_p;
      if (message.sender_slug() != "*" && message.sender() != my_psn) {
        messages.erase(it);
        continue;
      }
      if (message.time().b()) {
        replay_message_spec_t m = message;
        m.time() = m.time().eval();
        messages.emplace(std::move(m));
        messages.erase(it);
        continue;
      }
      std::shared_ptr<std::vector<psn_t> const> const p_clients =
          engine.config().clients();
      std::vector<psn_t> const & clients = *p_clients;
      psn_t receiver;
      if (message.receiver_slug() == "*") {
        using i_t = decltype(clients.size());
        i_t i;
        do {
          i = sst::rand_range(
              static_cast<i_t>(0),
              static_cast<i_t>(clients.size() - static_cast<i_t>(1)));
        } while (clients[i] == my_psn);
        receiver = clients[i];
      } else if (std::binary_search(clients.begin(),
                                    clients.end(),
                                    message.receiver())) {
        receiver = message.receiver();
      } else {
        messages.erase(it);
        continue;
      }
      sst::precise_sleep_until_unix_time_ns(
          dawn + (sst::checked(message.time().a()) - replay->offset()));
      engine.processClrMsg(receiver.value(), message.content());
      if (message.repeat()) {
        replay_message_spec_t m = message;
        m.time() = sst::checked_cast<sst::unix_time_ns_t>(
            sst::checked(message.time().a())
            + message.repeat()->delay().eval().a());
        if (sst::perfect_eq(m.repeat()->count(), 1)) {
          m.repeat().reset();
        } else if (sst::is_positive(m.repeat()->count())) {
          --m.repeat()->count();
        }
        messages.emplace(std::move(m));
      }
      messages.erase(it);
    }
  }

  engine.wait();

} //

} // namespace kestrel
