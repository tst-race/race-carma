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
#include <kestrel/cluster_vector_t.hpp>
// Include twice to test idempotence.
#include <kestrel/cluster_vector_t.hpp>
//

#include <string>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/json/types.hpp>

#include <kestrel/cluster_spec_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// vector
//----------------------------------------------------------------------

std::vector<cluster_spec_t> & cluster_vector_t::vector() {
  vector_.ensure();
  if (vector_->empty()) {
    vector_->emplace_back();
  }
  return *vector_;
}

std::vector<cluster_spec_t> const & cluster_vector_t::vector() const {
  return const_cast<cluster_vector_t &>(*this).vector();
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t const & src, cluster_vector_t & dst) {

  std::vector<cluster_spec_t> & v = dst.vector();

  if (src.is_object()) {
    v.resize(1);
    sst::json::get_to(src, v.front());
  } else if (src.is_array()) {
    sst::json::get_to(src, v);
    static_cast<void>(dst.vector());
  } else {
    throw sst::json::exception::expected(sst::json::types::object()
                                         | sst::json::types::array());
  }

  {
    bool bad = true;
    for (cluster_spec_t const & cluster : v) {
      if (sst::is_positive(cluster.count())) {
        bad = false;
        break;
      }
    }
    if (bad) {
      throw sst::json::exception(
          "At least one cluster must have count > 0");
    }
  }

  for (decltype(v.size()) i = 0; i < v.size(); ++i) {
    for (decltype(v.size()) j = i; ++j < v.size();) {
      std::vector<std::string> const & a = v[i].pin();
      std::vector<std::string> const & b = v[j].pin();
      auto p = a.begin();
      auto q = b.begin();
      while (p != a.end() && q != b.end()) {
        int const c = p->compare(*q);
        if (c < 0) {
          ++p;
        } else if (c > 0) {
          ++q;
        } else {
          throw sst::json::exception("Node is pinned twice: "
                                     + sst::c_quote(*p));
        }
      }
    }
  }

} //

void to_json(json_t & dst, cluster_vector_t const & src) {
  dst = src.vector();
}

//----------------------------------------------------------------------

} // namespace kestrel
