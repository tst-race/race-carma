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
#include <kestrel/carma/phonebook_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_entry_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_default_hash_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------
// Config traversal
//----------------------------------------------------------------------

void phonebook_entry_t::set_phonebook(phonebook_t & src) noexcept {
  SST_ASSERT((!moved_from_));
  phonebook_ = &src;
}

phonebook_t & phonebook_entry_t::phonebook() noexcept {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((phonebook_));
  return *phonebook_;
}

phonebook_t const & phonebook_entry_t::phonebook() const noexcept {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((phonebook_));
  return *phonebook_;
}

config_t & phonebook_entry_t::config() noexcept {
  SST_ASSERT((!moved_from_));
  return phonebook().config();
}

config_t const & phonebook_entry_t::config() const noexcept {
  SST_ASSERT((!moved_from_));
  return phonebook().config();
}

global_config_t & phonebook_entry_t::global() noexcept {
  SST_ASSERT((!moved_from_));
  return config().global();
}

global_config_t const & phonebook_entry_t::global() const noexcept {
  SST_ASSERT((!moved_from_));
  return config().global();
}

local_config_t & phonebook_entry_t::local() noexcept {
  SST_ASSERT((!moved_from_));
  return config().local();
}

local_config_t const & phonebook_entry_t::local() const noexcept {
  SST_ASSERT((!moved_from_));
  return config().local();
}

//----------------------------------------------------------------------
// psn
//----------------------------------------------------------------------

void phonebook_entry_t::parse_psn(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (psn_) {
    return;
  }
  psn_.emplace();
  sst::json::remove_to(src, *psn_, "psn");
}

void phonebook_entry_t::unparse_psn(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((psn_));
  dst["psn"] = *psn_;
}

psn_t const & phonebook_entry_t::psn() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((psn_));
  return *psn_;
}

void phonebook_entry_t::set_psn(psn_t src) {
  SST_ASSERT((!moved_from_));
  psn_.emplace(std::move(src));
}

//----------------------------------------------------------------------
// psn_hash
//----------------------------------------------------------------------

psn_default_hash_t const & phonebook_entry_t::psn_hash() const {
  SST_ASSERT((!moved_from_));
  psn_default_hash_t * val = psn_hash_.load();
  if (!val) {
    sst::unique_ptr<psn_default_hash_t> new_val{sst::in_place, psn()};
    psn_hash_.compare_exchange_strong(val, std::move(new_val));
  }
  return *val;
}

//----------------------------------------------------------------------
// Anonymous public-key encryption
//----------------------------------------------------------------------

bytes_t
phonebook_entry_t::anon_encrypt(bytes_t const & plaintext) const {
  SST_ASSERT((!moved_from_));
  return global().anon_encrypt(plaintext, pk());
}

//----------------------------------------------------------------------
// Authenticated public-key encryption
//----------------------------------------------------------------------

bytes_t
phonebook_entry_t::auth_encrypt(bytes_t const & plaintext) const {
  SST_ASSERT((!moved_from_));
  return global().auth_encrypt(plaintext,
                               local().pk(),
                               local().sk(),
                               pk());
}

bytes_t
phonebook_entry_t::auth_decrypt(bytes_t const & ciphertext) const {
  SST_ASSERT((!moved_from_));
  return global().auth_decrypt(ciphertext,
                               local().pk(),
                               local().sk(),
                               pk());
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
