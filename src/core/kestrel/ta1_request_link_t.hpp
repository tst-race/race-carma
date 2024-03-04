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

#ifndef KESTREL_TA1_REQUEST_LINK_T_HPP
#define KESTREL_TA1_REQUEST_LINK_T_HPP

#include <kestrel/psn_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <set>
#include <string>

namespace kestrel {

class ta1_request_link_t final {
  ta1_request_link_t() = default;

  //--------------------------------------------------------------------
  // channels
  //--------------------------------------------------------------------

private:
  bool done_channels_ = false;
  std::set<std::string> channels_;
  std::set<std::string> const & channels(nlohmann::json & src);

public:
  std::set<std::string> const & channels() const;

  //--------------------------------------------------------------------
  // recipients
  //--------------------------------------------------------------------

private:
  bool done_recipients_ = false;
  std::set<psn_t> recipients_;
  std::set<psn_t> const & recipients(nlohmann::json & src);

public:
  std::set<psn_t> const & recipients() const;

  //--------------------------------------------------------------------
  // sender
  //--------------------------------------------------------------------

private:
  bool done_sender_ = false;
  psn_t sender_;
  psn_t const & sender(nlohmann::json & src);

public:
  psn_t const & sender() const;

  //--------------------------------------------------------------------

public:
  ta1_request_link_t(ta1_request_link_t const &) = default;
  ta1_request_link_t(ta1_request_link_t &&) = default;
  ta1_request_link_t & operator=(ta1_request_link_t const &) = default;
  ta1_request_link_t & operator=(ta1_request_link_t &&) = default;
  ~ta1_request_link_t() = default;

  static ta1_request_link_t from_json(nlohmann::json && src);
  static ta1_request_link_t from_json(nlohmann::json const & src);

  friend void to_json(nlohmann::json & dst,
                      ta1_request_link_t const & src);
  nlohmann::json to_json() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA1_REQUEST_LINK_T_HPP
