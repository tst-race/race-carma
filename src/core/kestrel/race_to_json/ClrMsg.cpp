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
#include <kestrel/race_to_json.hpp>
// Include twice to test idempotence.
#include <kestrel/race_to_json.hpp>
//

#include <sst/catalog/to_string.hpp>

#include <ClrMsg.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

json_t race_to_json(ClrMsg const & x) {
  return {
      {"ampIndex", sst::to_string(x.getAmpIndex())},
      {"from", x.getFrom()},
      {"message", x.getMsg()},
      {"nonce", sst::to_string(x.getNonce())},
      {"spanId", sst::to_string(x.getSpanId())},
      {"time", sst::to_string(x.getTime())},
      {"to", x.getTo()},
      {"traceId", sst::to_string(x.getTraceId())},
  };
}

} // namespace kestrel
