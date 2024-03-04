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

#ifndef KESTREL_CLUSTER_VECTOR_T_HPP
#define KESTREL_CLUSTER_VECTOR_T_HPP

#include <vector>

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/cluster_spec_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

class cluster_vector_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  cluster_vector_t() SST_NOEXCEPT(true) = default;

  ~cluster_vector_t() SST_NOEXCEPT(true) = default;

  cluster_vector_t(cluster_vector_t const &) = default;

  cluster_vector_t & operator=(cluster_vector_t const &) = default;

  cluster_vector_t(cluster_vector_t &&) SST_NOEXCEPT(true) = default;

  cluster_vector_t & operator=(cluster_vector_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // vector
  //--------------------------------------------------------------------

private:

  mutable sst::unique_ptr<std::vector<cluster_spec_t>> vector_;

public:

  SST_NODISCARD() std::vector<cluster_spec_t> & vector();

  SST_NODISCARD() std::vector<cluster_spec_t> const & vector() const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t const & src, cluster_vector_t & dst);

  friend void to_json(json_t & dst, cluster_vector_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_CLUSTER_VECTOR_T_HPP
