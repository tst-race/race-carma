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

#ifndef KESTREL_STACK_SPEC_T_HPP
#define KESTREL_STACK_SPEC_T_HPP

#include <vector>

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/cluster_spec_t.hpp>
#include <kestrel/cluster_vector_t.hpp>
#include <kestrel/genesis_spec_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

class stack_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  stack_spec_t() SST_NOEXCEPT(true) = default;

  ~stack_spec_t() SST_NOEXCEPT(true) = default;

  stack_spec_t(stack_spec_t const &) = default;

  stack_spec_t & operator=(stack_spec_t const &) = default;

  stack_spec_t(stack_spec_t &&) SST_NOEXCEPT(true) = default;

  stack_spec_t & operator=(stack_spec_t &&)
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
    bool called_parse_clusters_ = false;
    bool called_parse_genesis_ = false;
  };

  //--------------------------------------------------------------------
  // clusters
  //--------------------------------------------------------------------

private:

  cluster_vector_t clusters_;

  void parse_clusters(json_t & src, parse_state_t & state);

  void unparse_clusters(json_t & dst) const;

public:

  SST_NODISCARD() std::vector<cluster_spec_t> & clusters();

  SST_NODISCARD() std::vector<cluster_spec_t> const & clusters() const;

  //--------------------------------------------------------------------
  // genesis
  //--------------------------------------------------------------------

private:

  sst::optional<genesis_spec_t> genesis_;

  void parse_genesis(json_t & src, parse_state_t & state);

  void unparse_genesis(json_t & dst) const;

public:

  SST_NODISCARD() sst::optional<genesis_spec_t> & genesis();

  SST_NODISCARD() sst::optional<genesis_spec_t> const & genesis() const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, stack_spec_t & dst);

  friend void to_json(json_t & dst, stack_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_STACK_SPEC_T_HPP
