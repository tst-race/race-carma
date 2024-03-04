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

#ifndef KESTREL_COMMON_PLUGIN_T_COMMON_INIT_HPP
#define KESTREL_COMMON_PLUGIN_T_COMMON_INIT_HPP

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <PluginConfig.h>

#include <kestrel/common_plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Sdk>
void common_plugin_t<Sdk>::common_init(tracing_event_t tev,
                                       PluginConfig const &) {
  SST_TEV_ADD(tev);
  try {
    persona_.emplace(sdk_.getActivePersona(SST_TEV_ARG(tev)));

    // Call xListDirRecursive just to log what's there.
    // sdk_.xListDirRecursive(SST_TEV_ARG(tev), ".");

  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel

#endif // #ifndef KESTREL_COMMON_PLUGIN_T_COMMON_INIT_HPP
