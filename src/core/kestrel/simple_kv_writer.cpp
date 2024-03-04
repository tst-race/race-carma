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
#include <kestrel/simple_kv_writer.hpp>
// Include twice to test idempotence.
#include <kestrel/simple_kv_writer.hpp>
//

#include <sst/catalog/SST_ASSERT.h>

namespace kestrel {

void simple_kv_writer::escape_into_buf(char_t const * p) {
  SST_ASSERT(!moved_from_);
  SST_ASSERT(p != nullptr);
  for (; *p != static_cast<char_t>(0); ++p) {
    if (*p == static_cast<char_t>(':') || *p == static_cast<char_t>(',')
        || *p == static_cast<char_t>('%')) {
      buf_ += static_cast<char_t>('%');
    }
    buf_ += *p;
  }
}

void simple_kv_writer::write(char_t const * const key,
                             char_t const * const value) {
  SST_ASSERT(!moved_from_);
  SST_ASSERT(key != nullptr);
  SST_ASSERT(value != nullptr);
  buf_.clear();
  if (!first_) {
    buf_ += static_cast<char_t>(',');
  }
  escape_into_buf(key);
  buf_ += static_cast<char_t>(':');
  escape_into_buf(value);
  dst_ += buf_;
  first_ = false;
}

} // namespace kestrel
