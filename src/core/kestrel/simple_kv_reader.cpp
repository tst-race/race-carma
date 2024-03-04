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
#include <kestrel/simple_kv_reader.hpp>
// Include twice to test idempotence.
#include <kestrel/simple_kv_reader.hpp>
//

#include <sst/catalog/SST_ASSERT.h>

#include <kestrel/simple_kv_syntax_error.hpp>

namespace kestrel {

bool simple_kv_reader::read() {
  SST_ASSERT(!moved_from_);
  have_kv_ = false;
  char_t const * src = src_;
  while (*src == static_cast<char_t>(',')) {
    ++src;
  }
  if (*src == static_cast<char_t>(0)) {
    return false;
  }
  key_.clear();
  while (true) {
    if (*src == static_cast<char_t>(0)) {
      throw simple_kv_syntax_error();
    } else if (*src == static_cast<char_t>(':')) {
      ++src;
      break;
    } else if (*src != static_cast<char_t>('%')) {
      key_ += *src++;
    } else if (*++src == static_cast<char_t>(0)) {
      throw simple_kv_syntax_error();
    } else {
      key_ += *src++;
    }
  }
  value_.clear();
  while (true) {
    if (*src == static_cast<char_t>(0)) {
      break;
    } else if (*src == static_cast<char_t>(',')) {
      ++src;
      break;
    } else if (*src != static_cast<char_t>('%')) {
      value_ += *src++;
    } else if (*++src == static_cast<char_t>(0)) {
      throw simple_kv_syntax_error();
    } else {
      value_ += *src++;
    }
  }
  src_ = src;
  have_kv_ = true;
  return true;
}

} // namespace kestrel
