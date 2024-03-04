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

#ifndef KESTREL_POST_JSON_T_HPP
#define KESTREL_POST_JSON_T_HPP

#include <kestrel/curl_scope_t.hpp>
#include <exception>
#include <future>
#include <memory>
#include <new>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <string>

namespace kestrel {

//
// post_json_t lets us send HTTP JSON POST requests asynchronously:
//
//       // Synchronous (due to get()), throws upon any error.
//       post_json_t post_json;
//       post_json("http://foo.com/bar", json).get();
//
//       // Asynchronous, never throws.
//       post_json_t post_json(std::nothrow);
//       post_json("http://foo.com/bar", json, std::nothrow);
//
// The reason for having the std::nothrow variant is because we're
// sometimes interested in sending "fire and forget" HTTP requests where
// we don't care whether the request succeeds, and this variant makes it
// easier to do that.
//

class post_json_t {
  std::shared_ptr<curl_scope_t> curl_scope;
  std::exception_ptr ctor_exception;

public:
  post_json_t();
  post_json_t(std::nothrow_t const &) noexcept;

  ~post_json_t() noexcept {
  }

  post_json_t(post_json_t const &) = delete;
  post_json_t & operator=(post_json_t const &) = delete;
  post_json_t(post_json_t &&) = delete;
  post_json_t & operator=(post_json_t &&) = delete;

  std::future<void> operator()(
      std::string const & url, nlohmann::json const & json) const;

  void operator()(
      std::string const & url,
      nlohmann::json const & json,
      std::nothrow_t const &) const noexcept;
};

} // namespace kestrel

#endif // #ifndef KESTREL_POST_JSON_T_HPP
