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

#include <sst/catalog/what.hpp>

#include <IRacePluginNM.h>
#include <IRacePluginComms.h>
#include <RaceLog.h>
#include <RacePluginExports.h>

#include <kestrel/config.h>

RaceVersionInfo const raceVersion = RACE_VERSION;

char const * const racePluginId = PLUGIN_ID;

char const * const racePluginDescription =
    PLUGIN_ID " (Kestrel version " CARMA_VERSION
              ", RACE version " CARMA_RACE_VERSION
              ", Artifactory revision " CARMA_ARTIFACTORY_REVISION
              ", RiB version " CARMA_RIB_VERSION ")";

#define PASTE0(a, b) a##b
#define PASTE1(a, b) PASTE0(a, b)
#define STRINGIFY0(a) #a
#define STRINGIFY1(a) STRINGIFY0(a)

#define BASE PASTE1(IRacePlugin, PLUGIN_KIND)
#define SDK PASTE1(IRaceSdk, PLUGIN_KIND)
#define CREATE PASTE1(createPlugin, PLUGIN_KIND)
#define DESTROY PASTE1(destroyPlugin, PLUGIN_KIND)

BASE * CREATE(SDK * const sdk) {
  if (sdk == nullptr) {
    try {
      RaceLog::log(RaceLog::LL_ERROR,
                   racePluginId,
                   STRINGIFY1(CREATE) ": sdk is null.",
                   "");
    } catch (...) {
    }
    return nullptr;
  }
  try {
    return new PLUGIN_TYPE(*sdk);
  } catch (...) {
    try {
      RaceLog::log(RaceLog::LL_ERROR,
                   racePluginId,
                   std::string(STRINGIFY1(CREATE) ": new " STRINGIFY1(
                       PLUGIN_TYPE) "(*sdk) failed: ")
                       + sst::what(),
                   "");
    } catch (...) {
    }
    return nullptr;
  }
}

void DESTROY(BASE * const plugin) {
  delete plugin;
}
