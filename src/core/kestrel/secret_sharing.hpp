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

#ifndef KESTREL_SECRET_SHARING_HPP
#define KESTREL_SECRET_SHARING_HPP

#include <sst/catalog/bigint.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/perfect_gt.hpp>
#include <sst/catalog/perfect_lt.hpp>
#include <sst/catalog/type_max.hpp>
#include <stdexcept>
#include <vector>

namespace kestrel {

template<class Numshares, class Degree>
void sss_share(std::vector<sst::bigint> & shrs,
               sst::bigint const & secret,
               Numshares numshares,
               Degree degree,
               sst::bigint const & modulus) {
  SST_ASSERT(sst::perfect_gt(numshares, 0));
  SST_ASSERT(sst::perfect_gt(degree, 0));
  SST_ASSERT(sst::perfect_gt(numshares, degree));
  SST_ASSERT(sst::perfect_lt(degree, sst::type_max<Degree>::value));
  SST_ASSERT(
      sst::perfect_lt(numshares, sst::type_max<Numshares>::value));
  SST_ASSERT(modulus > 1);
  SST_ASSERT(secret >= 0);
  SST_ASSERT(secret < modulus);

  sst::checked_resize(shrs, numshares);
  // TODO: pool
  std::vector<sst::bigint> poly;
  sst::checked_resize(poly, degree + 1);
  poly[0] = secret;
  for (decltype(poly.size()) i = 1; i < poly.size(); ++i) {
    poly[i] = rand_range(modulus);
    if (i == poly.size() - 1) {
      while (poly[poly.size() - 1] == 0) {
        poly[i] = rand_range(modulus);
      }
    }
  }
  sst::bigint evalpoint;
  for (decltype(+numshares) i = 0; i != numshares; ++i) {
    evalpoint = i + 1; // i < numshares < max, hence does not need check
    shrs[i] = poly[poly.size() - 1];
    for (decltype(poly.size()) j = 1; j < poly.size(); ++j) {
      shrs[i] = mul_mod(shrs[i], evalpoint, modulus);
      shrs[i] = add_mod(shrs[i], poly[poly.size() - 1 - j], modulus);
    }
  }
}

template<class Index>
void sss_recon(sst::bigint & secret,
               std::vector<sst::bigint> const & shrs,
               std::vector<Index> const & pts,
               sst::bigint const & modulus) {
  SST_ASSERT(sst::perfect_gt(shrs.size(), 0));
  SST_ASSERT(sst::perfect_gt(pts.size(), 0));
  SST_ASSERT(sst::perfect_ge(shrs.size(), pts.size()));
  SST_ASSERT(modulus > 1);

  auto const numshares = pts.size();
  sst::bigint inv, nxt;
  // TODO: pool
  std::vector<sst::bigint> x;
  sst::checked_resize(x, numshares);

  for (decltype(+numshares) i = 0; i != numshares; ++i) {
    x[i] = sst::bigint(1) + pts[i];
  }

  secret = 0;
  for (decltype(+numshares) j = 0; j != numshares; ++j) {
    if (pts[j] >= shrs.size()) {
      throw std::runtime_error("Invalid reconstruction point");
    }
    nxt = shrs[pts[j]];
    for (decltype(+numshares) i = 0; i != numshares; ++i) {
      if (i == j) {
        continue;
      }
      inv = sub_mod(x[i], x[j], modulus);
      inv = inv_mod(inv, modulus);
      nxt = mul_mod(nxt, x[i], modulus);
      nxt = mul_mod(nxt, inv, modulus);
    }
    secret = add_mod(secret, nxt, modulus);
  }
}

} // namespace kestrel

#endif // #ifndef KESTREL_SECRET_SHARING_HPP
