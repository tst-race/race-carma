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

#ifndef KESTREL_CLUSTER_SPEC_T_HPP
#define KESTREL_CLUSTER_SPEC_T_HPP

#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/cluster_provider_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

class cluster_spec_t final {

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  cluster_spec_t() = default;

  ~cluster_spec_t() SST_NOEXCEPT(true) = default;

  cluster_spec_t(cluster_spec_t const &) = default;

  cluster_spec_t & operator=(cluster_spec_t const &) = default;

  cluster_spec_t(cluster_spec_t &&) SST_NOEXCEPT(true) = default;

  cluster_spec_t & operator=(cluster_spec_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // parse_state_t
  //--------------------------------------------------------------------
  //
  // Keeps track of which parse_*() functions have already been called
  // during a from_json() call.
  //

private:

  struct parse_state_t {
    bool called_parse_capacity_ = false;
    bool called_parse_count_ = false;
    bool called_parse_os_version_ = false;
    bool called_parse_pin_ = false;
    bool called_parse_profile_ = false;
    bool called_parse_provider_ = false;
    bool called_parse_regions_ = false;
    bool called_parse_spot_ = false;
    bool called_parse_subscription_ = false;
    bool called_parse_type_ = false;
  };

  //--------------------------------------------------------------------
  // capacity
  //--------------------------------------------------------------------

private:

  static constexpr double default_capacity() noexcept {
    return 1.0;
  }

  double capacity_ = default_capacity();

  void parse_capacity(json_t & src, parse_state_t & state);

  void unparse_capacity(json_t & dst) const;

public:

  double capacity() const noexcept;

  //--------------------------------------------------------------------
  // count
  //--------------------------------------------------------------------

private:

  static constexpr int default_count() noexcept {
    return 1;
  }

  int count_ = default_count();

  void parse_count(json_t & src, parse_state_t & state);

  void unparse_count(json_t & dst) const;

public:

  int count() const noexcept;

  //--------------------------------------------------------------------
  // os_version
  //--------------------------------------------------------------------

private:

  mutable sst::unique_ptr<std::string> os_version_;

  void parse_os_version(json_t & src, parse_state_t & state);

  void unparse_os_version(json_t & dst) const;

public:

  std::string const & os_version() const;

  //--------------------------------------------------------------------
  // pin
  //--------------------------------------------------------------------
  //
  // The list of nodes that are pinned to this cluster. This is usually
  // empty, meaning no nodes are pinned to this cluster. When it is not
  // empty, it will be sorted and have no duplicates.
  //
  // The names are unslugified. TODO: This will be implicit from psn_t.
  // TODO: Use psn_t after its from_json unslugifies.
  //

private:

  mutable sst::unique_ptr<std::vector<std::string>> pin_;

  void parse_pin(json_t & src, parse_state_t & state);

  void unparse_pin(json_t & dst) const;

public:

  std::vector<std::string> const & pin() const;

  //--------------------------------------------------------------------
  // profile
  //--------------------------------------------------------------------
  //
  // For the aws provider, indicates which credentials profile to use.
  //

private:

  sst::unique_ptr<std::string> profile_;

  void parse_profile(json_t & src, parse_state_t & state);

  void unparse_profile(json_t & dst) const;

public:

  std::string const * profile() const noexcept;

  //--------------------------------------------------------------------
  // provider
  //--------------------------------------------------------------------

private:

  static constexpr cluster_provider_t default_provider() noexcept {
    return cluster_provider_t::manual();
  }

  cluster_provider_t provider_ = default_provider();

  void parse_provider(json_t & src, parse_state_t & state);

  void unparse_provider(json_t & dst) const;

public:

  cluster_provider_t provider() const noexcept;

  //--------------------------------------------------------------------
  // regions
  //--------------------------------------------------------------------
  //
  // For the aws and azure providers, indicates which regions to use.
  //
  // The value can be a single region or an array of regions. When more
  // than one region is given, the behavior is as if this cluster were
  // repeated for each individual region.
  //
  // The value can be omitted, in which case the following region is
  // used by default:
  //
  //       aws: us-west-1
  //       azure: westus
  //

private:

  sst::unique_ptr<std::vector<std::string>> regions_;

  void parse_regions(json_t & src, parse_state_t & state);

  void unparse_regions(json_t & dst) const;

public:

  std::vector<std::string> const & regions() const noexcept;

  //--------------------------------------------------------------------
  // spot
  //--------------------------------------------------------------------
  //
  // For the aws provider, indicates whether to use spot instances.
  //

private:

  static constexpr bool default_spot() noexcept {
    return false;
  }

  bool spot_ = default_spot();

  void parse_spot(json_t & src, parse_state_t & state);

  void unparse_spot(json_t & dst) const;

public:

  bool spot() const noexcept;

  //--------------------------------------------------------------------
  // subscription
  //--------------------------------------------------------------------
  //
  // For the azure provider, indicates which subscription to use.
  //

private:

  sst::unique_ptr<std::string> subscription_;

  void parse_subscription(json_t & src, parse_state_t & state);

  void unparse_subscription(json_t & dst) const;

public:

  std::string const * subscription() const noexcept;

  //--------------------------------------------------------------------
  // type
  //--------------------------------------------------------------------
  //
  // For the aws and azure providers, indicates what type of machine to
  // use.
  //
  // The value can be omitted, in which case the following machine type
  // is used by default:
  //
  //       aws: t3.large
  //       azure: Standard_B2ms
  //

private:

  sst::unique_ptr<std::string> type_;

  void parse_type(json_t & src, parse_state_t & state);

  void unparse_type(json_t & dst) const;

public:

  std::string const & type() const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, cluster_spec_t & dst);

  friend void to_json(json_t & dst, cluster_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_CLUSTER_SPEC_T_HPP
