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
#include <kestrel/kestrel.hpp>
// Include twice to test idempotence.
#include <kestrel/kestrel.hpp>
//

#include <exception>
#include <iostream>
#include <list>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/unknown_opt.hpp>

#include <kestrel/kestrel_cli_args_t.hpp>
#include <kestrel/kestrel_genesis.hpp>
#include <kestrel/kestrel_node.hpp>
#include <kestrel/kestrel_stack.hpp>

namespace kestrel {

namespace {

void help(std::string const & argv0, std::ostream & cout) {
  cout << "Usage: " << argv0 << " <command>";
  cout << R"(

Commands:
  genesis
  node
  stack
)";
}

} // namespace

void kestrel(std::list<std::string> argv, std::ostream & cout) {

  if (argv.empty()) {
    argv.push_back("kestrel");
  }

  std::string const argv0 = std::move(argv.front());

  kestrel_cli_args_t kestrel_cli_args;

  bool parse_options = true;
  while (argv.pop_front(), !argv.empty()) {
    if (parse_options) {

      //----------------------------------------------------------------
      // Options terminator
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--", sst::opt_arg::forbidden)) {
        parse_options = false;
        continue;
      }

      //----------------------------------------------------------------
      // --help
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--help", sst::opt_arg::forbidden)) {
        help(argv0, cout);
        return;
      }

      //----------------------------------------------------------------
      // Unknown options
      //----------------------------------------------------------------

      sst::unknown_opt(argv);

      //----------------------------------------------------------------
    }

    //------------------------------------------------------------------
    // Commands
    //------------------------------------------------------------------

    std::unordered_map<std::string,
                       void (*)(
                           kestrel_cli_args_t const & kestrel_cli_args,
                           std::list<std::string> argv,
                           std::ostream & cout)> const commands{
        {"genesis", &kestrel_genesis},
        {"node", &kestrel_node},
        {"stack", &kestrel_stack},
    };
    std::string const command = std::move(argv.front());
    auto const it = commands.find(command);
    if (it == commands.end()) {
      throw std::runtime_error("Unknown command: "
                               + sst::c_quote(command));
    }
    argv.front() = argv0 + " " + command;
    try {
      it->second(kestrel_cli_args, std::move(argv), cout);
    } catch (...) {
      std::throw_with_nested(std::runtime_error(command));
    }
    return;

    //------------------------------------------------------------------
  }

  help(argv0, cout);
}

void kestrel(int const argc, char ** const argv) {
  SST_ASSERT((argc >= 0));
  SST_ASSERT((argv[argc] == nullptr));
  kestrel(std::list<std::string>(argv, argv + argc), std::cout);
}

} // namespace kestrel
