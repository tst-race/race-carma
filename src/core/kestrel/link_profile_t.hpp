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

#ifndef KESTREL_LINK_PROFILE_T_HPP
#define KESTREL_LINK_PROFILE_T_HPP

#include <string>
#include <unordered_set>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/moved_from.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/link_role_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

class link_profile_t final {

  //--------------------------------------------------------------------
  // address
  //--------------------------------------------------------------------

private:

  bool have_address_ = false;
  std::string address_;

  std::string const & parse_address(json_t & src);

public:

  std::string const & address() const noexcept {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((have_address_));
    return address_;
  }

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------

public:

  link_profile_t() = default;

  //--------------------------------------------------------------------
  // copy-assign
  //--------------------------------------------------------------------

public:

  link_profile_t & operator=(link_profile_t const & other) = default;

  //--------------------------------------------------------------------
  // copy-construct
  //--------------------------------------------------------------------

public:

  link_profile_t(link_profile_t const & other) = default;

  //--------------------------------------------------------------------
  // description
  //--------------------------------------------------------------------

private:

  bool have_description_ = false;
  std::string description_;

  std::string const & parse_description(json_t & src);

public:

  std::string const & description() const noexcept {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((have_description_));
    return description_;
  }

  //--------------------------------------------------------------------
  // destruct
  //--------------------------------------------------------------------

public:

  ~link_profile_t() = default;

  //--------------------------------------------------------------------
  // from_json
  //--------------------------------------------------------------------

  friend void from_json(json_t const & src, link_profile_t & dst);

  //--------------------------------------------------------------------
  // move-assign
  //--------------------------------------------------------------------

public:

  link_profile_t & operator=(link_profile_t && other) = default;

  //--------------------------------------------------------------------
  // move-construct
  //--------------------------------------------------------------------

public:

  link_profile_t(link_profile_t && other) = default;

  //--------------------------------------------------------------------
  // moved_from_
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_{true};

  //--------------------------------------------------------------------
  // personas
  //--------------------------------------------------------------------

private:

  bool have_personas_ = false;
  std::unordered_set<psn_t> personas_;
  std::vector<std::string> personas_vector_;

  std::unordered_set<psn_t> const & parse_personas(json_t & src);

public:

  std::unordered_set<psn_t> const & personas() const noexcept {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((have_personas_));
    return personas_;
  }

  std::vector<std::string> const & personas_vector() const noexcept {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((have_personas_));
    return personas_vector_;
  }

  //--------------------------------------------------------------------
  // role
  //--------------------------------------------------------------------

private:

  bool have_role_ = false;
  link_role_t role_;

  link_role_t const & parse_role(json_t & src);

public:

  link_role_t const & role() const noexcept {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((have_role_));
    return role_;
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_LINK_PROFILE_T_HPP
