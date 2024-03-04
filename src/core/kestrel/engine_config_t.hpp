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

#ifndef KESTREL_ENGINE_CONFIG_T_HPP
#define KESTREL_ENGINE_CONFIG_T_HPP

#include <memory>
#include <string>
#include <vector>

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/role_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class engine_config_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  engine_config_t() = default;

  ~engine_config_t() SST_NOEXCEPT(true) = default;

  engine_config_t(engine_config_t const &) = default;

  engine_config_t & operator=(engine_config_t const &) = default;

  engine_config_t(engine_config_t &&) SST_NOEXCEPT(true) = default;

  engine_config_t & operator=(engine_config_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // parse_state_t
  //--------------------------------------------------------------------
  //
  // Keeps track of which parse_*() functions have already been called
  // during a from_json() call.
  //

private:

  struct parse_state_t {
    bool called_parse_clients_ = false;
    bool called_parse_psn_ = false;
    bool called_parse_role_ = false;
  };

  //--------------------------------------------------------------------
  // clients
  //--------------------------------------------------------------------
  //
  // The list of all known clients, including this node if this node is
  // a client. The list is sorted and has no duplicates. It is possible
  // that the list is empty.
  //
  // It is safe to call clients() from any thread at any time. It is
  // safe to read the returned list from any thread at any time.
  //
  // A list returned by clients() will never change. clients() must be
  // called again to possibly observe any change.
  //
  // It is safe to call set_clients() from any thread at any time.
  //

private:

  sst::atomic<std::shared_ptr<std::vector<psn_t> const>> clients_{
      std::make_shared<std::vector<psn_t> const>()};

  void parse_clients(json_t & src, parse_state_t & state);

  void unparse_clients(json_t & dst) const;

public:

  SST_NODISCARD()
  std::shared_ptr<std::vector<psn_t> const> clients() const;

  void set_clients(std::vector<psn_t> src);

  //--------------------------------------------------------------------
  // psn
  //--------------------------------------------------------------------
  //
  // The PSN of this node.
  //
  // It is safe to call psn() from any thread at any time after the
  // initial call to set_psn(), and it is safe to read the returned
  // object from any thread at any time.
  //
  // It is safe to call set_psn() from any thread at any time.
  //

private:

  sst::atomic<std::shared_ptr<psn_t const>> psn_;

  void parse_psn(json_t & src, parse_state_t & state);

  void unparse_psn(json_t & dst) const;

public:

  SST_NODISCARD() std::shared_ptr<psn_t const> psn() const;

  void set_psn(psn_t src);

  //--------------------------------------------------------------------
  // role
  //--------------------------------------------------------------------
  //
  // The role of this node.
  //
  // It is safe to call role() from any thread at any time after the
  // initial call to set_role(), and it is safe to read the returned
  // object from any thread at any time.
  //
  // It is safe to call set_role() from any thread at any time.
  //

private:

  sst::atomic<std::shared_ptr<role_t const>> role_;

  void parse_role(json_t & src, parse_state_t & state);

  void unparse_role(json_t & dst) const;

public:

  SST_NODISCARD() std::shared_ptr<role_t const> role() const;

  void set_role(role_t src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, engine_config_t & dst);

  friend void to_json(json_t & dst, engine_config_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_ENGINE_CONFIG_T_HPP
