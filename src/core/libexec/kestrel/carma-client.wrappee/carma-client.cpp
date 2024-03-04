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
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <RaceLog.h>

#include <kestrel/carma/plugin_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/mock_sdk_t.hpp>

namespace {

std::atomic_bool sigint_(false);

} // namespace

extern "C" void handle_sigint(int) {
  sigint_ = true;
}

int main(int const argc, char ** const argv) {

  if (std::signal(SIGINT, &handle_sigint) == SIG_ERR) {
    throw std::runtime_error("std::signal() failed");
  }

  if (argc < 2) {
    std::cerr << "Usage: carma client <json_file>" << std::endl;
    return EXIT_FAILURE;
  }

  RaceLog::setLogLevel(RaceLog::LL_DEBUG);
  RaceLog::setLogLevelStdout(RaceLog::LL_DEBUG);
  RaceLog::setLogLevelFile(RaceLog::LL_DEBUG);

  std::ifstream f(argv[1]);
  kestrel::mock_sdk_t sdk(sigint_, nlohmann::json::parse(f));
  kestrel::carma::plugin_t plugin(sdk);
  sdk.run(plugin);
  return EXIT_SUCCESS;
}
