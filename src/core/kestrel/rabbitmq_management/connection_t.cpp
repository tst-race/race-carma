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
#include <kestrel/rabbitmq_management/connection_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq_management/connection_t.hpp>
//

#ifdef __EMSCRIPTEN__

#include <cstring>
#include <string>
#include <thread>
#include <utility>

#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/from_hex.hpp>
#include <sst/catalog/from_hex_options.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <EncPkg.h>

#include <emscripten.h>
#include <emscripten/fetch.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/rabbitmq_management/link_t.hpp>
#include <kestrel/rabbitmq_management/plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq_management {

namespace {

struct context_t {
  easier_ta2_parent_ptr_t<connection_t> ptr;
  std::string request_body;
  context_t(easier_ta2_parent_ptr_t<connection_t> ptr)
      : ptr(std::move(ptr)) {
  }
};

} // namespace

extern "C" {

static void recv_success(emscripten_fetch_t * const fetch) {
  SST_ASSERT((fetch != nullptr));
  try {
    SST_ASSERT((fetch->userData != nullptr));
    sst::unique_ptr<context_t> context{
        static_cast<context_t *>(fetch->userData)};
    auto scope = context->ptr.lock();
    if (scope) {
      connection_t & connection = scope.parent();
      connection.recv_proc(&context, fetch, true);
    }
  } catch (...) {
  }
  emscripten_fetch_close(fetch);
}

static void recv_failure(emscripten_fetch_t * const fetch) {
  SST_ASSERT((fetch != nullptr));
  try {
    SST_ASSERT((fetch->userData != nullptr));
    sst::unique_ptr<context_t> context{
        static_cast<context_t *>(fetch->userData)};
    auto scope = context->ptr.lock();
    if (scope) {
      connection_t & connection = scope.parent();
      connection.recv_proc(&context, fetch, false);
    }
  } catch (...) {
  }
  emscripten_fetch_close(fetch);
}

static void recv_again(void * const arg) {
  try {
    SST_ASSERT((arg != nullptr));
    sst::unique_ptr<context_t> context{static_cast<context_t *>(arg)};
    auto scope = context->ptr.lock();
    if (scope) {
      connection_t & connection = scope.parent();
      connection.recv();
    }
  } catch (...) {
  }
}

} //

void connection_t::recv() {
  if (!link_type_t(this->link().properties().linkType).can_recv()) {
    return;
  }
  auto const p_attr = std::make_shared<emscripten_fetch_attr_t>();
  emscripten_fetch_attr_t & attr = *p_attr;
  emscripten_fetch_attr_init(&attr);
  static char const * const headers[] = {
      "Content-Type",
      "application/json",
      "Authorization",
      "Basic c3RlYWx0aDpTQ1Y3WXpuclBxanU=",
      nullptr};
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = recv_success;
  attr.onerror = recv_failure;
  attr.requestHeaders = headers;
  //  attr.withCredentials = EM_TRUE;
  //  attr.userName = this->link().addrinfo().username().c_str();
  //  attr.password = this->link().addrinfo().password().c_str();
  sst::unique_ptr<context_t> context{sst::in_place, this->ptr()};
  std::string url = "https://" + this->link().addrinfo().host() + ':'
                    + sst::to_string(this->link().addrinfo().port());
  if (!declared_queue_) {
    std::strcpy(attr.requestMethod, "PUT");
    // TODO: Need to use vhost(), not hardcode %2F.
    url += "/api/queues/%2F/" + this->link().addrinfo().queue();
    context->request_body =
        json_t{{"auto_delete", false},
               {"durable", false},
               {"arguments", {{"x-expires", 3600000}}}}
            .dump();
  } else {
    std::strcpy(attr.requestMethod, "POST");
    url +=
        "/api/queues/%2F/" + this->link().addrinfo().queue() + "/get";
    context->request_body = json_t{
        {"count", 10},
        {"ackmode", "ack_requeue_false"},
        {"encoding", "base64"}}.dump();
  }
  attr.requestData = context->request_body.c_str();
  attr.requestDataSize =
      sst::checked_cast<decltype(attr.requestDataSize)>(
          context->request_body.size());
  attr.userData = context.release();
  // If we don't use a thread here, the callbacks may never be called.
  std::thread([=] {
    emscripten_fetch(p_attr.get(), url.c_str());
  }).detach();
}

void connection_t::recv_proc(void * const p_context,
                             void * const p_fetch,
                             bool const success) {
  sst::unique_ptr<context_t> & context =
      *static_cast<sst::unique_ptr<context_t> *>(p_context);
  emscripten_fetch_t & fetch =
      *static_cast<emscripten_fetch_t *>(p_fetch);
  try {
    if (success) {
      if (!declared_queue_) {
        declared_queue_ = true;
      } else {
        json_t const json = sst::json::get_from_string<json_t>(
            std::string(fetch.data, fetch.numBytes));
        for (auto const & obj : json) {
          try {
            this->sdk().receiveEncPkg(
                SST_TEV_DEF(tracing_event_t),
                EncPkg(sst::from_hex(
                    obj.at("payload")
                        .template get_ref<std::string const &>(),
                    sst::from_hex_options().base(64))),
                std::vector<std::string>{},
                0);
          } catch (...) {
          }
        }
      }
    }
  } catch (...) {
  }
  emscripten_async_call(recv_again,
                        context.release(),
                        sst::rand_range(1000, 3000));
}

} // namespace rabbitmq_management
} // namespace kestrel

#else

#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq_management/link_t.hpp>
#include <kestrel/rabbitmq_management/plugin_t.hpp>

namespace kestrel {
namespace rabbitmq_management {

void connection_t::recv() {
  ;
}

void connection_t::recv_proc(void *, void *, bool) {
  ;
}

} // namespace rabbitmq_management
} // namespace kestrel

#endif
