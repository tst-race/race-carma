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

#ifndef KESTREL_SST_FROM_NLOHMANN_JSON_HPP
#define KESTREL_SST_FROM_NLOHMANN_JSON_HPP

#include <ChannelProperties.h>
#include <ChannelRole.h>
#include <LinkPropertyPair.h>
#include <LinkPropertySet.h>

#include <kestrel/json_t.hpp>

//
// Note that these functions are in the same namespace as the RACE SDK
// (the global namespace), not in the kestrel namespace. This ensures
// that ADL will find them when we call sst::json::get_to.
//

void sst_from_nlohmann_json(kestrel::json_t src,
                            ChannelProperties & dst);

void sst_from_nlohmann_json(kestrel::json_t src, ChannelRole & dst);

void sst_from_nlohmann_json(kestrel::json_t src,
                            LinkPropertyPair & dst);

void sst_from_nlohmann_json(kestrel::json_t src, LinkPropertySet & dst);

#endif // #ifndef KESTREL_SST_FROM_NLOHMANN_JSON_HPP
