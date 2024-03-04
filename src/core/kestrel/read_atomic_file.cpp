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
#include <kestrel/read_atomic_file.hpp>
// Include twice to test idempotence.
#include <kestrel/read_atomic_file.hpp>
//

#include <algorithm>
#include <regex>
#include <string>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/read_whole_file.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_e.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/atomic_file_gt.hpp>
#include <kestrel/atomic_file_regex.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

std::vector<unsigned char> read_atomic_file(tracing_event_t tev,
                                            common_sdk_t * const sdk,
                                            std::string const & file) {
  SST_TEV_TOP(tev);

  if (sdk) {
    if (!sdk->xPathExists(SST_TEV_ARG(tev), file)) {
      return {};
    }
  } else {
    if (!sst::test_e(file)) {
      return {};
    }
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

  std::vector<unsigned char> data;

  std::string d(file);
  d += '/';
  auto const dn = d.size();

  // Search for the latest copy, deleting any broken copies.
  auto x = xs.begin();
  std::vector<std::string> ys;
  for (; x != xs.end(); ++x) {
    d.resize(dn);
    d += *x;
    if (sdk) {
      ys = sdk->listDir(SST_TEV_ARG(tev), d);
    } else {
      ys.clear();
      for (sst::path const & y : sst::dir_it(d)) {
        ys.emplace_back(y.str());
      }
    }
    bool has_data = false;
    bool has_done = false;
    for (std::string const & y : ys) {
      if (y == "data") {
        has_data = true;
      } else if (y == "done") {
        has_done = true;
      }
    }
    if (has_data && has_done) {
      break;
    }
    if (sdk) {
      sdk->removeDir(SST_TEV_ARG(tev), d);
    } else {
      sst::rm_f_r(d);
    }
  }

  // If we found the latest copy, read it. Otherwise, leave data empty.
  if (x != xs.end()) {
    if (sdk) {
      data = sdk->readFile(SST_TEV_ARG(tev), d + "/data");
    } else {
      data = sst::read_whole_file(d + "/data");
    }
    ++x;
  }

  // Delete any outdated copies.
  for (; x != xs.end(); ++x) {
    d.resize(dn);
    d += *x;
    if (sdk) {
      sdk->removeDir(SST_TEV_ARG(tev), d);
    } else {
      sst::rm_f_r(d);
    }
  }

  return data;

  SST_TEV_BOT(tev);
}

} // namespace kestrel
