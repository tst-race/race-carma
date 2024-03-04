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

#ifndef KESTREL_CHANNEL_STATUS_T_HPP
#define KESTREL_CHANNEL_STATUS_T_HPP

#include <ChannelStatus.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_CHANNEL_STATUS_ITEMS(ITEM)                             \
  ITEM(available, CHANNEL_AVAILABLE)                                   \
  ITEM(disabled, CHANNEL_DISABLED)                                     \
  ITEM(enabled, CHANNEL_ENABLED)                                       \
  ITEM(failed, CHANNEL_FAILED)                                         \
  ITEM(starting, CHANNEL_STARTING)                                     \
  ITEM(unavailable, CHANNEL_UNAVAILABLE)                               \
  ITEM(undef, CHANNEL_UNDEF)                                           \
  ITEM(unsupported, CHANNEL_UNSUPPORTED)

KESTREL_SDK_ENUM_WRAPPER(channel_status_t,
                         KESTREL_CHANNEL_STATUS_ITEMS,
                         ChannelStatus,
                         CHANNEL_UNDEF)

} // namespace kestrel

#endif // #ifndef KESTREL_CHANNEL_STATUS_T_HPP
