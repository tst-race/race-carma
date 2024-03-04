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
#include <kestrel/common_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/common_sdk_t.hpp>
//

#include <RaceLog.h>

#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template bool common_sdk_t::log<decltype(__LINE__)>(
    char const * file,
    char const * func,
    decltype(__LINE__) line,
    RaceLog::LogLevel level,
    tracing_event_t tev) const noexcept;

} // namespace kestrel
