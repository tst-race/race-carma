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

#ifndef KESTREL_BOOTSTRAP_ACTION_TYPE_T_HPP
#define KESTREL_BOOTSTRAP_ACTION_TYPE_T_HPP

#include <RaceEnums.h>

#include <kestrel/KESTREL_SDK_ENUM_WRAPPER.hpp>

namespace kestrel {

#define KESTREL_BOOTSTRAP_ACTION_TYPE_ITEMS(ITEM)                      \
  ITEM(acquiring_artifact, RaceEnums::BS_ACQUIRING_ARTIFACT)           \
  ITEM(complete, RaceEnums::BS_COMPLETE)                               \
  ITEM(creating_bundle, RaceEnums::BS_CREATING_BUNDLE)                 \
  ITEM(download_bundle, RaceEnums::BS_DOWNLOAD_BUNDLE)                 \
  ITEM(failed, RaceEnums::BS_FAILED)                                   \
  ITEM(network_connect, RaceEnums::BS_NETWORK_CONNECT)                 \
  ITEM(preparing_bootstrap, RaceEnums::BS_PREPARING_BOOTSTRAP)         \
  ITEM(preparing_configs, RaceEnums::BS_PREPARING_CONFIGS)             \
  ITEM(preparing_transfer, RaceEnums::BS_PREPARING_TRANSFER)           \
  ITEM(undef, RaceEnums::BS_UNDEF)

KESTREL_SDK_ENUM_WRAPPER(bootstrap_action_type_t,
                         KESTREL_BOOTSTRAP_ACTION_TYPE_ITEMS,
                         RaceEnums::BootstrapActionType,
                         RaceEnums::BS_UNDEF)

} // namespace kestrel

#endif // #ifndef KESTREL_BOOTSTRAP_ACTION_TYPE_T_HPP
