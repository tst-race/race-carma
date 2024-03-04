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
#include <kestrel/curl_scope_t.hpp>
// Include twice to test idempotence.
#include <kestrel/curl_scope_t.hpp>

#include <curl/curl.h>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>

namespace kestrel {

std::mutex curl_scope_t::mutex;

curl_scope_t::curl_scope_t(std::nothrow_t const &) noexcept {
  std::lock_guard<decltype(mutex)> const lock(mutex);
  s = curl_global_init(CURL_GLOBAL_ALL);
}

curl_scope_t::curl_scope_t() : curl_scope_t(std::nothrow) {
  if (s != CURLE_OK) {
    throw std::runtime_error(
        std::string("curl_global_init failed: ") +
        curl_easy_strerror(s));
  }
}

curl_scope_t::~curl_scope_t() noexcept {
  if (s == CURLE_OK) {
    std::lock_guard<decltype(mutex)> const lock(mutex);
    curl_global_cleanup();
  }
}

CURLcode curl_scope_t::status() const noexcept {
  return s;
}

} // namespace kestrel
