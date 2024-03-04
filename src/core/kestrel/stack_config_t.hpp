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

#ifndef KESTREL_STACK_CONFIG_T_HPP
#define KESTREL_STACK_CONFIG_T_HPP

#include <array>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/bigint.hpp>
#include <string>
#include <vector>

namespace kestrel {

class stack_config_t final {

  stack_config_t() = default;

  //--------------------------------------------------------------------
  // allow_multicast
  //--------------------------------------------------------------------

private:
  bool done_allow_multicast_ = false;
  bool allow_multicast_;
  bool const & allow_multicast(nlohmann::json & src);

public:
  bool const & allow_multicast() const;

  //--------------------------------------------------------------------
  // num_clients
  //--------------------------------------------------------------------

private:
  bool done_num_clients_ = false;
  sst::bigint num_clients_;
  sst::bigint const & num_clients(nlohmann::json & src);

public:
  sst::bigint const & num_clients() const;
  static int default_num_clients();

  //--------------------------------------------------------------------
  // num_mc_groups
  //--------------------------------------------------------------------

private:
  bool done_num_mc_groups_ = false;
  sst::bigint num_mc_groups_;
  sst::bigint const & num_mc_groups(nlohmann::json & src);

public:
  sst::bigint const & num_mc_groups() const;

  //--------------------------------------------------------------------
  // num_servers
  //--------------------------------------------------------------------

private:
  bool done_num_servers_ = false;
  sst::bigint num_servers_;
  sst::bigint const & num_servers(nlohmann::json & src);

public:
  sst::bigint const & num_servers() const;
  static int default_num_servers();

  //--------------------------------------------------------------------
  // send_retry_count
  //--------------------------------------------------------------------

private:
  bool done_send_retry_count_ = false;
  sst::bigint send_retry_count_;
  sst::bigint const & send_retry_count(nlohmann::json & src);

public:
  sst::bigint const & send_retry_count() const;

  //--------------------------------------------------------------------
  // send_retry_window
  //--------------------------------------------------------------------

private:
  bool done_send_retry_window_ = false;
  sst::bigint send_retry_window_;
  sst::bigint const & send_retry_window(nlohmann::json & src);

public:
  sst::bigint const & send_retry_window() const;

  //--------------------------------------------------------------------
  // send_timeout
  //--------------------------------------------------------------------

private:
  bool done_send_timeout_ = false;
  sst::bigint send_timeout_;
  sst::bigint const & send_timeout(nlohmann::json & src);

public:
  sst::bigint const & send_timeout() const;

  //--------------------------------------------------------------------

public:
  stack_config_t(stack_config_t const &) = default;
  stack_config_t & operator=(stack_config_t const &) = default;

  stack_config_t(stack_config_t &&) = default;
  stack_config_t & operator=(stack_config_t &&) = delete;

  ~stack_config_t() = default;

  sst::bigint prime;
  int mix_size;
  int secret_key_length;
  int threshold;
  int log_level;

  bool leader_relay_only;
  bool inter_server_direct_only;

  static stack_config_t from_json(nlohmann::json src);
};

} // namespace kestrel

#endif // #ifndef KESTREL_STACK_CONFIG_T_HPP
