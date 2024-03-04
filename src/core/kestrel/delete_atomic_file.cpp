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
#include <kestrel/delete_atomic_file.hpp>
// Include twice to test idempotence.
#include <kestrel/delete_atomic_file.hpp>
//

#include <string>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/rm_f_r.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void delete_atomic_file(tracing_event_t tev,
                        common_sdk_t * const sdk,
                        std::string const & file) {
  SST_TEV_TOP(tev);
  if (sdk) {
    sdk->removeDir(SST_TEV_ARG(tev), file);
  } else {
    sst::rm_f_r(file);
  }
  SST_TEV_BOT(tev);
}

} // namespace kestrel
