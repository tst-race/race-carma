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

#ifndef KESTREL_RACE_TO_JSON_HPP
#define KESTREL_RACE_TO_JSON_HPP

#include <ChannelProperties.h>
#include <ClrMsg.h>
#include <DeviceInfo.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <PluginConfig.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------

json_t race_to_json(ChannelProperties const & x);

//----------------------------------------------------------------------

json_t race_to_json(ClrMsg const & x);

//----------------------------------------------------------------------

json_t race_to_json(DeviceInfo const & x);

//----------------------------------------------------------------------

json_t race_to_json(EncPkg const & x);

//----------------------------------------------------------------------

json_t race_to_json(LinkProperties const & x);

//----------------------------------------------------------------------

json_t race_to_json(PluginConfig const & x);

//----------------------------------------------------------------------

json_t race_to_json(SdkResponse const & x);

//----------------------------------------------------------------------

} // namespace kestrel

#endif // #ifndef KESTREL_RACE_TO_JSON_HPP
