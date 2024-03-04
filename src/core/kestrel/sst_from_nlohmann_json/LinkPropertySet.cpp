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

// Include first to test independence.
#include <kestrel/sst_from_nlohmann_json.hpp>
// Include twice to test idempotence.
#include <kestrel/sst_from_nlohmann_json.hpp>
//

#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>

#include <LinkPropertySet.h>

#include <kestrel/json_t.hpp>

using namespace kestrel;

void sst_from_nlohmann_json(json_t src, LinkPropertySet & dst) {
  sst::json::remove_to(src, dst.bandwidth_bps, "bandwidth_bps");
  sst::json::remove_to(src, dst.latency_ms, "latency_ms");
  sst::json::remove_to(src, dst.loss, "loss");
  sst::json::unknown_key(src);
}
