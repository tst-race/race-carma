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

#ifndef KESTREL_TRANSMISSION_TYPE_T_HPP
#define KESTREL_TRANSMISSION_TYPE_T_HPP

#include <TransmissionType.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_TRANSMISSION_TYPE_ITEMS(ITEM)                          \
  ITEM(multicast, TT_MULTICAST)                                        \
  ITEM(undef, TT_UNDEF)                                                \
  ITEM(unicast, TT_UNICAST)

KESTREL_SDK_ENUM_WRAPPER(transmission_type_t,
                         KESTREL_TRANSMISSION_TYPE_ITEMS,
                         TransmissionType,
                         TT_UNDEF)

} // namespace kestrel

#endif // #ifndef KESTREL_TRANSMISSION_TYPE_T_HPP
