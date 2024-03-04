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

#ifndef KESTREL_READ_ATOMIC_JSON_FILE_HPP
#define KESTREL_READ_ATOMIC_JSON_FILE_HPP

#include <exception>
#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/set_from_string.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/read_atomic_file.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Dst>
void read_atomic_json_file(tracing_event_t tev,
                           common_sdk_t * const sdk,
                           std::string const & file,
                           Dst & dst) {
  SST_TEV_TOP(tev);
  std::vector<unsigned char> const data =
      read_atomic_file(SST_TEV_ARG(tev), sdk, file);
  if (!data.empty()) {
    try {
      sst::json::set_from_string<json_t>(data, dst);
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_file(file));
    }
  }
  SST_TEV_BOT(tev);
}

} // namespace kestrel

#endif // #ifndef KESTREL_READ_ATOMIC_JSON_FILE_HPP
