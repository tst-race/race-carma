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

#ifndef KESTREL_EASY_TA1_PLUGIN_T_OPENCONNECTION_ID_TYPE_HPP
#define KESTREL_EASY_TA1_PLUGIN_T_OPENCONNECTION_ID_TYPE_HPP

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Config>
race_handle_t
easy_ta1_plugin_t<Config>::openConnection(tracing_event_t tev,
                                          link_id_t const & link_id,
                                          link_type_t const link_type) {
  SST_TEV_TOP(tev);
  return race_handle_t(sdk().openConnection(SST_TEV_ARG(tev),
                                            link_type.value(),
                                            link_id.value(),
                                            "{}",
                                            0,
                                            3600,
                                            0));
  SST_TEV_BOT(tev);
}

} // namespace kestrel

#endif // #ifndef KESTREL_EASY_TA1_PLUGIN_T_OPENCONNECTION_ID_TYPE_HPP
