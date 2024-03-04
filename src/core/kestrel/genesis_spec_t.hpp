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

#ifndef KESTREL_GENESIS_SPEC_T_HPP
#define KESTREL_GENESIS_SPEC_T_HPP

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

class genesis_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  genesis_spec_t() SST_NOEXCEPT(true) = default;

  ~genesis_spec_t() SST_NOEXCEPT(true) = default;

  genesis_spec_t(genesis_spec_t const &) = default;

  genesis_spec_t & operator=(genesis_spec_t const &) = default;

  genesis_spec_t(genesis_spec_t &&) SST_NOEXCEPT(true) = default;

  genesis_spec_t & operator=(genesis_spec_t &&)
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
    bool called_parse_num_clients_ = false;
    bool called_parse_num_servers_ = false;
  };

  //--------------------------------------------------------------------
  // num_clients
  //--------------------------------------------------------------------

private:

  sst::optional<long> num_clients_;

  void parse_num_clients(json_t & src, parse_state_t & state);

  void unparse_num_clients(json_t & dst) const;

public:

  SST_NODISCARD() long num_clients() const noexcept;

  //--------------------------------------------------------------------
  // num_servers
  //--------------------------------------------------------------------

private:

  sst::optional<long> num_servers_;

  void parse_num_servers(json_t & src, parse_state_t & state);

  void unparse_num_servers(json_t & dst) const;

public:

  SST_NODISCARD() long num_servers() const noexcept;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, genesis_spec_t & dst);

  friend void to_json(json_t & dst, genesis_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_GENESIS_SPEC_T_HPP
