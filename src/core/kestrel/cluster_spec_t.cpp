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
#include <kestrel/cluster_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/cluster_spec_t.hpp>
//

#include <algorithm>
#include <exception>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_UNREACHABLE.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/expect_object.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/types.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unsigned_eq.hpp>

#include <kestrel/cluster_provider_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/slugify.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// capacity
//----------------------------------------------------------------------

void cluster_spec_t::parse_capacity(json_t & src,
                                    parse_state_t & state) {
  if (state.called_parse_capacity_) {
    return;
  }
  state.called_parse_capacity_ = true;
  sst::json::remove_to(src, capacity_, "capacity", default_capacity());
}

void cluster_spec_t::unparse_capacity(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  dst["capacity"] = capacity_;
}

double cluster_spec_t::capacity() const noexcept {
  SST_ASSERT((!moved_from_));
  return capacity_;
}

//----------------------------------------------------------------------
// count
//----------------------------------------------------------------------

void cluster_spec_t::parse_count(json_t & src, parse_state_t & state) {
  if (state.called_parse_count_) {
    return;
  }
  state.called_parse_count_ = true;
  sst::json::remove_to(src, count_, "count", default_count());
  try {
    if (!sst::is_positive(count_)) {
      throw sst::json::exception("Value must be positive");
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("count"));
  }
}

void cluster_spec_t::unparse_count(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  dst["count"] = sst::to_string(count_);
}

int cluster_spec_t::count() const noexcept {
  SST_ASSERT((!moved_from_));
  return count_;
}

//----------------------------------------------------------------------
// os_version
//----------------------------------------------------------------------

void cluster_spec_t::parse_os_version(json_t & src,
                                      parse_state_t & state) {
  if (state.called_parse_os_version_) {
    return;
  }
  state.called_parse_os_version_ = true;
  sst::json::remove_to(src, os_version_, "os_version");
}

void cluster_spec_t::unparse_os_version(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  dst["os_version"] = os_version();
}

std::string const & cluster_spec_t::os_version() const {
  SST_ASSERT((!moved_from_));
  return os_version_.ensure("22.04");
}

//----------------------------------------------------------------------
// pin
//----------------------------------------------------------------------

void cluster_spec_t::parse_pin(json_t & src, parse_state_t & state) {
  if (state.called_parse_pin_) {
    return;
  }
  state.called_parse_pin_ = true;
  try {
    pin_.ensure();
    sst::json::expect_object(src);
    auto const it = src.find("pin");
    if (it == src.end()) {
      pin_->clear();
    } else if (it->is_string()) {
      sst::checked_resize(*pin_, 1);
      sst::json::get_to(*it, pin_->front());
      pin_->front() = unslugify(pin_->front());
      src.erase(it);
    } else if (it->is_array()) {
      sst::json::get_to(*it, *pin_);
      for (std::string & x : *pin_) {
        x = unslugify(x);
      }
      std::sort(pin_->begin(), pin_->end());
      pin_->erase(std::unique(pin_->begin(), pin_->end()), pin_->end());
      src.erase(it);
    } else {
      sst::json::exception::expected(sst::json::types::string()
                                     | sst::json::types::array());
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("pin"));
  }
}

void cluster_spec_t::unparse_pin(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  json_t & xs = dst["pin"];
  xs = json_t::array();
  if (pin_) {
    for (std::string const & x : *pin_) {
      xs.emplace_back(slugify(x));
    }
  }
}

std::vector<std::string> const & cluster_spec_t::pin() const {
  SST_ASSERT((!moved_from_));
  return pin_.ensure();
}

//----------------------------------------------------------------------
// profile
//----------------------------------------------------------------------

void cluster_spec_t::parse_profile(json_t & src,
                                   parse_state_t & state) {
  if (state.called_parse_profile_) {
    return;
  }
  state.called_parse_profile_ = true;
  parse_provider(src, state);
  if (provider() == cluster_provider_t::aws()) {
    sst::json::remove_to(src, profile_, "profile");
  }
}

void cluster_spec_t::unparse_profile(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (profile_) {
    dst["profile"] = *profile_;
  }
}

std::string const * cluster_spec_t::profile() const noexcept {
  SST_ASSERT((!moved_from_));
  return profile_.get();
}

//----------------------------------------------------------------------
// provider
//----------------------------------------------------------------------

void cluster_spec_t::parse_provider(json_t & src,
                                    parse_state_t & state) {
  if (state.called_parse_provider_) {
    return;
  }
  state.called_parse_provider_ = true;
  sst::json::remove_to(src, provider_, "provider", default_provider());
}

void cluster_spec_t::unparse_provider(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  dst["provider"] = provider_;
}

cluster_provider_t cluster_spec_t::provider() const noexcept {
  SST_ASSERT((!moved_from_));
  return provider_;
}

//----------------------------------------------------------------------
// regions
//----------------------------------------------------------------------

void cluster_spec_t::parse_regions(json_t & src,
                                   parse_state_t & state) {
  if (state.called_parse_regions_) {
    return;
  }
  state.called_parse_regions_ = true;
  parse_provider(src, state);
  sst::json::expect_object(src);
  switch (provider()) {
    case cluster_provider_t::aws():
    case cluster_provider_t::azure(): {
      try {
        regions_.ensure();
        auto const it = src.find("regions");
        if (it == src.end()) {
          sst::checked_resize(*regions_, 1);
          switch (provider()) {
            case cluster_provider_t::aws(): {
              regions_->front() = "us-west-1";
            } break;
            case cluster_provider_t::azure(): {
              regions_->front() = "westus";
            } break;
            default: {
              SST_UNREACHABLE();
            } break;
          }
        } else if (it->is_string()) {
          sst::checked_resize(*regions_, 1);
          sst::json::get_to(*it, regions_->front());
          src.erase(it);
        } else if (it->is_array()) {
          sst::json::get_to(*it, *regions_);
          if (regions_->empty()) {
            throw sst::json::exception("Array must be nonempty");
          }
          std::sort(regions_->begin(), regions_->end());
          regions_->erase(
              std::unique(regions_->begin(), regions_->end()),
              regions_->end());
          src.erase(it);
        } else {
          sst::json::exception::expected(sst::json::types::string()
                                         | sst::json::types::array());
        }
      } catch (sst::json::exception const & e) {
        std::throw_with_nested(e.add_key("regions"));
      }
    } break;
    default: {
      ;
    } break;
  }
}

void cluster_spec_t::unparse_regions(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (provider() == cluster_provider_t::aws()) {
    dst["regions"] = regions();
  }
}

std::vector<std::string> const &
cluster_spec_t::regions() const noexcept {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((regions_));
  return *regions_;
}

//----------------------------------------------------------------------
// spot
//----------------------------------------------------------------------

void cluster_spec_t::parse_spot(json_t & src, parse_state_t & state) {
  if (state.called_parse_spot_) {
    return;
  }
  state.called_parse_spot_ = true;
  sst::json::remove_to(src, spot_, "spot", default_spot());
}

void cluster_spec_t::unparse_spot(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (provider() == cluster_provider_t::aws()) {
    dst["spot"] = spot_;
  }
}

bool cluster_spec_t::spot() const noexcept {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((provider() == cluster_provider_t::aws()));
  return spot_;
}

//----------------------------------------------------------------------
// subscription
//----------------------------------------------------------------------

void cluster_spec_t::parse_subscription(json_t & src,
                                        parse_state_t & state) {
  if (state.called_parse_subscription_) {
    return;
  }
  state.called_parse_subscription_ = true;
  parse_provider(src, state);
  if (provider() == cluster_provider_t::azure()) {
    sst::json::remove_to(src, subscription_, "subscription");
  }
}

void cluster_spec_t::unparse_subscription(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (subscription_) {
    dst["subscription"] = *subscription_;
  }
}

std::string const * cluster_spec_t::subscription() const noexcept {
  SST_ASSERT((!moved_from_));
  return subscription_.get();
}

//----------------------------------------------------------------------
// type
//----------------------------------------------------------------------

void cluster_spec_t::parse_type(json_t & src, parse_state_t & state) {
  if (state.called_parse_type_) {
    return;
  }
  state.called_parse_type_ = true;
  parse_provider(src, state);
  switch (provider()) {
    case cluster_provider_t::aws(): {
      sst::json::remove_to(src, type_.ensure(), "type", "t3.large");
    } break;
    case cluster_provider_t::azure(): {
      sst::json::remove_to(src,
                           type_.ensure(),
                           "type",
                           "Standard_B2ms");
    } break;
    default: {
      ;
    } break;
  }
}

void cluster_spec_t::unparse_type(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  if (type_) {
    dst["type"] = *type_;
  }
}

std::string const & cluster_spec_t::type() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((type_));
  return *type_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, cluster_spec_t & dst) {

  cluster_spec_t::parse_state_t state;

  dst.parse_capacity(src, state);
  dst.parse_count(src, state);
  dst.parse_os_version(src, state);
  dst.parse_pin(src, state);
  dst.parse_profile(src, state);
  dst.parse_provider(src, state);
  dst.parse_regions(src, state);
  dst.parse_spot(src, state);
  dst.parse_subscription(src, state);
  dst.parse_type(src, state);

  SST_ASSERT((state.called_parse_capacity_));
  SST_ASSERT((state.called_parse_count_));
  SST_ASSERT((state.called_parse_os_version_));
  SST_ASSERT((state.called_parse_pin_));
  SST_ASSERT((state.called_parse_profile_));
  SST_ASSERT((state.called_parse_provider_));
  SST_ASSERT((state.called_parse_regions_));
  SST_ASSERT((state.called_parse_spot_));
  SST_ASSERT((state.called_parse_subscription_));
  SST_ASSERT((state.called_parse_type_));

  sst::json::unknown_key(src);

  dst.moved_from_ = false;

} //

void to_json(json_t & dst, cluster_spec_t const & src) {
  SST_ASSERT((!src.moved_from_));

  dst = json_t::object();

  src.unparse_capacity(dst);
  src.unparse_count(dst);
  src.unparse_os_version(dst);
  src.unparse_pin(dst);
  src.unparse_profile(dst);
  src.unparse_provider(dst);
  src.unparse_regions(dst);
  src.unparse_spot(dst);
  src.unparse_subscription(dst);
  src.unparse_type(dst);

} //

//----------------------------------------------------------------------

} // namespace kestrel
