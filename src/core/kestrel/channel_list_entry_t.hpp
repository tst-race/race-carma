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

#ifndef KESTREL_CHANNEL_LIST_ENTRY_T_HPP
#define KESTREL_CHANNEL_LIST_ENTRY_T_HPP

#include <string>

#include <kestrel/connection_type_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/transmission_type_t.hpp>

namespace kestrel {

class channel_list_entry_t final {
  friend class channel_list_t;

  channel_list_entry_t() = default;

  //--------------------------------------------------------------------
  // channel_id
  //--------------------------------------------------------------------

private:
  bool done_channel_id_ = false;
  std::string channel_id_;
  std::string const & channel_id(nlohmann::json & src);

public:
  std::string const & channel_id() const;

  //--------------------------------------------------------------------
  // connection_type
  //--------------------------------------------------------------------

private:
  bool done_connection_type_ = false;
  connection_type_t connection_type_;
  connection_type_t connection_type(nlohmann::json & src);

public:
  connection_type_t connection_type() const;

  //--------------------------------------------------------------------
  // description
  //--------------------------------------------------------------------

private:
  bool done_description_ = false;
  std::string description_;
  std::string const & description(nlohmann::json & src);

public:
  std::string const & description() const;

  //--------------------------------------------------------------------
  // reliable
  //--------------------------------------------------------------------

private:
  bool done_reliable_ = false;
  bool reliable_;
  bool reliable(nlohmann::json & src);

public:
  bool reliable() const;

  //--------------------------------------------------------------------
  // transmission_type
  //--------------------------------------------------------------------

private:
  bool done_transmission_type_ = false;
  transmission_type_t transmission_type_;
  transmission_type_t transmission_type(nlohmann::json & src);

public:
  transmission_type_t transmission_type() const;

  //--------------------------------------------------------------------

public:
  channel_list_entry_t(channel_list_entry_t const &) = default;
  channel_list_entry_t(channel_list_entry_t &&) = default;
  channel_list_entry_t &
  operator=(channel_list_entry_t const &) = default;
  channel_list_entry_t & operator=(channel_list_entry_t &&) = default;
  ~channel_list_entry_t() = default;

  static channel_list_entry_t from_json(nlohmann::json && src);
  static channel_list_entry_t from_json(nlohmann::json const & src);

  friend void to_json(nlohmann::json & dst,
                      channel_list_entry_t const & src);
  nlohmann::json to_json() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_CHANNEL_LIST_ENTRY_T_HPP
