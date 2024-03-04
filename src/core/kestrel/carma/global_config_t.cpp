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
#include <kestrel/carma/global_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/global_config_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/char_bit_v.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/to_unsigned.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/serialization.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------
// num_parallel_messages
//----------------------------------------------------------------------

unsigned long global_config_t::num_parallel_messages() const {
  SST_ASSERT((!moved_from_));
  using R = decltype(num_parallel_messages());
  return sst::checked_cast<R>(rangegen_row().num_parallel_messages);
}

//----------------------------------------------------------------------
// num_parallel_msgs_for_registration
//----------------------------------------------------------------------

unsigned long
global_config_t::num_parallel_msgs_for_registration() const {
  SST_ASSERT((!moved_from_));
  using R = decltype(num_parallel_msgs_for_registration());
  return sst::checked_cast<R>(num_parallel_messages());
}

//----------------------------------------------------------------------
// num_parallel_msgs_for_routing
//----------------------------------------------------------------------

unsigned long global_config_t::num_parallel_msgs_for_routing() const {
  SST_ASSERT((!moved_from_));
  using R = decltype(num_parallel_msgs_for_routing());
  return sst::checked_cast<R>(
      rangegen_row().num_parallel_msgs_for_routing);
}

//----------------------------------------------------------------------
// num_per_rs_layer
//----------------------------------------------------------------------

unsigned long global_config_t::num_per_rs_layer() const {
  SST_ASSERT((!moved_from_));
  using R = decltype(num_per_rs_layer());
  return sst::checked_cast<R>(rangegen_row().num_per_rs_layer);
}

//----------------------------------------------------------------------
// num_routing_layers
//----------------------------------------------------------------------

unsigned long global_config_t::num_routing_layers() const {
  SST_ASSERT((!moved_from_));
  using R = decltype(num_routing_layers());
  return sst::checked_cast<R>(rangegen_row().num_routing_layers);
}

//----------------------------------------------------------------------
// prime_space
//----------------------------------------------------------------------

unsigned long global_config_t::prime_space() const {
  auto const char_bit = sst::to_unsigned(sst::char_bit_v);
  auto const n = prime().num_bits<decltype(prime_space())>(
      sst::integer_rep::pure_unsigned());
  return (n - 1U) / char_bit;
}

//----------------------------------------------------------------------
// Anonymous public-key encryption
//----------------------------------------------------------------------

serialize_size_t
global_config_t::anon_encrypt_size(serialize_size_t const src) const {
  auto const a = pkc::anon_ciphertext_size(src.value);
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

serialize_size_t
global_config_t::anon_encrypt_size(bytes_t const & src) const {
  auto const a = pkc::anon_ciphertext_size(src.size());
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

bytes_t global_config_t::anon_encrypt(bytes_t const & plaintext,
                                      bytes_t const & recver_pk) const {
  return pkc::anon_encrypt(recver_pk, plaintext).first;
}

bytes_t global_config_t::anon_decrypt(bytes_t const & ciphertext,
                                      bytes_t const & recver_pk,
                                      bytes_t const & recver_sk) const {
  return pkc::anon_decrypt(recver_pk, recver_sk, ciphertext).first;
}

//----------------------------------------------------------------------
// Authenticated public-key encryption
//----------------------------------------------------------------------

serialize_size_t
global_config_t::auth_encrypt_size(serialize_size_t const src) const {
  auto const a = pkc::auth_ciphertext_size(src.value);
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

serialize_size_t
global_config_t::auth_encrypt_size(bytes_t const & src) const {
  auto const a = pkc::auth_ciphertext_size(src.size());
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

bytes_t global_config_t::auth_encrypt(bytes_t const & plaintext,
                                      bytes_t const & sender_pk,
                                      bytes_t const & sender_sk,
                                      bytes_t const & recver_pk) const {
  (void)sender_pk;
  return pkc::auth_encrypt(sender_sk, recver_pk, plaintext).first;
}

bytes_t global_config_t::auth_decrypt(bytes_t const & ciphertext,
                                      bytes_t const &,
                                      bytes_t const & recver_sk,
                                      bytes_t const & sender_pk) const {
  return pkc::auth_decrypt(sender_pk, recver_sk, ciphertext).first;
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
