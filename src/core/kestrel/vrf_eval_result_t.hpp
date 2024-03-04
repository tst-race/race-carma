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

#ifndef KESTREL_VRF_EVAL_RESULT_T_HPP
#define KESTREL_VRF_EVAL_RESULT_T_HPP

#include <vector>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

class vrf_eval_result_t final {

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  vrf_eval_result_t() = default;

  vrf_eval_result_t(vrf_eval_result_t const &) = default;

  vrf_eval_result_t & operator=(vrf_eval_result_t const &) = default;

  vrf_eval_result_t(vrf_eval_result_t &&) SST_NOEXCEPT(true) = default;

  vrf_eval_result_t & operator=(vrf_eval_result_t &&)
      SST_NOEXCEPT(true) = default;

  ~vrf_eval_result_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // output
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> output_{sst::in_place};

  bool done_output_ = false;

  void parse_output(json_t & src);

public:

  std::vector<unsigned char> const & output() const;

  vrf_eval_result_t & output(std::vector<unsigned char> src);

  //--------------------------------------------------------------------
  // proof
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> proof_{sst::in_place};

  bool done_proof_ = false;

  void parse_proof(json_t & src);

public:

  std::vector<unsigned char> const & proof() const;

  vrf_eval_result_t & proof(std::vector<unsigned char> src);

  //--------------------------------------------------------------------
  // verified
  //--------------------------------------------------------------------

private:

  sst::optional<bool> verified_;

  void parse_verified(json_t & src);

  void unparse_verified(json_t & dst) const;

public:

  bool verified() const;

  void set_verified(bool src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, vrf_eval_result_t & dst);

  friend void to_json(json_t & dst, vrf_eval_result_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_VRF_EVAL_RESULT_T_HPP
