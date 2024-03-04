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
#include <kestrel/common_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/common_sdk_t.hpp>
//

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <sst/catalog/c_quote.hpp>

#include <IRaceSdkCommon.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_INNER_E_FOO.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/normalize_path.hpp>

namespace kestrel {

namespace {

CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, appendFile)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, listDir)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, makeDir)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, readFile)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, removeDir)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, writeFile)

} // namespace

SdkResponse
common_sdk_t::e_appendFile(std::string const & filepath,
                           std::vector<std::uint8_t> const & data) {
  return inner_e_appendFile(sdk(), normalize_path(filepath), data);
}

std::vector<std::string>
common_sdk_t::e_listDir(std::string const & directoryPath) {
  return inner_e_listDir(sdk(), normalize_path(directoryPath));
}

SdkResponse common_sdk_t::e_makeDir(std::string const & directoryPath) {
  return inner_e_makeDir(sdk(), normalize_path(directoryPath));
}

std::vector<std::uint8_t>
common_sdk_t::e_readFile(std::string const & filepath) {
  return inner_e_readFile(sdk(), normalize_path(filepath));
}

SdkResponse
common_sdk_t::e_removeDir(std::string const & directoryPath) {
  return inner_e_removeDir(sdk(), normalize_path(directoryPath));
}

SdkResponse
common_sdk_t::e_writeFile(std::string const & filepath,
                          std::vector<std::uint8_t> const & data) {
  return inner_e_writeFile(sdk(), normalize_path(filepath), data);
}

std::vector<std::string>
common_sdk_t::e_xListDirRecursive(std::string const & dir) {
  std::string const d = normalize_path(dir);
  if (!e_xPathExists(d)) {
    throw std::runtime_error("Directory " + sst::c_quote(dir)
                             + " does not exist.");
  }
  std::vector<std::string> xs;
  try {
    xs = e_listDir(d);
  } catch (...) {
    // TODO: Distinguish between non-directory and error.
    throw std::runtime_error("Directory " + sst::c_quote(dir)
                             + " does not exist.");
  }
  std::vector<std::string> ys;
  for (decltype(xs.size()) i = 0; i < xs.size(); ++i) {
    try {
      ys = e_listDir(d + "/" + xs[i]);
    } catch (...) {
      // TODO: Distinguish between non-directory and error.
      continue;
    }
    for (std::string const & y : ys) {
      xs.emplace_back(xs[i] + "/" + y);
    }
  }
  return xs;
}

void common_sdk_t::e_xMakeDirs(std::string const & path) {
  std::string const x = "./" + normalize_path(path);
  std::string::size_type k = 1;
  while (k < x.size()) {
    std::string::size_type const i = k;
    ++k;
    std::string::size_type const j = k;
    while (k < x.size() && x[k] != '/') {
      ++k;
    }
    std::string const d = x.substr(0, i);
    std::vector<std::string> const ys = e_listDir(d);
    std::string const z = x.substr(j, k - j);
    if (z == ".") {
      continue;
    }
    bool found = false;
    for (std::string const & y : ys) {
      if (y == z) {
        found = true;
        break;
      }
    }
    if (!found) {
      e_makeDir(x.substr(0, k));
    }
  }
}

void common_sdk_t::e_xMakeParentDirs(std::string const & path) {
  std::string const x = "./" + normalize_path(path);
  std::string::size_type k = 1;
  while (k < x.size()) {
    std::string::size_type const i = k;
    ++k;
    std::string::size_type const j = k;
    while (k < x.size() && x[k] != '/') {
      ++k;
    }
    if (k == x.size()) {
      break;
    }
    std::string const d = x.substr(0, i);
    std::vector<std::string> const ys = e_listDir(d);
    std::string const z = x.substr(j, k - j);
    if (z == ".") {
      continue;
    }
    bool found = false;
    for (std::string const & y : ys) {
      if (y == z) {
        found = true;
        break;
      }
    }
    if (!found) {
      e_makeDir(x.substr(0, k));
    }
  }
}

bool common_sdk_t::e_xPathExists(std::string const & path) {
  std::string const x = "./" + normalize_path(path);
  std::string::size_type k = 1;
  while (k < x.size()) {
    std::string::size_type const i = k;
    ++k;
    std::string::size_type const j = k;
    while (k < x.size() && x[k] != '/') {
      ++k;
    }
    std::string const d = x.substr(0, i);
    std::vector<std::string> ys;
    try {
      ys = e_listDir(d);
    } catch (...) {
      // TODO: Distinguish between non-directory and error. The former
      //       should return false, and the latter should throw.
      return false;
    }
    std::string const z = x.substr(j, k - j);
    if (z == ".") {
      continue;
    }
    bool found = false;
    for (std::string const & y : ys) {
      if (y == z) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}

//
// In the official RACE 2.1.0 SDK, listDir() does not throw an exception
// or otherwise indicate failure when it is called on a file. This means
// we can't use it to determine whether an existent path is a directory.
// Fortunately, readFile() throws an exception when it is called on a
// directory, so we can utilize that.
//
// It seems that RACE 2.2.0 has additionally broken the above. Avoid
// using this function for now.
//

bool common_sdk_t::e_xPathIsDir(std::string const & path) {
  std::string const x = normalize_path(path);
  if (!e_xPathExists(x)) {
    return false;
  }
  try {
    e_readFile(x);
  } catch (...) {
    // TODO: Distinguish between not a file and error. The former should
    //       return true, and the latter should throw.
    return true;
  }
  return false;
}

void common_sdk_t::e_xWriteJsonFile(std::string const & filepath,
                                    json_t const & json) {
  std::string const text = json.dump(2, ' ', true);
  // We don't need to call normalize_path here because e_writeFile will
  // do it for us.
  e_writeFile(filepath,
              std::vector<std::uint8_t>(text.begin(), text.end()));
}

} // namespace kestrel
