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

#ifndef KESTREL_CARMA_PHONEBOOK_ENTRY_T_HPP
#define KESTREL_CARMA_PHONEBOOK_ENTRY_T_HPP

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_pair_eq_t.hpp>
#include <kestrel/carma/phonebook_pair_hash_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_default_hash_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/vrf_eval_result_t.hpp>

namespace kestrel {
namespace carma {

class config_t;
class global_config_t;
class local_config_t;
class phonebook_t;

class phonebook_entry_t {

  friend class phonebook_t;

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

protected:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Config traversal
  //--------------------------------------------------------------------

private:

  phonebook_t * phonebook_ = nullptr;

public:

  void set_phonebook(phonebook_t & src) noexcept;

  phonebook_t & phonebook() noexcept;

  phonebook_t const & phonebook() const noexcept;

  config_t & config() noexcept;

  config_t const & config() const noexcept;

  global_config_t & global() noexcept;

  global_config_t const & global() const noexcept;

  local_config_t & local() noexcept;

  local_config_t const & local() const noexcept;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  phonebook_entry_t() = default;

  phonebook_entry_t(phonebook_entry_t const &) = default;

  phonebook_entry_t & operator=(phonebook_entry_t const &) = default;

  phonebook_entry_t(phonebook_entry_t &&) SST_NOEXCEPT(true) = default;

  phonebook_entry_t & operator=(phonebook_entry_t &&)
      SST_NOEXCEPT(true) = default;

  virtual ~phonebook_entry_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Non-default constructors
  //--------------------------------------------------------------------

public:

  explicit phonebook_entry_t(psn_t psn) {
    this->set_psn(std::move(psn));
  }

  //--------------------------------------------------------------------
  // bucket
  //--------------------------------------------------------------------
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<node_count_t> bucket_{node_count_max()};

  void clear_bucket() const;

  void parse_bucket(json_t & src);

  void unparse_bucket(json_t & dst) const;

public:

  SST_NODISCARD() node_count_t bucket(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // bucket_clients
  //--------------------------------------------------------------------
  //
  // For a client or MB server, returns the set of clients that are in
  // its bucket.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_set_t>> bucket_clients_;

  void clear_bucket_clients() const;

  void parse_bucket_clients(json_t & src);

  void unparse_bucket_clients(json_t & dst) const;

public:

  SST_NODISCARD()
  phonebook_set_t const & bucket_clients(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // bucket_mb_servers
  //--------------------------------------------------------------------
  //
  // For a client or MB server, returns the set of MB servers that are
  // in its bucket.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_set_t>>
      bucket_mb_servers_;

  void clear_bucket_mb_servers() const;

  void parse_bucket_mb_servers(json_t & src);

  void unparse_bucket_mb_servers(json_t & dst) const;

public:

  SST_NODISCARD()
  phonebook_set_t const & bucket_mb_servers(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // clear_deducible
  //--------------------------------------------------------------------

public:

  virtual void clear_deducible() const;

  //--------------------------------------------------------------------
  // group
  //--------------------------------------------------------------------
  //
  // For an MC server, returns its MC group number.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

protected:

  sst::atomic<sst::optional<node_count_t>> group_;

  void parse_group(json_t & src);

  void unparse_group(json_t & dst) const;

public:

  node_count_t group() const;

  void set_group(node_count_t src);

  //--------------------------------------------------------------------
  // mc_leaders
  //--------------------------------------------------------------------
  //
  // For an MB server, returns the set of MC leaders it might send
  // packages to as part of upstream routing.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //
  // TODO: Should we make this a phonebook_vector_t? It seems like we
  //       want random access most of the time. We'd just have to be
  //       careful to deal with duplicates when parsing.
  // TODO: Yes, we should make this a vector, and we should also have a
  //       function called random_mc_leader(), as that's needed in
  //       multiple places.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_set_t>> mc_leaders_;

  void clear_mc_leaders() const;

  void parse_mc_leaders(json_t & src);

  void unparse_mc_leaders(json_t & dst) const;

public:

  phonebook_set_t const & mc_leaders(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // order
  //--------------------------------------------------------------------
  //
  // For an MC server, returns its index in its MC group. The MC leader
  // always has order 0.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

protected:

  sst::atomic<sst::optional<node_count_t>> order_;

  void parse_order(json_t & src);

  void unparse_order(json_t & dst) const;

public:

  node_count_t order() const;

  void set_order(node_count_t src);

  //--------------------------------------------------------------------
  // parse_phonebook_set
  //--------------------------------------------------------------------

protected:

  template<class Dst>
  void parse_phonebook_set(json_t & src,
                           char const * const key,
                           Dst & dst,
                           bool required = false) {
    SST_ASSERT((!moved_from_));
    kestrel::carma::parse_phonebook_set(phonebook(),
                                        src,
                                        key,
                                        dst,
                                        required);
  }

  //--------------------------------------------------------------------
  // pk
  //--------------------------------------------------------------------

protected:

  sst::unique_ptr<std::vector<unsigned char>> pk_{sst::in_place};
  bool have_pk_ = false;

  void parse_pk(json_t & src);

public:

  std::vector<unsigned char> const & pk() const;

  phonebook_entry_t & pk(std::vector<unsigned char> const & src);

  //--------------------------------------------------------------------
  // psn
  //--------------------------------------------------------------------
  //
  // Returns this node's PSN.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

protected:

  sst::unique_ptr<psn_t> psn_;

  void parse_psn(json_t & src);

  void unparse_psn(json_t & dst) const;

public:

  SST_NODISCARD() psn_t const & psn() const;

  void set_psn(psn_t src);

  //--------------------------------------------------------------------
  // psn_hash
  //--------------------------------------------------------------------
  //
  // Returns the hash of this node's PSN.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<sst::unique_ptr<psn_default_hash_t>> psn_hash_;

public:

  SST_NODISCARD() psn_default_hash_t const & psn_hash() const;

  //--------------------------------------------------------------------
  // role
  //--------------------------------------------------------------------
  //
  // Returns this node's role.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

protected:

  sst::atomic<sst::optional<role_t>> role_;

  void parse_role(json_t & src);

  void unparse_role(json_t & dst) const;

public:

  role_t role() const;

  void set_role(role_t src);

  //--------------------------------------------------------------------
  // ticket
  //--------------------------------------------------------------------

protected:

  mutable sst::unique_ptr<vrf_eval_result_t> ticket_;

  void parse_ticket(json_t & src);

  void unparse_ticket(json_t & dst) const;

public:

  virtual vrf_eval_result_t const & ticket() const;

  //--------------------------------------------------------------------
  // vrf_pk
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> vrf_pk_;

  void parse_vrf_pk(json_t & src);

  void unparse_vrf_pk(json_t & dst) const;

public:

  std::vector<unsigned char> const & vrf_pk() const;

  void set_vrf_pk(std::vector<unsigned char> src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

protected:

  void from_json_core(json_t & src);

  void to_json_core(json_t & dst) const;

public:

  friend void from_json(json_t src, phonebook_entry_t & dst);

  friend void to_json(json_t & dst, phonebook_entry_t const & src);

  //--------------------------------------------------------------------
  // Anonymous public-key encryption
  //--------------------------------------------------------------------

public:

  bytes_t anon_encrypt(bytes_t const & plaintext) const;

  //--------------------------------------------------------------------
  // Authenticated public-key encryption
  //--------------------------------------------------------------------

public:

  bytes_t auth_encrypt(bytes_t const & plaintext) const;

  bytes_t auth_decrypt(bytes_t const & ciphertext) const;

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_PHONEBOOK_ENTRY_T_HPP
