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

#ifndef KESTREL_EASY_TA2_PLUGIN_T_VERIFY_CHANNEL_ID_HPP
#define KESTREL_EASY_TA2_PLUGIN_T_VERIFY_CHANNEL_ID_HPP

#include <string>

namespace kestrel {

template<class Plugin, class Types>
void easy_ta2_plugin_t<Plugin, Types>::verify_channel_id(
    std::string const & channel_id) {
  if (channel_id != this->sdk().plugin_name()) {
    throw this->sdk().unknown_channel_id(channel_id);
  }
}

} // namespace kestrel

#endif // #ifndef KESTREL_EASY_TA2_PLUGIN_T_VERIFY_CHANNEL_ID_HPP
