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

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/mkdir_p_new.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/to_integer.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/what.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten/val.h>
#include <kestrel/carma/generate_configs.hpp>
#include <kestrel/engine_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/kestrel_cli_args_t.hpp>
#include <kestrel/kestrel_genesis.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/rabbitmq/generate_configs.hpp>
#include <kestrel/slugify.hpp>

extern "C" {

extern void my_js();

void downloadSucceeded(emscripten_fetch_t * fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n",
         fetch->numBytes,
         fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  my_js();
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t * fetch) {
  size_t len = emscripten_fetch_get_response_headers_length(fetch);
  printf("Downloading %s failed, HTTP failure status code: %d. And "
         "text %s\n",
         fetch->url,
         (int)fetch->status,
         fetch->statusText);

  emscripten_fetch_close(fetch); // Also free data on failure.
}

void dopost(char * queue_name) {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "POST");
  //attr.withCredentials = EM_TRUE;
  //attr.userName = "guest";
  //attr.password = "guest";
  std::string q(queue_name);
  std::string x = "{\"properties\": {}, \"routing_key\": \"" + q
                  + "\", \"payload\": \"Hello, XHR world!\", "
                    "\"payload_encoding\": \"string\"}";
  attr.requestData =
      x.c_str(); // TODO: look for a better way to do this
  attr.requestDataSize = strlen(attr.requestData);
  static char const * const request_headers[] =
      {"Authorization", "Basic Z3Vlc3Q6Z3Vlc3Q=", 0};

  attr.requestHeaders = request_headers;
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = downloadSucceeded;
  attr.onerror = downloadFailed;
  emscripten_fetch(
      &attr,
      "http://192.168.1.55:8080/api/exchanges/%2F/amq.default/publish");
}
}

namespace {

using namespace kestrel;

//----------------------------------------------------------------------
//
// Every kestrel_* function should put "try {} catch (...) {}" around
// its entire body and call this function in the catch.
//

void top_level_error() noexcept {
  static std::string e;
  bool have_e = false;
  try {
    e = sst::what();
    have_e = true;
  } catch (...) {
  }
  MAIN_THREAD_ASYNC_EM_ASM(
      {
        console.error("Error: "
                      + ($0 ? UTF8ToString($1) : "Unknown error."));
      },
      have_e,
      e.c_str());
}

//----------------------------------------------------------------------

engine_t & engine() {
  static engine_t * p = nullptr;
  if (!p) {
    // TODO: Put the engine dir under idbfs_dir eventually.
    p = new engine_t("/engine");
    if (!p) {
      throw std::runtime_error("Engine creation failed");
    }
  }
  return *p;
}

//----------------------------------------------------------------------
// kestrel_clients
//----------------------------------------------------------------------
//
// Returns the list of all known client PSN slugs, including the PSN
// slug of this node if this node is a client. The list is sorted in
// non-slug order and has no duplicates (slug or not). It is possible
// that the list is empty.
//

std::vector<std::string> kestrel_clients() {
  std::shared_ptr<std::vector<psn_t> const> const xs =
      engine().config().clients();
  std::vector<std::string> ys;
  for (psn_t const & x : *xs) {
    ys.push_back(x.to_path_slug());
  }
  return ys;
}

//----------------------------------------------------------------------
// kestrel_generate_configs
//----------------------------------------------------------------------
//
// Runs genesis.
//
// options is a JavaScript object that specifies the genesis parameters.
// All members of options must be strings, even if they are numeric. The
// following members are supported:
//
//    output_dir (required)
//       A nonexistent directory to create and output the
//       resulting files to (using Emscripten's Filesystem API).
//       The resulting directory will be such that each immediate
//       subdirectory represents a node. The subdirectory name is
//       the slugified node name, and the content is exactly what
//       should be zipped for that node.
//
//    num_clients (required)
//       The number of clients.
//
//    num_servers (required)
//       The number of servers.
//
// callback_name should be the name of a global JavaScript function to
// call when the operation is complete. The function will be passed two
// parameters: error and result. error will be null to mean the call was
// successful, or a string that contains an error message if not. If the
// call was successful, result will be a map of (psn_slug, dir) pairs,
// where psn_slug is a PSN slug and dir is the directory whose content
// should be zipped for that PSN.
//
// temp_name should be the name of a global JavaScript variable to use
// for temporary purposes.
//

void kestrel_generate_configs(emscripten::val options,
                              std::string const callback_name,
                              std::string const temp_name) {
  try {

    std::list<std::string> argv1;

    kestrel_cli_args_t kestrel_cli_args;

    json_t kestrel_genesis_cli_args;

    if (!options.hasOwnProperty("num_clients")) {
      throw std::runtime_error("Missing num_clients");
    }
    kestrel_genesis_cli_args["num_clients"] =
        options["num_clients"].as<std::string>();

    if (!options.hasOwnProperty("num_servers")) {
      throw std::runtime_error("Missing num_servers");
    }
    kestrel_genesis_cli_args["num_servers"] =
        options["num_servers"].as<std::string>();

    std::string output_dir;
    if (!options.hasOwnProperty("output_dir")) {
      throw std::runtime_error("Missing output_dir");
    }
    kestrel_cli_args.work_dir(options["output_dir"].as<std::string>());

    std::thread([=] {
      try {

        kestrel_genesis(
            kestrel_cli_args,
            sst::json::get_as<genesis_spec_t>(kestrel_genesis_cli_args),
            std::cout);

        MAIN_THREAD_EM_ASM(({ window[UTF8ToString($0)] = {}; }),
                           temp_name.c_str());

        {
          for (auto it =
                   sst::dir_it(kestrel_cli_args.work_dir() + "/nodes");
               it != it.end();
               ++it) {
            char const * const dir = it->c_str();
            char const * const psn_slug = dir + it.prefix();
            MAIN_THREAD_EM_ASM(
                ({
                  window[UTF8ToString($0)][UTF8ToString($1)] =
                      UTF8ToString($2);
                }),
                temp_name.c_str(),
                psn_slug,
                dir);
          }
        }

        MAIN_THREAD_EM_ASM(
            {
              window[UTF8ToString($0)](null, window[UTF8ToString($1)]);
            },
            callback_name.c_str(),
            temp_name.c_str());

      } catch (...) {
        sst::what const error;
        MAIN_THREAD_EM_ASM(
            { window[UTF8ToString($0)](UTF8ToString($1)); },
            callback_name.c_str(),
            error.c_str());
      }
    }).detach();

  } catch (...) {
    sst::what const error;
    EM_ASM({ window[UTF8ToString($0)](UTF8ToString($1)); },
           callback_name.c_str(),
           error.c_str());
  }
}

//----------------------------------------------------------------------
// kestrel_on_recv_clrmsg
//----------------------------------------------------------------------

void kestrel_on_recv_clrmsg(std::string const callback) {
  try {
    engine().on_recv_clrmsg(
        [=](engine_t::on_recv_clrmsg_info_t const & info) {
          // TODO: This is a bit of a mess here, be careful regarding
          //       _data, _slug, and _text forms. There's something
          //       called "HEAPU8" or something we can use? We really
          //       just want to pass the JavaScript some Uint8Array's
          //       for it to do what it wants with.
          std::string const psn_slug = slugify(info.psn);
          MAIN_THREAD_EM_ASM(({
                               const callback = UTF8ToString($0);
                               const psn_slug = UTF8ToString($1);
                               const msg_text = UTF8ToString($2);
                               const unix_time_us = $3;
                               window[callback]({
                                 psn_slug: psn_slug,
                                 msg_text: msg_text,
                                 unix_time_us: unix_time_us
                               });
                             }),
                             callback.c_str(),
                             psn_slug.c_str(),
                             info.msg.c_str(),
                             static_cast<double>(info.unix_time_us));
        });
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------
// kestrel_on_start
//----------------------------------------------------------------------

void kestrel_on_start(std::string const callback) {
  try {
    engine().on_start([=](engine_t::on_start_info_t const & info) {
      MAIN_THREAD_EM_ASM(({
                           const callback = UTF8ToString($0);
                           const psn_slug = UTF8ToString($1);
                           window[callback]({
                             psn_slug: psn_slug,
                           });
                         }),
                         callback.c_str(),
                         info.psn_slug.c_str());
    });
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------
// kestrel_on_stop
//----------------------------------------------------------------------
//
// Registers a callback for when the engine stops. f should be the name
// of a global JavaScript function to use as the callback. The callback
// will always be called on the main thread.
//

void kestrel_on_stop(std::string f) {
  try {
    static std::string g;
    static std::string e;
    g = std::move(f);
    engine().on_stop([](char const * const error) {
      bool const have_error = error;
      bool have_e = false;
      if (have_error) {
        try {
          e = error;
          have_e = true;
        } catch (...) {
        }
      }
      MAIN_THREAD_ASYNC_EM_ASM(
          {
            window[UTF8ToString($0)](($1 ?
                                          "Error: " + UTF8ToString($2) :
                                      $3 ? "Error: Unknown error." :
                                           null));
          },
          g.c_str(),
          have_e,
          e.c_str(),
          have_error);
    });
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------
// kestrel_send
//----------------------------------------------------------------------

void kestrel_send(std::string psn, std::string message) {
  try {
    engine().processClrMsg(std::move(psn), std::move(message));
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------
// kestrel_start
//----------------------------------------------------------------------
//
// Starts the engine.
//

void kestrel_start() {
  try {
    engine().start();
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------
// kestrel_stop
//----------------------------------------------------------------------

void kestrel_stop() {
  try {
    engine().stop();
  } catch (...) {
    top_level_error();
  }
}

//----------------------------------------------------------------------

} // namespace

EMSCRIPTEN_BINDINGS(kestrel) {

  using namespace emscripten;

  register_vector<std::string>("StringVector");

  function("kestrel_clients", &kestrel_clients);
  function("kestrel_generate_configs", &kestrel_generate_configs);
  function("kestrel_on_recv_clrmsg", &kestrel_on_recv_clrmsg);
  function("kestrel_on_start", &kestrel_on_start);
  function("kestrel_on_stop", &kestrel_on_stop);
  function("kestrel_send", &kestrel_send);
  function("kestrel_start", &kestrel_start);
  function("kestrel_stop", &kestrel_stop);

} //
