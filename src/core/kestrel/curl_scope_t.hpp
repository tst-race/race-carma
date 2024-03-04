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

#ifndef KESTREL_CURL_SCOPE_T_HPP
#define KESTREL_CURL_SCOPE_T_HPP

#include <curl/curl.h>
#include <mutex>
#include <new>

namespace kestrel {

//
// curl_scope_t calls curl_global_init on construction and
// curl_global_cleanup on destruction:
//
//       // Throws upon any error.
//       curl_scope_t curl_scope;
//
//       // Never throws, check status() for success.
//       curl_scope_t curl_scope(std::nothrow);
//
// The reason for having the std::nothrow variant is because we're
// sometimes interested in sending "fire and forget" HTTP requests where
// we don't care whether the request succeeds, and this variant makes it
// easier to do that.
//

class curl_scope_t {
  static std::mutex mutex;
  CURLcode s;

public:
  curl_scope_t();
  curl_scope_t(std::nothrow_t const &) noexcept;
  ~curl_scope_t() noexcept;

  curl_scope_t(curl_scope_t const &) = delete;
  curl_scope_t & operator=(curl_scope_t const &) = delete;
  curl_scope_t(curl_scope_t &&) = delete;
  curl_scope_t & operator=(curl_scope_t &&) = delete;

  CURLcode status() const noexcept;
};

} // namespace kestrel

#endif // #ifndef KESTREL_CURL_SCOPE_T_HPP
