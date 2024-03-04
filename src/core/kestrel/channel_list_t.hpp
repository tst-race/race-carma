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

#ifndef KESTREL_CHANNEL_LIST_T_HPP
#define KESTREL_CHANNEL_LIST_T_HPP

#include <kestrel/channel_list_entry_t.hpp>
#include <map>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <string>

namespace kestrel {

class channel_list_t final {
  channel_list_t() = default;

  //--------------------------------------------------------------------
  // channels
  //--------------------------------------------------------------------

private:
  bool done_channels_ = false;
  std::map<std::string, channel_list_entry_t> channels_;
  std::map<std::string, channel_list_entry_t> const &
  channels(nlohmann::json & src);

public:
  std::map<std::string, channel_list_entry_t> const & channels() const;

  //--------------------------------------------------------------------

public:
  channel_list_t(channel_list_t const &) = default;
  channel_list_t(channel_list_t &&) = default;
  channel_list_t & operator=(channel_list_t const &) = default;
  channel_list_t & operator=(channel_list_t &&) = default;
  ~channel_list_t() = default;

  static channel_list_t from_json(nlohmann::json && src);
  static channel_list_t from_json(nlohmann::json const & src);

  friend void to_json(nlohmann::json & dst, channel_list_t const & src);
  nlohmann::json to_json() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_CHANNEL_LIST_T_HPP
