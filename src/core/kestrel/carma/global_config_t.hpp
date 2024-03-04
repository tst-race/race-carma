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

#ifndef KESTREL_CARMA_GLOBAL_CONFIG_T_HPP
#define KESTREL_CARMA_GLOBAL_CONFIG_T_HPP

#include <string>
#include <vector>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/rangegen/row.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/serialization.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/vrf_shell_t.hpp>

namespace kestrel {
namespace carma {

//
// global_config_t holds the persistent state that should be duplicated
// across all nodes. In particular, when a node X bootstraps a new node
// Y into the network, X will give Y a copy of its global config.
//

class global_config_t final {

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  global_config_t() = default;

  global_config_t(global_config_t const &) = default;

  global_config_t & operator=(global_config_t const &) = default;

  global_config_t(global_config_t &&) SST_NOEXCEPT(true) = default;

  global_config_t & operator=(global_config_t &&)
      SST_NOEXCEPT(true) = default;

  ~global_config_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Construction from a file
  //--------------------------------------------------------------------

public:

  explicit global_config_t(tracing_event_t tev,
                           common_sdk_t * sdk,
                           std::string const & file);

  //--------------------------------------------------------------------
  // clear_deducible
  //--------------------------------------------------------------------

public:

  void clear_deducible() const;

  //--------------------------------------------------------------------
  // correctness_test_only
  //--------------------------------------------------------------------

private:

  bool correctness_test_only_{};

  bool have_correctness_test_only_ = false;

  void parse_correctness_test_only(json_t & src);

public:

  bool correctness_test_only() const;

  global_config_t & correctness_test_only(bool src);

  //--------------------------------------------------------------------
  // corruption_rate
  //--------------------------------------------------------------------

private:

  double corruption_rate_{};

  bool have_corruption_rate_ = false;

  void parse_corruption_rate(json_t & src);

public:

  double corruption_rate() const;

  global_config_t & corruption_rate(double src);

  //--------------------------------------------------------------------
  // default_rate
  //--------------------------------------------------------------------

public:

  bool default_rate() const;

  //--------------------------------------------------------------------
  // down_degree
  //--------------------------------------------------------------------

public:

  long long down_degree() const;

  //--------------------------------------------------------------------
  // epoch_nonce
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::vector<unsigned char>> epoch_nonce_ = nullptr;

  bool have_epoch_nonce_ = false;

  void parse_epoch_nonce(json_t & src);

public:

  std::vector<unsigned char> const & epoch_nonce() const;

  global_config_t & epoch_nonce(std::vector<unsigned char> const & src);

  //--------------------------------------------------------------------
  // exp_mc_size
  //--------------------------------------------------------------------

public:

  long long exp_mc_size() const;

  //--------------------------------------------------------------------
  // exotic_rate
  //--------------------------------------------------------------------

public:

  bool exotic_rate() const;

  //--------------------------------------------------------------------
  // is_rigid
  //--------------------------------------------------------------------
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

public:

  bool is_rigid() const;

  //--------------------------------------------------------------------
  // max_connections
  //--------------------------------------------------------------------

public:

  long long max_connections() const;

  //--------------------------------------------------------------------
  // mbs_per_client
  //--------------------------------------------------------------------

public:

  node_count_t mbs_per_client() const;

  //--------------------------------------------------------------------
  // mc_size
  //--------------------------------------------------------------------

public:

  long long mc_size() const;

  //--------------------------------------------------------------------
  // min_good_mc_size
  //--------------------------------------------------------------------

public:

  long long min_good_mc_size() const;

  //--------------------------------------------------------------------
  // min_good_rs_layer_size
  //--------------------------------------------------------------------

public:

  long long min_good_rs_layer_size() const;

  //--------------------------------------------------------------------
  // min_mc_size
  //--------------------------------------------------------------------
  //
  // The minimum possible number of MC servers in an MC group.
  //

public:

  node_count_t min_mc_size() const;

  //--------------------------------------------------------------------
  // min_servers
  //--------------------------------------------------------------------
  //
  // The minimum possible number of servers in a deployment.
  //

public:

  node_count_t min_servers() const;

  //--------------------------------------------------------------------
  // num_buckets
  //--------------------------------------------------------------------

public:

  node_count_t num_buckets() const;

  //--------------------------------------------------------------------
  // num_clients
  //--------------------------------------------------------------------
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  long long num_clients_{};

  bool have_num_clients_ = false;

  void parse_num_clients(json_t & src);

public:

  long long num_clients() const;

  global_config_t & num_clients(long long src);

  //--------------------------------------------------------------------
  // num_idle
  //--------------------------------------------------------------------

public:

  long long num_idle() const;

  //--------------------------------------------------------------------
  // num_mbs
  //--------------------------------------------------------------------

public:

  node_count_t num_mbs() const;

  //--------------------------------------------------------------------
  // num_mcs
  //--------------------------------------------------------------------

public:

  node_count_t num_mcs() const;

  //--------------------------------------------------------------------
  // num_parallel_messages
  //--------------------------------------------------------------------

public:

  unsigned long num_parallel_messages() const;

  //--------------------------------------------------------------------
  // num_parallel_msgs_for_registration
  //--------------------------------------------------------------------

public:

  unsigned long num_parallel_msgs_for_registration() const;

  //--------------------------------------------------------------------
  // num_parallel_msgs_for_routing
  //--------------------------------------------------------------------

public:

  unsigned long num_parallel_msgs_for_routing() const;

  //--------------------------------------------------------------------
  // num_per_rs_layer
  //--------------------------------------------------------------------

public:

  unsigned long num_per_rs_layer() const;

  //--------------------------------------------------------------------
  // num_routing_layers
  //--------------------------------------------------------------------

public:

  unsigned long num_routing_layers() const;

  //--------------------------------------------------------------------
  // num_servers
  //--------------------------------------------------------------------
  //
  // This getter is thread safe. It is safe to call multiple thread safe
  // getters on the same instance of this class from multiple threads at
  // the same time.
  //

private:

  sst::optional<node_count_t> num_servers_;

  void parse_num_servers(json_t & src);

  void unparse_num_servers(json_t & dst) const;

public:

  node_count_t num_servers() const;

  void set_num_servers(node_count_t src);

  //--------------------------------------------------------------------
  // prime
  //--------------------------------------------------------------------

private:

  sst::bigint prime_{};

  bool have_prime_ = false;

  void parse_prime(json_t & src);

public:

  sst::bigint const & prime() const;

  global_config_t & prime(sst::bigint const & src);

  //--------------------------------------------------------------------
  // prime_space
  //--------------------------------------------------------------------
  //
  // Returns the number of bytes that can safely fit under the prime.
  //

public:

  unsigned long prime_space() const;

  //--------------------------------------------------------------------
  // privacy_failure_rate
  //--------------------------------------------------------------------

private:

  double privacy_failure_rate_{};

  bool have_privacy_failure_rate_ = false;

  void parse_privacy_failure_rate(json_t & src);

public:

  double privacy_failure_rate() const;

  global_config_t & privacy_failure_rate(double src);

  //--------------------------------------------------------------------
  // rangegen_row
  //--------------------------------------------------------------------
  //
  // Returns the appropriate row from the rangegen CSV tables based on
  // is_rigid and correctness_test_only.
  //

private:

  rangegen::row const & rangegen_row() const;

  //--------------------------------------------------------------------
  // robustness_failure_rate
  //--------------------------------------------------------------------

private:

  double robustness_failure_rate_{};

  bool have_robustness_failure_rate_ = false;

  void parse_robustness_failure_rate(json_t & src);

public:

  double robustness_failure_rate() const;

  global_config_t & robustness_failure_rate(double src);

  //--------------------------------------------------------------------
  // shamir_threshold
  //--------------------------------------------------------------------

private:

  sst::optional<node_count_t> shamir_threshold_;

  void parse_shamir_threshold(json_t & src);

  void unparse_shamir_threshold(json_t & dst) const;

public:

  node_count_t shamir_threshold() const;

  void set_shamir_threshold(node_count_t src);

  //--------------------------------------------------------------------
  // validate
  //--------------------------------------------------------------------

public:

  void validate() const;

  //--------------------------------------------------------------------
  // vrf
  //--------------------------------------------------------------------

private:

  vrf_shell_t vrf_{};

  bool have_vrf_ = false;

  void parse_vrf(json_t & src);

public:

  vrf_shell_t const & vrf() const;

  global_config_t & vrf(vrf_shell_t src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, global_config_t & dst);

  friend void to_json(json_t & dst, global_config_t const & src);

  //--------------------------------------------------------------------
  // Anonymous public-key encryption
  //--------------------------------------------------------------------

public:

  serialize_size_t anon_encrypt_size(serialize_size_t src) const;

  serialize_size_t anon_encrypt_size(bytes_t const & src) const;

  bytes_t anon_encrypt(bytes_t const & plaintext,
                       bytes_t const & recver_pk) const;

  bytes_t anon_decrypt(bytes_t const & ciphertext,
                       bytes_t const & recver_pk,
                       bytes_t const & recver_sk) const;

  //--------------------------------------------------------------------
  // Authenticated public-key encryption
  //--------------------------------------------------------------------

public:

  serialize_size_t auth_encrypt_size(serialize_size_t src) const;

  serialize_size_t auth_encrypt_size(bytes_t const & src) const;

  bytes_t auth_encrypt(bytes_t const & plaintext,
                       bytes_t const & sender_pk,
                       bytes_t const & sender_sk,
                       bytes_t const & recver_pk) const;

  bytes_t auth_decrypt(bytes_t const & ciphertext,
                       bytes_t const & recver_pk,
                       bytes_t const & recver_sk,
                       bytes_t const & sender_pk) const;

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_GLOBAL_CONFIG_T_HPP
