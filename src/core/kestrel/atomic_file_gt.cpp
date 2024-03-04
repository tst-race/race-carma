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
#include <kestrel/atomic_file_gt.hpp>
// Include twice to test idempotence.
#include <kestrel/atomic_file_gt.hpp>
//

#include <string>

namespace kestrel {

bool atomic_file_gt::operator()(std::string const & x,
                                std::string const & y) const {

  using sz = std::string::size_type;

  // 12345.12345
  // ^    ^^    ^
  // a    bc    d

  sz const xa = static_cast<sz>(0);
  sz const xb = x.find('.', xa);
  sz const xc = xb + static_cast<sz>(1);
  sz const xd = x.size();

  sz const ya = static_cast<sz>(0);
  sz const yb = y.find('.', ya);
  sz const yc = yb + static_cast<sz>(1);
  sz const yd = y.size();

  if (xb - xa < yb - ya) {
    return false;
  }
  if (xb - xa > yb - ya) {
    return true;
  }
  auto const c1 = x.compare(xa, xb - xa, y, ya, yb - ya);
  if (c1 < 0) {
    return false;
  }
  if (c1 > 0) {
    return true;
  }

  if (xd - xc < yd - yc) {
    return false;
  }
  if (xd - xc > yd - yc) {
    return true;
  }
  auto const c2 = x.compare(xc, xd - xc, y, yc, yd - yc);
  if (c2 < 0) {
    return false;
  }
  if (c2 > 0) {
    return true;
  }

  return false;

} //

} // namespace kestrel
