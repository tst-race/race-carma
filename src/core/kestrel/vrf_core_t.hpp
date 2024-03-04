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

#ifndef KESTREL_VRF_CORE_T_HPP
#define KESTREL_VRF_CORE_T_HPP

#include <cstddef>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

class vrf_core_t {

public:

  virtual std::size_t pk_size() const = 0;

  virtual std::size_t sk_size() const = 0;

  virtual std::size_t output_size() const = 0;

  virtual std::size_t proof_size() const = 0;

  virtual void keygen(unsigned char * pk, unsigned char * sk) const = 0;

  virtual void eval(unsigned char const * sk,
                    unsigned char const * input,
                    std::size_t size,
                    unsigned char * proof,
                    unsigned char * output) const = 0;

  virtual bool verify(unsigned char const * pk,
                      unsigned char const * input,
                      std::size_t size,
                      unsigned char const * proof,
                      unsigned char const * output) const = 0;

  virtual sst::unique_ptr<vrf_core_t> clone() const = 0;

  virtual json_t params() const = 0;

  virtual ~vrf_core_t() SST_NOEXCEPT(true) = default;

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_VRF_CORE_T_HPP
