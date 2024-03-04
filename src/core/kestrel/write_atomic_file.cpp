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
#include <kestrel/write_atomic_file.hpp>
// Include twice to test idempotence.
#include <kestrel/write_atomic_file.hpp>
//

#include <algorithm>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/mkdir.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_e.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unix_time_s.hpp>
#include <sst/catalog/write_whole_file.hpp>

#include <kestrel/atomic_file_gt.hpp>
#include <kestrel/atomic_file_regex.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void write_atomic_file(tracing_event_t tev,
                       common_sdk_t * const sdk,
                       std::string const & file,
                       std::vector<unsigned char> const & data) {
  SST_TEV_TOP(tev);

  using sz = std::string::size_type;

  if (sdk) {
    sdk->xMakeDirs(SST_TEV_ARG(tev), file);
  } else {
    sst::mkdir_p(file);
  }

  std::vector<std::string> xs;
  if (sdk) {
    xs = sdk->listDir(SST_TEV_ARG(tev), file);
  } else {
    for (sst::path const & x : sst::dir_it(file)) {
      xs.emplace_back(x.str());
    }
  }

  std::regex const r(atomic_file_regex());
  xs.erase(std::remove_if(xs.begin(),
                          xs.end(),
                          [&r](std::string const & x) {
                            return !std::regex_match(x, r);
                          }),
           xs.end());

  std::sort(xs.begin(), xs.end(), atomic_file_gt());

  // Construct our desired version pair.
  std::string y = sst::to_string(sst::unix_time_s());
  y += ".1";

  // If our desired version pair isn't greater than all existing pairs,
  // reconstruct it.
  if (!xs.empty() && atomic_file_gt()(xs.front(), y)) {
    std::string const & x = xs.front();
    sz const i = x.find('.');
    y = x;
    y.resize(i + static_cast<sz>(1));
    sst::to_string(sst::checked_cast<unsigned long>(
                       sst::checked(sst::to_integer<unsigned long>(
                           &x[i] + 1,
                           &x[0] + x.size()))
                       + 1),
                   std::back_inserter(y));
  }

  // Write out "data" and "done".
  y = file + '/' + y;
  std::vector<unsigned char> const empty;
  if (sdk) {
    sdk->makeDir(SST_TEV_ARG(tev), y);
    sdk->writeFile(SST_TEV_ARG(tev), y + "/data", data);
    sdk->writeFile(SST_TEV_ARG(tev), y + "/done", empty);
  } else {
    sst::mkdir(y);
    sst::write_whole_file(data, y + "/data");
    sst::write_whole_file(empty, y + "/done");
  }

  // Delete any outdated copies.
  if (!xs.empty()) {
    std::string d(file);
    d += '/';
    auto const dn = d.size();
    for (std::string const & x : xs) {
      d.resize(dn);
      d += x;
      if (sdk) {
        sdk->removeDir(SST_TEV_ARG(tev), d);
      } else {
        sst::rm_f_r(d);
      }
    }
  }

  SST_TEV_BOT(tev);
}

} // namespace kestrel
