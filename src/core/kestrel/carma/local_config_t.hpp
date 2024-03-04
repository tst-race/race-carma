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

#ifndef KESTREL_CARMA_LOCAL_CONFIG_T_HPP
#define KESTREL_CARMA_LOCAL_CONFIG_T_HPP

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/cbegin.hpp>
#include <sst/catalog/cend.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/is_byte.hpp>
#include <sst/catalog/is_integer_ish.hpp>
#include <sst/catalog/is_iterable.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/mutex.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/span.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_eq_t.hpp>
#include <kestrel/carma/phonebook_pair_hash_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/phonebook_vector_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/carma/vrf.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/vrf_eval_result_t.hpp>

namespace kestrel {
namespace carma {

class local_config_t final : public phonebook_entry_t {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  local_config_t() = default;

  local_config_t(local_config_t const &) = default;

  local_config_t & operator=(local_config_t const &) = default;

  local_config_t(local_config_t &&) SST_NOEXCEPT(true) = default;

  local_config_t & operator=(local_config_t &&)
      SST_NOEXCEPT(true) = default;

  ~local_config_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // clear_deducible
  //--------------------------------------------------------------------

public:

  void clear_deducible() const final;

  //--------------------------------------------------------------------
  // dice_roll
  //--------------------------------------------------------------------

public:

  template<class Modulus>
  struct dice_roll_result {
    std::vector<unsigned char> proof;
    std::vector<unsigned char> output;
    Modulus residue;
  };

  template<
      class Prefix,
      class Input,
      class Modulus,
      sst::enable_if_t<sst::is_iterable<Prefix, sst::is_byte>::value
                       && sst::is_iterable<Input, sst::is_byte>::value
                       && sst::is_integer_ish<Modulus>::value> = 0>
  dice_roll_result<Modulus> dice_roll(Prefix const & prefix,
                                      Input const & input,
                                      Modulus const & modulus) {
    std::vector<unsigned char> v;
    v.insert(v.end(), sst::cbegin(prefix), sst::cend(prefix));
    v.insert(v.end(), sst::cbegin(input), sst::cend(input));
    std::vector<unsigned char> proof;
    sst::checked_resize(proof, vrf::proof_size());
    std::vector<unsigned char> output;
    sst::checked_resize(output, vrf::output_size());
    vrf::eval(vrf_sk(), v, proof, output);
    sst::bigint n;
    n.set_from_bytes(output.data(),
                     output.size(),
                     sst::integer_rep::pure_unsigned());
    Modulus residue = static_cast<Modulus>(n % modulus);
    return {std::move(proof), std::move(output), std::move(residue)};
  }

  template<class Input,
           class Modulus,
           sst::enable_if_t<sst::is_iterable<Input, sst::is_byte>::value
                            && sst::is_integer_ish<Modulus>::value> = 0>
  dice_roll_result<Modulus> dice_roll(unsigned char const prefix,
                                      Input const & input,
                                      Modulus const & modulus) {
    return dice_roll(sst::span(&prefix, 1), input, modulus);
  }

  //--------------------------------------------------------------------
  // mb_servers
  //--------------------------------------------------------------------
  //
  // For an MC leader, returns the set of MB servers that it might
  // receive packages from.
  //

private:

  mutable sst::unique_ptr<phonebook_set_t> mb_servers_;

  void parse_mb_servers(json_t & src);

  void unparse_mb_servers(json_t & dst) const;

public:

  phonebook_set_t const & mb_servers(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // mc_group
  //--------------------------------------------------------------------

private:

  mutable sst::unique_ptr<
      std::unordered_map<node_count_t, phonebook_vector_t>>
      mc_groups_;

  mutable sst::mutex mc_groups_mutex_;

  void clear_mc_groups() const;

  void parse_mc_groups(json_t & src);

  void unparse_mc_groups(json_t & dst) const;

public:

  // Returns the MC group for a given MC group number.
  phonebook_vector_t const & mc_group(tracing_event_t tev,
                                      node_count_t group) const;

  // Returns the MC group for a given MC server.
  phonebook_vector_t const &
  mc_group(tracing_event_t tev, phonebook_entry_t const & server) const;

  // Returns the MC group for a given MC server.
  phonebook_vector_t const &
  mc_group(tracing_event_t tev, phonebook_pair_t const & server) const;

  // Returns the MC group for a given MC server.
  phonebook_vector_t const & mc_group(tracing_event_t tev,
                                      psn_t const & server) const;

  // Returns the MC group for this MC server.
  phonebook_vector_t const & mc_group(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // mc_leader
  //--------------------------------------------------------------------
  //
  // For an MC follower, returns its MC leader.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_pair_t const *>>
      mc_leader_;

  void clear_mc_leader() const;

  void parse_mc_leader(json_t & src);

  void unparse_mc_leader(json_t & dst) const;

public:

  phonebook_pair_t const & mc_leader(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // next_layer
  //--------------------------------------------------------------------
  //
  // For an MC leader or RS server, returns the set of servers it might
  // send packages to as part of downstream routing.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_set_t>> next_layer_;

  void clear_next_layer() const;

  void parse_next_layer(json_t & src);

  void unparse_next_layer(json_t & dst) const;

public:

  phonebook_set_t const & next_layer(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // other_servers
  //--------------------------------------------------------------------
  //
  // For an idle server, returns the set of servers it might
  // send/receive packages to/from as part of the idle server protocol.
  //
  // For a non-idle server, returns the set of idle servers it might
  // send/receive packages to/from as part of the idle server protocol.
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  mutable sst::atomic<sst::unique_ptr<phonebook_set_t>> other_servers_;

  void parse_other_servers(json_t & src);

  void unparse_other_servers(json_t & src) const;

public:

  phonebook_set_t const & other_servers() const;

  phonebook_set_t & mutable_other_servers();

  //--------------------------------------------------------------------
  // prev_layer
  //--------------------------------------------------------------------
  //
  // For an RS server or MB server, returns the set of servers it might
  // receive packages from as part of downstream routing.
  //

private:

  mutable sst::unique_ptr<phonebook_set_t> prev_layer_;

  void parse_prev_layer(json_t & src);

  void unparse_prev_layer(json_t & dst) const;

public:

  phonebook_set_t const & prev_layer(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // rx_nodes
  //--------------------------------------------------------------------
  //
  // Returns the set of nodes this node might receive messages from.
  //

private:

  mutable sst::unique_ptr<phonebook_set_t> rx_nodes_;

  void parse_rx_nodes(json_t & src);

  void unparse_rx_nodes(json_t & dst) const;

public:

  phonebook_set_t const & rx_nodes(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // sk
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> sk_{sst::in_place};
  bool have_sk_ = false;

  void parse_sk(json_t & src);

public:

  std::vector<unsigned char> const & sk() const;

  local_config_t & sk(std::vector<unsigned char> const & src);

  //--------------------------------------------------------------------
  // ticket
  //--------------------------------------------------------------------

public:

  vrf_eval_result_t const & ticket() const final;

  //--------------------------------------------------------------------
  // tx_nodes
  //--------------------------------------------------------------------
  //
  // Returns the set of nodes this node might send messages to.
  //

private:

  mutable sst::unique_ptr<phonebook_set_t> tx_nodes_;

  mutable sst::mutex tx_nodes_mutex_;

  void clear_tx_nodes() const;

  void parse_tx_nodes(json_t & src);

  void unparse_tx_nodes(json_t & dst) const;

public:

  phonebook_set_t const & tx_nodes(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // vrf_sk
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> vrf_sk_;

  void parse_vrf_sk(json_t & src);

  void unparse_vrf_sk(json_t & dst) const;

public:

  std::vector<unsigned char> const & vrf_sk() const;

  void set_vrf_sk(std::vector<unsigned char> src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, local_config_t & dst);

  friend void to_json(json_t & dst, local_config_t const & src);

  //--------------------------------------------------------------------
  // Anonymous public-key encryption
  //--------------------------------------------------------------------

public:

  bytes_t anon_decrypt(bytes_t const & ciphertext) const;

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_LOCAL_CONFIG_T_HPP
