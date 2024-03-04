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

#ifndef KESTREL_VRF_SHELL_T_HPP
#define KESTREL_VRF_SHELL_T_HPP

#include <cstddef>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/promote_unsigned_t.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/vrf_core_t.hpp>
#include <kestrel/vrf_eval_result_t.hpp>

namespace kestrel {

class vrf_shell_t final {

  sst::unique_ptr<vrf_core_t> core_ = nullptr;

public:

  using size_type = sst::promote_unsigned_t<std::size_t>;

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

protected:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  vrf_shell_t() SST_NOEXCEPT(true) = default;

  vrf_shell_t(vrf_shell_t const & other) {
    SST_ASSERT((other.core_ != nullptr));
    core_ = other.core_->clone();
  }

  vrf_shell_t & operator=(vrf_shell_t const & other) {
    SST_ASSERT((other.core_ != nullptr));
    core_ = other.core_->clone();
    return *this;
  }

  vrf_shell_t(vrf_shell_t &&) SST_NOEXCEPT(true) = default;

  vrf_shell_t & operator=(vrf_shell_t &&) SST_NOEXCEPT(true) = default;

  ~vrf_shell_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Non-default constructors
  //--------------------------------------------------------------------

public:

  explicit vrf_shell_t(sst::unique_ptr<vrf_core_t> core)
      : core_(std::move(core)) {
  }

  //--------------------------------------------------------------------
  // keygen
  //--------------------------------------------------------------------

public:

  void keygen(std::vector<unsigned char> & pk,
              std::vector<unsigned char> & sk) const;

  //--------------------------------------------------------------------
  // output_size
  //--------------------------------------------------------------------

public:

  size_type output_size() const;

  //--------------------------------------------------------------------
  // pk_buffer
  //--------------------------------------------------------------------
  //
  // Returns a zero vector of the appropriate size to be used as a pk
  // parameter.
  //

public:

  std::vector<unsigned char> pk_buffer() const;

  //--------------------------------------------------------------------
  // pk_size
  //--------------------------------------------------------------------

public:

  size_type pk_size() const;

  //--------------------------------------------------------------------
  // proof_size
  //--------------------------------------------------------------------

public:

  size_type proof_size() const;

  //--------------------------------------------------------------------
  // sk_buffer
  //--------------------------------------------------------------------
  //
  // Returns a zero vector of the appropriate size to be used as an sk
  // parameter.
  //

public:

  std::vector<unsigned char> sk_buffer() const;

  //--------------------------------------------------------------------
  // sk_size
  //--------------------------------------------------------------------

public:

  size_type sk_size() const;

  //--------------------------------------------------------------------
  // ticket_eval
  //--------------------------------------------------------------------

public:

  vrf_eval_result_t
  ticket_eval(std::vector<unsigned char> const & sk,
              std::vector<unsigned char> const & input) const;

  //--------------------------------------------------------------------
  // ticket_input
  //--------------------------------------------------------------------

public:

  static std::vector<unsigned char>
  ticket_input(std::vector<unsigned char> const & epoch_nonce);

  //--------------------------------------------------------------------
  // verify
  //--------------------------------------------------------------------

public:

  bool verify(std::vector<unsigned char> const & pk,
              std::vector<unsigned char> const & input,
              std::vector<unsigned char> const & proof,
              std::vector<unsigned char> const & output) const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, vrf_shell_t & dst);

  friend void to_json(json_t & dst, vrf_shell_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_VRF_SHELL_T_HPP
