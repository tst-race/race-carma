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
#include <kestrel/package_status_t.hpp>
// Include twice to test idempotence.
#include <kestrel/package_status_t.hpp>
//

#include <PackageStatus.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

void to_json(json_t & dst, package_status_t const & src) {
  PackageStatus const x = src.value();
  // clang-format off
  switch (x) {
    default: dst = x; break;
#define F(y) case y: dst = #y; break;
    F(PACKAGE_FAILED_GENERIC)
    F(PACKAGE_FAILED_NETWORK_ERROR)
    F(PACKAGE_FAILED_TIMEOUT)
    F(PACKAGE_INVALID)
    F(PACKAGE_RECEIVED)
    F(PACKAGE_SENT)
#undef F
  }
  // clang-format on
}

} // namespace kestrel
