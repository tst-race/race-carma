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
#include <kestrel/vrf_shell_t.hpp>
// Include twice to test idempotence.
#include <kestrel/vrf_shell_t.hpp>
//

#include <string>
#include <utility>

#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/remove_as.hpp>
#include <sst/catalog/json/unknown_key.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/vrf_algorand_t.hpp>

namespace kestrel {

void from_json(json_t src, vrf_shell_t & dst) {

  std::string const name =
      sst::json::remove_as<std::string>(src, "name");

  json_t params =
      src.contains("params") ? src["params"] : json_t(nullptr);
  src.erase("params");

  if (name == "algorand") {
    dst.core_.reset(new vrf_algorand_t(std::move(params)));
  } else {
    throw sst::json::exception::invalid_value();
  }

  sst::json::unknown_key(src);

} //

} // namespace kestrel
