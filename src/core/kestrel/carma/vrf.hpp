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

#ifndef KESTREL_CARMA_VRF_HPP
#define KESTREL_CARMA_VRF_HPP

#include <array>
#include <stdexcept>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/data.hpp>
#include <sst/catalog/is_byte.hpp>
#include <sst/catalog/is_contiguous.hpp>
#include <sst/catalog/is_iterable.hpp>
#include <sst/catalog/size.hpp>
#include <sst/catalog/unsigned_eq.hpp>
#include <sst/catalog/value_type_t.hpp>

#include <kestrel/sodium.hpp>
#include <sodium.h>

//
// TODO: This whole file will eventually be replaced by vrf_shell_t and
//       friends.
//

namespace kestrel {
namespace carma {
namespace vrf {

SST_NODISCARD()
constexpr decltype(+crypto_vrf_PUBLICKEYBYTES) pk_size() noexcept {
  return crypto_vrf_PUBLICKEYBYTES;
}

SST_NODISCARD()
constexpr decltype(+crypto_vrf_SECRETKEYBYTES) sk_size() noexcept {
  return crypto_vrf_SECRETKEYBYTES;
}

SST_NODISCARD()
constexpr decltype(+crypto_vrf_OUTPUTBYTES) output_size() noexcept {
  return crypto_vrf_OUTPUTBYTES;
}

SST_NODISCARD()
constexpr decltype(+crypto_vrf_PROOFBYTES) proof_size() noexcept {
  return crypto_vrf_PROOFBYTES;
}

template<class Pk, class Sk>
void keygen(Pk & pk, Sk & sk) {
  SST_STATIC_ASSERT((sst::is_iterable<Pk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Sk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Pk>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Sk>::value));
  SST_ASSERT((sst::unsigned_eq(sst::size(pk), pk_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(sk), sk_size())));
  int const s = sodium::crypto_vrf_keypair(
      reinterpret_cast<unsigned char *>(sst::data(pk)),
      reinterpret_cast<unsigned char *>(sst::data(sk)));
  if (s != 0) {
    throw std::runtime_error("crypto_vrf_keypair() failed");
  }
}

template<class Sk, class Input, class Proof>
void eval(Sk const & sk, Input const & input, Proof & proof) {
  SST_STATIC_ASSERT((sst::is_iterable<Sk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Input, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Proof, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Sk>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Input>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Proof>::value));
  SST_ASSERT((sst::unsigned_eq(sst::size(sk), sk_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(proof), proof_size())));
  int const s = sodium::crypto_vrf_prove(
      reinterpret_cast<unsigned char *>(sst::data(proof)),
      reinterpret_cast<unsigned char const *>(sst::data(sk)),
      reinterpret_cast<unsigned char const *>(sst::data(input)),
      sst::checked_cast<unsigned long long>(sst::size(input)));
  if (s != 0) {
    throw std::runtime_error("crypto_vrf_prove() failed");
  }
}

template<class Sk, class Input, class Proof, class Output>
void eval(Sk const & sk,
          Input const & input,
          Proof & proof,
          Output & output) {
  SST_STATIC_ASSERT((sst::is_iterable<Sk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Input, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Proof, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Output, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Sk>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Input>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Proof>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Output>::value));
  SST_ASSERT((sst::unsigned_eq(sst::size(sk), sk_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(proof), proof_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(output), output_size())));
  eval(sk, input, proof);
  int const s = sodium::crypto_vrf_proof_to_hash(
      reinterpret_cast<unsigned char *>(sst::data(output)),
      reinterpret_cast<unsigned char const *>(sst::data(proof)));
  if (s != 0) {
    throw std::runtime_error("crypto_vrf_proof_to_hash() failed");
  }
}

template<class Pk, class Input, class Proof, class Output>
SST_NODISCARD()
bool verify(Pk const & pk,
            Input const & input,
            Proof const & proof,
            Output & output) {
  SST_STATIC_ASSERT((sst::is_iterable<Pk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Input, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Proof, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Output, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Pk>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Input>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Proof>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Output>::value));
  SST_ASSERT((sst::unsigned_eq(sst::size(pk), pk_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(proof), proof_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(output), output_size())));
  int const s = sodium::crypto_vrf_verify(
      reinterpret_cast<unsigned char *>(sst::data(output)),
      reinterpret_cast<unsigned char const *>(sst::data(pk)),
      reinterpret_cast<unsigned char const *>(sst::data(proof)),
      reinterpret_cast<unsigned char const *>(sst::data(input)),
      sst::checked_cast<unsigned long long>(sst::size(input)));
  return s == 0;
}

template<class Pk, class Input, class Proof>
SST_NODISCARD()
bool verify(Pk const & pk, Input const & input, Proof const & proof) {
  SST_STATIC_ASSERT((sst::is_iterable<Pk, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Input, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_iterable<Proof, sst::is_byte>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Pk>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Input>::value));
  SST_STATIC_ASSERT((sst::is_contiguous<Proof>::value));
  SST_ASSERT((sst::unsigned_eq(sst::size(pk), pk_size())));
  SST_ASSERT((sst::unsigned_eq(sst::size(proof), proof_size())));
  std::array<unsigned char, crypto_vrf_OUTPUTBYTES> output;
  return verify(pk, input, proof, output);
}

} // namespace vrf
} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_VRF_HPP
