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

#ifndef KESTREL_CONNECTION_STATUS_T_HPP
#define KESTREL_CONNECTION_STATUS_T_HPP

#include <ConnectionStatus.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_CONNECTION_STATUS_ITEMS(ITEM)                          \
  ITEM(available, CONNECTION_AVAILABLE)                                \
  ITEM(awaiting_contact, CONNECTION_AWAITING_CONTACT)                  \
  ITEM(closed, CONNECTION_CLOSED)                                      \
  ITEM(init_failed, CONNECTION_INIT_FAILED)                            \
  ITEM(invalid, CONNECTION_INVALID)                                    \
  ITEM(open, CONNECTION_OPEN)                                          \
  ITEM(unavailable, CONNECTION_UNAVAILABLE)

KESTREL_SDK_ENUM_WRAPPER(connection_status_t,
                         KESTREL_CONNECTION_STATUS_ITEMS,
                         ConnectionStatus,
                         CONNECTION_INVALID)

} // namespace kestrel

#endif // #ifndef KESTREL_CONNECTION_STATUS_T_HPP
