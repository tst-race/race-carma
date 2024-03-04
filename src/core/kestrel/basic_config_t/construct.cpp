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
#include <kestrel/basic_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/basic_config_t.hpp>
//

#include <string>
#include <utility>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

basic_config_t::basic_config_t(tracing_event_t,
                               std::string dir,
                               common_sdk_t * const sdk)
    : dir_(std::move(dir)),
      json_file_(dir_ + "/config.json"),
      sdk_(sdk) {
}

} // namespace kestrel
