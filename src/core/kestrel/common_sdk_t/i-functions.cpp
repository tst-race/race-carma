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
#include <string>
#include <vector>

#include <kestrel/json_t.hpp>

namespace kestrel {

json_t
common_sdk_t::i_appendFile(std::string const & filepath,
                           std::vector<std::uint8_t> const & data) {
  return {
      {"filepath", filepath},
      {"data", describe_blob(data)},
  };
}

json_t common_sdk_t::i_listDir(std::string const & directoryPath) {
  return {
      {"directoryPath", directoryPath},
  };
}

json_t common_sdk_t::i_makeDir(std::string const & directoryPath) {
  return {
      {"directoryPath", directoryPath},
  };
}

json_t common_sdk_t::i_readFile(std::string const & filepath) {
  return {
      {"filepath", filepath},
  };
}

json_t common_sdk_t::i_removeDir(std::string const & directoryPath) {
  return {
      {"directoryPath", directoryPath},
  };
}

json_t
common_sdk_t::i_writeFile(std::string const & filepath,
                          std::vector<std::uint8_t> const & data) {
  return {
      {"filepath", filepath},
      {"data", describe_blob(data)},
  };
}

json_t common_sdk_t::i_xListDirRecursive(std::string const & dir) {
  return {
      {"dir", dir},
  };
}

json_t common_sdk_t::i_xMakeDirs(std::string const & path) {
  return {
      {"path", path},
  };
}

json_t common_sdk_t::i_xMakeParentDirs(std::string const & path) {
  return {
      {"path", path},
  };
}

json_t common_sdk_t::i_xPathExists(std::string const & path) {
  return {
      {"path", path},
  };
}

json_t common_sdk_t::i_xPathIsDir(std::string const & path) {
  return {
      {"path", path},
  };
}

json_t common_sdk_t::i_xWriteJsonFile(std::string const & filepath,
                                      json_t const & json) {
  return {
      {"filepath", filepath},
      {"json", json},
  };
}

} // namespace kestrel
