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

#ifndef KESTREL_WRITE_ATOMIC_JSON_FILE_HPP
#define KESTREL_WRITE_ATOMIC_JSON_FILE_HPP

#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/write_atomic_file.hpp>

namespace kestrel {

template<class Src>
void write_atomic_json_file(tracing_event_t tev,
                            common_sdk_t * const sdk,
                            std::string const & file,
                            Src const & src) {
  SST_TEV_TOP(tev);
  std::string const text = json_t(src).dump(2, ' ', true);
  std::vector<unsigned char> const data(text.begin(), text.end());
  write_atomic_file(SST_TEV_ARG(tev), sdk, file, data);
  SST_TEV_BOT(tev);
}

} // namespace kestrel

#endif // #ifndef KESTREL_WRITE_ATOMIC_JSON_FILE_HPP
