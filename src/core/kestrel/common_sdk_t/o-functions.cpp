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

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>
#include <kestrel/race_to_json.hpp>

namespace kestrel {

json_t common_sdk_t::o_appendFile(SdkResponse const & return_value,
                                  std::string const &,
                                  std::vector<std::uint8_t> const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t common_sdk_t::o_asyncError(SdkResponse const & return_value,
                                  RaceHandle,
                                  PluginResponse) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
common_sdk_t::o_getActivePersona(std::string const & return_value) {
  return {
      {"return_value", return_value},
  };
}

json_t common_sdk_t::o_getAllChannelProperties(
    std::vector<ChannelProperties> const & return_value) {
  json_t dst = json_t::object();
  json_t & ys = dst["return_value"];
  ys = json_t::array();
  for (ChannelProperties const & x : return_value) {
    ys += race_to_json(x);
  }
  return dst;
}

json_t common_sdk_t::o_getChannelProperties(
    ChannelProperties const & return_value,
    std::string const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t common_sdk_t::o_getEntropy(RawData const & return_value,
                                  std::uint32_t) {
  return {
      {"return_value", describe_blob(return_value)},
  };
}

json_t
common_sdk_t::o_listDir(std::vector<std::string> const & return_value,
                        std::string const &) {
  return {
      {"return_value", return_value},
  };
}

json_t common_sdk_t::o_makeDir(SdkResponse const & return_value,
                               std::string const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t
common_sdk_t::o_readFile(std::vector<std::uint8_t> const & return_value,
                         std::string const &) {
  return {
      {"return_value", describe_blob(return_value)},
  };
}

json_t common_sdk_t::o_removeDir(SdkResponse const & return_value,
                                 std::string const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t common_sdk_t::o_writeFile(SdkResponse const & return_value,
                                 std::string const &,
                                 std::vector<std::uint8_t> const &) {
  return {
      {"return_value", race_to_json(return_value)},
  };
}

json_t common_sdk_t::o_xListDirRecursive(
    std::vector<std::string> const & return_value,
    std::string const &) {
  return {
      {"return_value", return_value},
  };
}

json_t common_sdk_t::o_xMakeDirs(std::string const &) {
  return {
      {"return_value", nullptr},
  };
}

json_t common_sdk_t::o_xMakeParentDirs(std::string const &) {
  return {
      {"return_value", nullptr},
  };
}

json_t common_sdk_t::o_xPathExists(bool const return_value,
                                   std::string const &) {
  return {
      {"return_value", return_value},
  };
}

json_t common_sdk_t::o_xPathIsDir(bool const return_value,
                                  std::string const &) {
  return {
      {"return_value", return_value},
  };
}

json_t common_sdk_t::o_xWriteJsonFile(std::string const &,
                                      json_t const &) {
  return {
      {"return_value", nullptr},
  };
}

} // namespace kestrel
