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

#ifndef KESTREL_VRF_ALGORAND_T_HPP
#define KESTREL_VRF_ALGORAND_T_HPP

#include <cstddef>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/vrf_core_t.hpp>

namespace kestrel {

class vrf_algorand_t : public vrf_core_t {

public:

  vrf_algorand_t() SST_NOEXCEPT(true) = default;

  explicit vrf_algorand_t(json_t params);

  std::size_t pk_size() const final;

  std::size_t sk_size() const final;

  std::size_t output_size() const final;

  std::size_t proof_size() const final;

  void keygen(unsigned char * pk, unsigned char * sk) const final;

  void eval(unsigned char const * sk,
            unsigned char const * input,
            std::size_t size,
            unsigned char * proof,
            unsigned char * output) const final;

  bool verify(unsigned char const * pk,
              unsigned char const * input,
              std::size_t size,
              unsigned char const * proof,
              unsigned char const * output) const final;

  sst::unique_ptr<vrf_core_t> clone() const final;

  json_t params() const final;

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_VRF_ALGORAND_T_HPP
