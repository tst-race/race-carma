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

#ifndef KESTREL_OLD_CONFIG_T_HPP
#define KESTREL_OLD_CONFIG_T_HPP

#include <algorithm>
#include <kestrel/logging.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <cstdint>
#include <map>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <openssl/bn.h>
#include <set>
#include <sst/bn_ctx.h>
#include <sst/catalog/bigint.hpp>
#include <stdexcept>
#include <string>
#include <vector>

// TODO: This should go away once we're inside the kestrel::carma
//       namespace too.
using namespace kestrel::carma;

namespace kestrel {

using message_type = std::vector<std::uint8_t>;
using raw_packet_type = std::vector<std::uint8_t>;

/* TODO: Move these converters somewhere reasonable */
template<class SequenceContainer>
static void string2seq(SequenceContainer & v, std::string const & s) {
  uint8_t const * const s_data =
      reinterpret_cast<uint8_t const *>(s.data());
  v = SequenceContainer{s_data, s_data + s.size()};
}

template<class SequenceContainer>
static void seq2BN(BIGNUM * b, SequenceContainer const & v) {
  auto len = v.size();

  if (len > std::numeric_limits<uint32_t>::max()) {
    throw std::invalid_argument{"Input longer than 4GB"};
  }

  SequenceContainer vv{v};

  for (int i = 0; i != 4; ++i) {
    vv.push_back(static_cast<uint8_t>(len));
    len >>= 1;
    len >>= 7;
  }

  BN_bin2bn(vv.data(), vv.size(), b);
}

template<class SequenceContainer>
static void BN2seq(SequenceContainer & v, BIGNUM const * const b) {
  using size_type = typename SequenceContainer::size_type;
  std::vector<unsigned char> const x{sst::bn_ctx::bn2bin(*b)};
  auto const xn{x.size()};
  unsigned long const vn{
      (static_cast<unsigned long>((xn >= 4) ? x[xn - 4] : 0) << 0)
      | (static_cast<unsigned long>((xn >= 3) ? x[xn - 3] : 0) << 8)
      | (static_cast<unsigned long>((xn >= 2) ? x[xn - 2] : 0) << 16)
      | (static_cast<unsigned long>((xn >= 1) ? x[xn - 1] : 0) << 24)};
  if (xn > 4 && xn - 4 > vn) {
    throw std::runtime_error{"malformed"};
  }
  if (vn > std::numeric_limits<size_type>::max()) {
    throw std::overflow_error{"size_type overflow"};
  }
  v.clear();
  v.resize(static_cast<size_type>(vn), 0);
  if (xn > 4) {
    std::copy(x.begin(), x.end() - 4, v.end() - (xn - 4));
  }
}

class old_config_t final {

  //--------------------------------------------------------------------
  // allow_multicast
  //--------------------------------------------------------------------

private:
  bool done_allow_multicast_ = false;
  bool allow_multicast_;
  bool allow_multicast(nlohmann::json & src);

public:
  bool allow_multicast() const;

  //--------------------------------------------------------------------
  // dynamic_only
  //--------------------------------------------------------------------
  //
  // When dynamic_only is true, this node will never call
  // getLinksForPersonas().
  //

private:
  bool done_dynamic_only_ = false;
  bool dynamic_only_;
  bool dynamic_only(nlohmann::json & src);

public:
  bool dynamic_only() const;

  //--------------------------------------------------------------------
  // link_discovery_cooldown
  //--------------------------------------------------------------------

private:
  bool done_link_discovery_cooldown_ = false;
  std::uintmax_t link_discovery_cooldown_;
  std::uintmax_t link_discovery_cooldown(nlohmann::json & src);

public:
  std::uintmax_t link_discovery_cooldown() const;

  //--------------------------------------------------------------------
  // send_retry_count
  //--------------------------------------------------------------------

private:
  bool done_send_retry_count_ = false;
  sst::bigint send_retry_count_;
  sst::bigint const & send_retry_count(nlohmann::json & src);

public:
  sst::bigint const & send_retry_count() const;

  //--------------------------------------------------------------------
  // send_retry_window
  //--------------------------------------------------------------------

private:
  bool done_send_retry_window_ = false;
  sst::bigint send_retry_window_;
  sst::bigint const & send_retry_window(nlohmann::json & src);

public:
  sst::bigint const & send_retry_window() const;

  //--------------------------------------------------------------------
  // send_timeout
  //--------------------------------------------------------------------

private:
  bool done_send_timeout_ = false;
  sst::bigint send_timeout_;
  sst::bigint const & send_timeout(nlohmann::json & src);

public:
  sst::bigint const & send_timeout() const;

  //--------------------------------------------------------------------

public:

  unsigned int threshold;
  sst::bigint prime;

  size_t mixsize;

  int zeta = 3;
  int two_exponent = 3767;
  int odd_factor = 101;

  log_level_t loglevel;

  bool leader_relay_only;
  bool inter_server_direct_only;

  friend void from_json(nlohmann::json src, old_config_t & dst);
};

} // namespace kestrel

#endif // #ifndef KESTREL_OLD_CONFIG_T_HPP
