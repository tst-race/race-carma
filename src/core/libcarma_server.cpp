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

#include <string>

#include <IRacePluginNM.h>
#include <RaceLog.h>
#include <RacePluginExports.h>

#include <kestrel/carma/plugin_t.hpp>
#include <kestrel/config.h>

RaceVersionInfo const raceVersion = RACE_VERSION;

char const * const racePluginId = "carma";

char const * const racePluginDescription =
    "Stealth CARMA RACE TA1 server plugin (Artifactory "
    "revision " CARMA_ARTIFACTORY_REVISION
    ", RACE version " CARMA_RACE_VERSION
    ", RiB version " CARMA_RIB_VERSION ", CARMA version " CARMA_VERSION
    ")";

IRacePluginNM * createPluginNM(IRaceSdkNM * const sdk) {
  if (sdk == nullptr) {
    try {
      RaceLog::log(RaceLog::LL_ERROR,
                   "carma",
                   "createPluginNM: sdk == nullptr",
                   "");
    } catch (...) {
    }
    return nullptr;
  }
  try {
    return new kestrel::carma::plugin_t(*sdk);
  } catch (std::exception const & e) {
    try {
      RaceLog::log(
          RaceLog::LL_ERROR,
          "carma",
          std::string("createPluginNM: new "
                      "kestrel::carma::plugin_t(*sdk) failed: ")
              + e.what(),
          "");
    } catch (...) {
    }
    return nullptr;
  } catch (...) {
    try {
      RaceLog::log(
          RaceLog::LL_ERROR,
          "carma",
          "createPluginNM: new kestrel::carma::plugin_t(*sdk) "
          "failed: unknown error",
          "");
    } catch (...) {
    }
    return nullptr;
  }
}

void destroyPluginNM(IRacePluginNM * const plugin) {
  delete plugin;
}
