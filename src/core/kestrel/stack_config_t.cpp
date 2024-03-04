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
#include <kestrel/stack_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/stack_config_t.hpp>
//

#include <array>
#include <exception>
#include <string>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/ceil_lg.hpp>
#include <sst/catalog/floor_sqrt.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/perfect_lt.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// allow_multicast
//----------------------------------------------------------------------

bool const & stack_config_t::allow_multicast(nlohmann::json & src) {
  if (!done_allow_multicast_) {
    sst::json::remove_to(src,
                         allow_multicast_,
                         "allow_multicast",
                         false);
    done_allow_multicast_ = true;
  }
  return allow_multicast_;
}

bool const & stack_config_t::allow_multicast() const {
  SST_ASSERT((done_allow_multicast_));
  return allow_multicast_;
}

//----------------------------------------------------------------------
// num_clients
//----------------------------------------------------------------------

int stack_config_t::default_num_clients() {
  return 3;
}

sst::bigint const & stack_config_t::num_clients(nlohmann::json & src) {
  if (!done_num_clients_) {
    sst::json::remove_to(src,
                         num_clients_,
                         "num_clients",
                         default_num_clients());
    try {
      if (num_clients_ < 1) {
        throw sst::json::exception("value must be a positive integer");
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("num_clients"));
    }
    done_num_clients_ = true;
  }
  return num_clients_;
}

sst::bigint const & stack_config_t::num_clients() const {
  SST_ASSERT((done_num_clients_));
  return num_clients_;
}

//----------------------------------------------------------------------
// num_servers
//----------------------------------------------------------------------

int stack_config_t::default_num_servers() {
  return 15;
}

sst::bigint const & stack_config_t::num_servers(nlohmann::json & src) {
  if (!done_num_servers_) {
    sst::json::remove_to(src,
                         num_servers_,
                         "num_servers",
                         default_num_servers());
    try {
      if (num_servers_ < 1) {
        throw sst::json::exception("value must be a positive integer");
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("num_servers"));
    }
    done_num_servers_ = true;
  }
  return num_servers_;
}

sst::bigint const & stack_config_t::num_servers() const {
  SST_ASSERT((done_num_servers_));
  return num_servers_;
}

//----------------------------------------------------------------------
// send_retry_count
//----------------------------------------------------------------------

sst::bigint const &
stack_config_t::send_retry_count(nlohmann::json & src) {
  if (!done_send_retry_count_) {
    sst::json::remove_to(src,
                         send_retry_count_,
                         "send_retry_count",
                         sst::bigint(0));
    try {
      if (send_retry_count_ < 0) {
        throw sst::json::exception(
            "value must be a nonnegative integer");
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("send_retry_count"));
    }
    done_send_retry_count_ = true;
  }
  return send_retry_count_;
}

sst::bigint const & stack_config_t::send_retry_count() const {
  SST_ASSERT((done_send_retry_count_));
  return send_retry_count_;
}

//----------------------------------------------------------------------
// send_retry_window
//----------------------------------------------------------------------

sst::bigint const &
stack_config_t::send_retry_window(nlohmann::json & src) {
  if (!done_send_retry_window_) {
    sst::json::remove_to(src,
                         send_retry_window_,
                         "send_retry_window",
                         sst::bigint(20));
    try {
      if (send_retry_window_ < 0) {
        throw sst::json::exception(
            "value must be a nonnegative integer");
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("send_retry_window"));
    }
    done_send_retry_window_ = true;
  }
  return send_retry_window_;
}

sst::bigint const & stack_config_t::send_retry_window() const {
  SST_ASSERT((done_send_retry_window_));
  return send_retry_window_;
}

//----------------------------------------------------------------------
// send_timeout
//----------------------------------------------------------------------

sst::bigint const & stack_config_t::send_timeout(nlohmann::json & src) {
  if (!done_send_timeout_) {
    sst::json::remove_to(src,
                         send_timeout_,
                         "send_timeout",
                         sst::bigint(600));
    try {
      if (send_timeout_ < 0) {
        throw sst::json::exception(
            "value must be a nonnegative integer");
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("send_timeout"));
    }
    done_send_timeout_ = true;
  }
  return send_timeout_;
}

sst::bigint const & stack_config_t::send_timeout() const {
  SST_ASSERT((done_send_timeout_));
  return send_timeout_;
}

//----------------------------------------------------------------------

stack_config_t stack_config_t::from_json(nlohmann::json src) {
  stack_config_t dst;

  dst.allow_multicast(src);
  dst.num_clients(src);
  dst.num_servers(src);
  dst.send_retry_count(src);
  dst.send_retry_window(src);
  dst.send_timeout(src);

  // TODO: These should be converted to the above form.
#define F(x) sst::json::get_to(src, dst.x, #x)
  F(prime);
  F(secret_key_length);
  F(threshold);
  F(leader_relay_only);
  F(log_level);
  F(inter_server_direct_only);
  F(mix_size);
#undef F

  try {
    if (sst::perfect_lt(dst.mix_size, 4)) {
      throw sst::json::exception("value must be at least 4");
    }
    auto const x = sst::floor_sqrt(dst.mix_size);
    if (x * x != dst.mix_size) {
      throw sst::json::exception("value must be a square");
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("mix_size"));
  }

  return dst;
}

} // namespace kestrel
