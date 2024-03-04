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

#ifndef KESTREL_PLUGIN_RESPONSE_T_HPP
#define KESTREL_PLUGIN_RESPONSE_T_HPP

#include <PluginResponse.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_PLUGIN_RESPONSE_ITEMS(ITEM)                            \
  ITEM(invalid, PLUGIN_INVALID)                                        \
  ITEM(ok, PLUGIN_OK)                                                  \
  ITEM(temp_error, PLUGIN_TEMP_ERROR)                                  \
  ITEM(error, PLUGIN_ERROR)                                            \
  ITEM(fatal, PLUGIN_FATAL)

KESTREL_SDK_ENUM_WRAPPER(plugin_response_t,
                         KESTREL_PLUGIN_RESPONSE_ITEMS,
                         PluginResponse,
                         PLUGIN_INVALID)

} // namespace kestrel

#endif // #ifndef KESTREL_PLUGIN_RESPONSE_T_HPP
