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
#include <kestrel/range_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/range_config_t.hpp>
//

#include <exception>
#include <regex>
#include <string>

#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_as.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

void from_json(json_t const & src, range_config_t & dst) {
  dst.clients.clear();
  dst.servers.clear();
  std::regex const client_regex("client", std::regex_constants::icase);
  std::regex const server_regex("server", std::regex_constants::icase);
  if (!src.is_object()) {
    throw sst::json::exception("Value must be an object");
  }
  try {
    auto const range_it = src.find("range");
    if (range_it == src.end()) {
      throw sst::json::exception("Value is missing");
    }
    json_t const & range = *range_it;
    try {
      auto const nodes_it = range.find("RACE_nodes");
      if (nodes_it == range.end()) {
        throw sst::json::exception("Value is missing");
      }
      json_t const & nodes = *nodes_it;
      if (!nodes.is_array()) {
        throw sst::json::exception("Value must be an array");
      }
      decltype(+nodes.size()) i = 0;
      for (json_t const & node : nodes) {
        try {
          bool const genesis = sst::json::get_as<bool>(node, "genesis");
          if (!genesis) {
            continue;
          }
          psn_t const name = sst::json::get_as<psn_t>(node, "name");
          try {
            if (dst.clients.find(name) != dst.clients.end()
                || dst.servers.find(name) != dst.servers.end()) {
              throw sst::json::exception("Persona "
                                         + sst::c_quote(name.value())
                                         + " is duplicated");
            }
          } catch (sst::json::exception const & e) {
            std::throw_with_nested(e.add_key("name"));
          }
          std::string const type =
              sst::json::get_as<std::string>(node, "type");
          try {
            if (std::regex_search(type, client_regex)) {
              dst.clients.insert(name);
            } else if (std::regex_search(type, server_regex)) {
              dst.servers.insert(name);
            } else {
              throw sst::json::exception(
                  "Value does not contain \"client\" or \"server\"");
            }
          } catch (sst::json::exception const & e) {
            std::throw_with_nested(e.add_key("type"));
          }
        } catch (sst::json::exception const & e) {
          std::throw_with_nested(e.add_index(i));
        }
        ++i;
      }
    } catch (sst::json::exception const & e) {
      std::throw_with_nested(e.add_key("RACE_nodes"));
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("range"));
  }
}

} // namespace kestrel
