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
#include <kestrel/post_json_t.hpp>
// Include twice to test idempotence.
#include <kestrel/post_json_t.hpp>

#include <cassert>
#include <curl/curl.h>
#include <exception>
#include <future>
#include <memory>
#include <new>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/checked.h>
#include <string>
#include <vector>

namespace kestrel {

namespace {

template<class T>
void set_option(
    CURL * const curl, CURLoption const option, T const value) {
  assert(curl != nullptr);
  CURLcode const s = curl_easy_setopt(curl, option, value);
  if (s != CURLE_OK) {
    throw std::runtime_error(
        std::string("curl_easy_setopt failed: ") +
        curl_easy_strerror(s));
  }
}

template<class StringIt>
std::unique_ptr<struct curl_slist, void (*)(struct curl_slist *)>
make_list(StringIt first, StringIt const last) {
  std::unique_ptr<struct curl_slist, void (*)(struct curl_slist *)>
      list(nullptr, [](struct curl_slist * const p) noexcept {
        curl_slist_free_all(p);
      });
  for (; first != last; ++first) {
    struct curl_slist * const p =
        curl_slist_append(list.get(), (*first).c_str());
    if (p == nullptr) {
      throw std::runtime_error("curl_slist_append failed");
    }
    list.release();
    list.reset(p);
  }
  return list;
}

} // namespace

post_json_t::post_json_t() {
  curl_scope = std::make_shared<curl_scope_t>();
}

post_json_t::post_json_t(std::nothrow_t const &) noexcept {
  try {
    curl_scope = std::make_shared<curl_scope_t>();
  } catch (...) {
    ctor_exception = std::current_exception();
  }
}

std::future<void> post_json_t::operator()(
    std::string const & url, nlohmann::json const & json) const {
  // We need to be careful to explicitly capture curl_scope by value in
  // the lambda, as we need its lifetime to contain the lambda, but the
  // lambda never actually uses it by name, so using [=] won't work.
  auto & curl_scope = this->curl_scope;
  auto & ctor_exception = this->ctor_exception;
  return std::async(
      std::launch::async, [url, json, curl_scope, ctor_exception] {
        if (ctor_exception) {
          std::rethrow_exception(ctor_exception);
        }
        std::unique_ptr<CURL, void (*)(CURL *)> const curl(
            curl_easy_init(), [](CURL * const p) noexcept {
              curl_easy_cleanup(p);
            });
        if (curl.get() == nullptr) {
          throw std::runtime_error("curl_easy_init failed");
        }
        std::string const body = json.dump(-1, ' ', true);
        std::vector<std::string> const headers = {
            "Content-Type: application/json",
        };
        set_option(
            curl.get(), CURLOPT_URL, const_cast<char *>(url.c_str()));
        auto const header_list =
            make_list(headers.cbegin(), headers.cend());
        if (header_list.get() != nullptr) {
          set_option(curl.get(), CURLOPT_HTTPHEADER, header_list.get());
        }
        set_option(
            curl.get(),
            CURLOPT_POSTFIELDSIZE,
            sst::checked_cast<long>(body.size()));
        set_option(
            curl.get(),
            CURLOPT_POSTFIELDS,
            const_cast<char *>(body.c_str()));
        set_option(
            curl.get(), CURLOPT_TIMEOUT, sst::checked_cast<long>(60));
        CURLcode const s = curl_easy_perform(curl.get());
        if (s != CURLE_OK) {
          throw std::runtime_error(
              std::string("curl_easy_perform failed: ") +
              curl_easy_strerror(s));
        }
      });
};

void post_json_t::operator()(
    std::string const & url,
    nlohmann::json const & json,
    std::nothrow_t const &) const noexcept {
  try {
    (*this)(url, json);
  } catch (...) {
  }
}

} // namespace kestrel
