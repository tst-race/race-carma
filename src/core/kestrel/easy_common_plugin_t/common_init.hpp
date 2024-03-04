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

#ifndef KESTREL_EASY_COMMON_PLUGIN_T_COMMON_INIT_HPP
#define KESTREL_EASY_COMMON_PLUGIN_T_COMMON_INIT_HPP

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <PluginConfig.h>

#include <kestrel/common_plugin_t.hpp>
#include <kestrel/easy_common_plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Sdk, class Config>
void easy_common_plugin_t<Sdk, Config>::common_init(
    tracing_event_t tev,
    PluginConfig const & pluginConfig) {
  SST_TEV_ADD(tev);
  try {
    common_plugin_t<Sdk>::common_init(SST_TEV_ARG(tev), pluginConfig);

    //------------------------------------------------------------------
    // Initialize the config
    //------------------------------------------------------------------

    config_.emplace(SST_TEV_ARG(tev), ".", &this->sdk());

    //------------------------------------------------------------------
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel

#endif // #ifndef KESTREL_EASY_COMMON_PLUGIN_T_COMMON_INIT_HPP
