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

#ifndef KESTREL_USER_DISPLAY_TYPE_T_HPP
#define KESTREL_USER_DISPLAY_TYPE_T_HPP

#include <RaceEnums.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_USER_DISPLAY_TYPE_ITEMS(ITEM)                          \
  ITEM(dialog, RaceEnums::UD_DIALOG)                                   \
  ITEM(notification, RaceEnums::UD_NOTIFICATION)                       \
  ITEM(qr_code, RaceEnums::UD_QR_CODE)                                 \
  ITEM(toast, RaceEnums::UD_TOAST)                                     \
  ITEM(undef, RaceEnums::UD_UNDEF)

KESTREL_SDK_ENUM_WRAPPER(user_display_type_t,
                         KESTREL_USER_DISPLAY_TYPE_ITEMS,
                         RaceEnums::UserDisplayType,
                         RaceEnums::UD_UNDEF)

} // namespace kestrel

#endif // #ifndef KESTREL_USER_DISPLAY_TYPE_T_HPP
