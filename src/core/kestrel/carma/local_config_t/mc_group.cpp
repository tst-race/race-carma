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
#include <kestrel/carma/local_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/local_config_t.hpp>
//

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/expect_array.hpp>
#include <sst/catalog/json/expect_object.hpp>
#include <sst/catalog/json/expect_string.hpp>
#include <sst/catalog/mutex.hpp>
#include <sst/catalog/remove_cvref_t.hpp>
#include <sst/catalog/to_integer.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_unsigned.hpp>

#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/phonebook_vector_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/config_exception.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::clear_mc_groups() const {
  SST_ASSERT((!moved_from_));
  mc_groups_.reset();
}

void local_config_t::parse_mc_groups(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (mc_groups_) {
    return;
  }
  auto const it = src.find("mc_groups");
  if (it == src.end()) {
    return;
  }
  try {
    auto const & xss = sst::json::expect_object(*it);
    if (xss.empty()) {
      return;
    }
    sst::remove_cvref_t<decltype(*mc_groups_)> yss;
    for (auto const & k_xs : xss.items()) {
      try {
        auto const & xs = sst::json::expect_array(k_xs.value());
        if (xs.empty()) {
          continue;
        }
        auto & ys = yss[sst::to_integer<node_count_t>(k_xs.key())];
        decltype(xs.size()) i = 0U;
        for (json_t const & x : xs) {
          try {
            ys.emplace_back(&phonebook().expect<sst::json::exception>(
                psn_t(sst::json::expect_string(x)
                          .get_ref<json_t::string_t const &>())));
            ++i;
          } catch (sst::json::exception const & e) {
            std::throw_with_nested(e.add_index(i));
          }
        }
      } catch (sst::json::exception const & e) {
        std::throw_with_nested(e.add_key(k_xs.key()));
      }
    }
    mc_groups_.emplace(std::move(yss));
    src.erase(it);
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("mc_groups"));
  }
}

void local_config_t::unparse_mc_groups(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (mc_groups_) {
    auto & a = dst["mc_groups"] = json_t::object();
    for (auto const & kv : *mc_groups_) {
      auto & b = a[sst::to_string(kv.first)] = json_t::array();
      for (auto const & x : kv.second) {
        b.emplace_back(x->first.value());
      }
    }
  }
}

phonebook_vector_t const &
local_config_t::mc_group(tracing_event_t tev,
                         node_count_t const group) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  std::lock_guard<sst::mutex> const lock(mc_groups_mutex_);
  mc_groups_.ensure();
  auto const it = mc_groups_->find(group);
  if (it != mc_groups_->end()) {
    return it->second;
  }
  std::map<node_count_t, phonebook_pair_t const *> xs;
  for (phonebook_pair_t const & psn_pbe : phonebook()) {
    std::shared_ptr<phonebook_entry_t const> const pbe =
        phonebook().at(SST_TEV_ARG(tev), psn_pbe);
    if ((pbe->role() == role_t::mc_leader()
         || pbe->role() == role_t::mc_follower())
        && pbe->group() == group) {
      node_count_t const order = pbe->order();
      if (pbe->role() == role_t::mc_leader() && order != 0U) {
        throw config_exception("MC leader "
                               + sst::c_quote(pbe->psn().value())
                               + " in group " + sst::to_string(group)
                               + " has order " + sst::to_string(order));
      }
      if (pbe->role() == role_t::mc_follower() && order == 0U) {
        throw config_exception("MC follower "
                               + sst::c_quote(pbe->psn().value())
                               + " in group " + sst::to_string(group)
                               + " has order " + sst::to_string(order));
      }
      auto const r = xs.emplace(order, &psn_pbe);
      if (!r.second) {
        throw config_exception(
            "MC servers "
            + sst::c_quote((*r.first).second->first.value()) + " and "
            + sst::c_quote(psn_pbe.first.value()) + " in group "
            + sst::to_string(group) + " both have order "
            + sst::to_string(order));
      }
    }
  }
  auto const size = sst::to_unsigned(xs.size());
  if (size < global().min_mc_size()) {
    throw config_exception(
        "MC group " + sst::to_string(group) + " only has "
        + sst::to_string(size)
        + " servers, which is fewer than the minimum of "
        + sst::to_string(global().min_mc_size()));
  }
  node_count_t const max_order = xs.rbegin()->first;
  if (max_order >= size) {
    throw config_exception(
        "MC server " + sst::c_quote(xs.at(max_order)->first.value())
        + " in group " + sst::to_string(group) + " has order "
        + sst::to_string(max_order)
        + ", which is not less than the size of its group, "
        + sst::to_string(size));
  }
  phonebook_vector_t ys;
  for (auto const & kv : xs) {
    ys.emplace_back(kv.second);
  }
  auto const r = mc_groups_->emplace(group, std::move(ys));
  SST_ASSERT((r.second));
  return r.first->second;
  SST_TEV_BOT(tev);
}

phonebook_vector_t const &
local_config_t::mc_group(tracing_event_t tev,
                         phonebook_entry_t const & server) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  if (server.role() != role_t::mc_leader()
      && server.role() != role_t::mc_follower()) {
    throw config_exception(
        "Node " + sst::c_quote(server.psn().value())
        + " was expected to have role " + role_t::mc_leader().name()
        + " or " + role_t::mc_follower().name() + " but has role "
        + server.role().name() + " instead");
  }
  return mc_group(SST_TEV_ARG(tev), server.group());
  SST_TEV_BOT(tev);
}

phonebook_vector_t const &
local_config_t::mc_group(tracing_event_t tev,
                         phonebook_pair_t const & server) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  return mc_group(SST_TEV_ARG(tev),
                  *phonebook().at(SST_TEV_ARG(tev), server));
  SST_TEV_BOT(tev);
}

phonebook_vector_t const &
local_config_t::mc_group(tracing_event_t tev,
                         psn_t const & server) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  return mc_group(SST_TEV_ARG(tev),
                  *phonebook().at(SST_TEV_ARG(tev), server));
  SST_TEV_BOT(tev);
}

phonebook_vector_t const &
local_config_t::mc_group(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  return mc_group(SST_TEV_ARG(tev), *this);
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
