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
#include <kestrel/clrmsg_t.hpp>
// Include twice to test idempotence.
#include <kestrel/clrmsg_t.hpp>

#include <cstddef>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_hex.hpp>
#include <string>

namespace kestrel {

//----------------------------------------------------------------------

template std::size_t clrmsg_t::to_bytes_size() const;

//----------------------------------------------------------------------
// JSON
//----------------------------------------------------------------------
//
// This is only for logging purposes, so convenience is more important
// than performance.
//

nlohmann::json clrmsg_t::to_json() const {
  return {{"msg",
           std::string(reinterpret_cast<char const *>(msg_.data()),
                       reinterpret_cast<char const *>(msg_.data())
                           + msg_.size())},
          {"from",
           std::string(reinterpret_cast<char const *>(from_.data()),
                       reinterpret_cast<char const *>(from_.data())
                           + from_.size())},
          {"to",
           std::string(reinterpret_cast<char const *>(to_.data()),
                       reinterpret_cast<char const *>(to_.data())
                           + to_.size())},
          {"time", sst::to_string(time_)},
          {"nonce", sst::to_string(nonce_)},
          {"trace_id", sst::to_string(trace_id_)},
          {"span_id", sst::to_string(span_id_)},
          {"hash", sst::to_hex(content_hash())}};
};

//----------------------------------------------------------------------

} // namespace kestrel
