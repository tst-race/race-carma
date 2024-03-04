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

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/mock_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/tracing_event_t.hpp>

using namespace kestrel;
using namespace kestrel::carma;

namespace {

std::atomic_bool sigint_(false);

} // namespace

int main(int const argc, char ** const argv) {

  tracing_event_t SST_TEV_DEF(tev);

  if (argc < 2) {
    std::cerr << "Usage: carma whisper <json_file> <to> <msg>";
    std::cerr << "Usage: carma whisper <configs_dir> <period>";
    return EXIT_FAILURE;
  }

  if (argc == 4) {
    kestrel::mock_sdk_t sdk(
        sigint_,
        sst::json::get_from_file<nlohmann::json>(argv[1]));
    sdk.whisper(sdk.getActivePersona(), argv[2], argv[3]);
    return EXIT_SUCCESS;
  }

  if (argc == 3) {

    std::string const configs_dir = argv[1];
    std::string first_client_dir;
    std::vector<psn_t> clients;
    for (sst::path & path : sst::dir_it(configs_dir)) {
      if (path.test_d()) {
        config_t const config(SST_TEV_ARG(tev), path.str());
        psn_t const & me = config.local().psn();
        if (config.phonebook().at(SST_TEV_ARG(tev), me)->role()
            == role_t::client()) {
          first_client_dir = path.str();
          for (auto const & psn_pbe : config.phonebook()) {
            psn_t const & persona = psn_pbe.first;
            if (config.phonebook().at(SST_TEV_ARG(tev), persona)->role()
                == role_t::client()) {
              clients.push_back(persona);
            }
          }
          break;
        }
      }
    }

    // We'll use several threads to do the work if needed, aiming for
    // each thread to do a sleep of up to 100ms between messages.
    double const s1 = std::stod(argv[2]) * 2 / clients.size();
    long long const num_threads =
        static_cast<long long>(std::ceil(0.1 / s1));
    long long const sleep_us =
        static_cast<long long>(s1 * num_threads * 1000000);

    nlohmann::json const whisper_json =
        sst::json::get_from_file<nlohmann::json>(first_client_dir
                                                 + "/whisper.json");

    std::mutex stdout_mutex;
    std::atomic<unsigned long long> msg_i{0};
    std::vector<std::future<void>> futures;
    std::string const nonce =
        sst::to_string(sst::rand_range<unsigned long long>());
    for (decltype(+num_threads) i = 0; i < num_threads; ++i) {
      futures.emplace_back(std::async(std::launch::async, [&] {
        mock_sdk_t sdk(sigint_, whisper_json);
        while (true) {
          std::this_thread::sleep_for(
              std::chrono::microseconds(sst::rand_range(sleep_us)));
          auto const i = sst::rand_range(clients.size() - 1);
          auto const j = [&] {
            while (true) {
              auto const j = sst::rand_range(clients.size() - 1);
              if (j != i) {
                return j;
              }
            }
          }();
          ++msg_i;
          sdk.whisper(clients[i].string(),
                      clients[j].string(),
                      "hello " + nonce + " "
                          + sst::to_string(msg_i.load()));
          {
            std::lock_guard<std::mutex> const lock(stdout_mutex);
            std::cout << "s" << std::endl;
          }
        }
      }));
    }
    for (auto & future : futures) {
      future.get();
    }

    return EXIT_SUCCESS;

  } //

  return EXIT_FAILURE;

} //
