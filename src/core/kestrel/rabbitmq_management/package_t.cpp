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
#include <kestrel/rabbitmq_management/package_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq_management/package_t.hpp>
//

#ifdef __EMSCRIPTEN__

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <utility>

#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_hex_options.hpp>
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
  easier_ta2_parent_ptr_t<package_t> ptr;
  std::string request_body;
  bool is_declare = false;
  context_t(easier_ta2_parent_ptr_t<package_t> ptr)
      : ptr(std::move(ptr)) {
  }
};

} // namespace

extern "C" {

static void send_success(emscripten_fetch_t * const fetch) {
  SST_ASSERT((fetch != nullptr));
  try {
    SST_ASSERT((fetch->userData != nullptr));
    sst::unique_ptr<context_t> context{
        static_cast<context_t *>(fetch->userData)};
    auto scope = context->ptr.lock();
    if (scope) {
      package_t & package = scope.parent();
      package.send_proc(&context, true);
    }
  } catch (...) {
  }
  emscripten_fetch_close(fetch);
}

static void send_failure(emscripten_fetch_t * const fetch) {
  SST_ASSERT((fetch != nullptr));
  try {
    SST_ASSERT((fetch->userData != nullptr));
    sst::unique_ptr<context_t> context{
        static_cast<context_t *>(fetch->userData)};
    auto scope = context->ptr.lock();
    if (scope) {
      package_t & package = scope.parent();
      package.send_proc(&context, false);
    }
  } catch (...) {
  }
  emscripten_fetch_close(fetch);
}
}

void package_t::send() {
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
  attr.onsuccess = send_success;
  attr.onerror = send_failure;
  attr.requestHeaders = headers;
  //  attr.withCredentials = EM_TRUE;
  //  attr.userName = this->link().addrinfo().username().c_str();
  //  attr.password = this->link().addrinfo().password().c_str();
  sst::unique_ptr<context_t> context{sst::in_place, this->ptr()};
  std::string url = "https://" + this->link().addrinfo().host() + ':'
                    + sst::to_string(this->link().addrinfo().port());
  if (!this->connection().declared_queue_) {
    std::strcpy(attr.requestMethod, "PUT");
    // TODO: Need to use vhost(), not hardcode %2F.
    url += "/api/queues/%2F/" + this->link().addrinfo().queue();
    context->request_body =
        json_t{{"auto_delete", false},
               {"durable", false},
               {"arguments", {{"x-expires", 3600000}}}}
            .dump();
    context->is_declare = true;
  } else {
    std::strcpy(attr.requestMethod, "POST");
    url += "/api/exchanges/%2F/amq.default/publish";
    context->request_body = json_t{
        {"properties", json_t::object()},
        {"routing_key", this->link().addrinfo().queue()},
        {"payload",
         sst::to_hex(this->data(),
                     sst::to_hex_options().base(64).pad(true))},
        {"payload_encoding",
         "base64"}}.dump();
  }
  attr.requestData = context->request_body.c_str();
  attr.requestDataSize =
      sst::checked_cast<decltype(attr.requestDataSize)>(
          context->request_body.size());
  attr.userData = context.release();
  // TODO: This sleep is a hack. Without it, we might spam thread
  //       creations so fast that it just doesn't work. We need better
  //       code design to deal with this that's tailored for Emscripten.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // If we don't use a thread here, the callbacks may never be called.
  std::thread([=] {
    emscripten_fetch(p_attr.get(), url.c_str());
  }).detach();
}

void package_t::send_proc(void * const p_context, bool const success) {
  sst::unique_ptr<context_t> & context =
      *static_cast<sst::unique_ptr<context_t> *>(p_context);
  if (success) {
    this->connection().declared_queue_ = true;
    if (context->is_declare) {
      send();
    } else {
      this->fire_status_event(package_status_t::received());
    }
  } else {
    this->fire_status_event(package_status_t::failed_generic());
  }
}

} // namespace rabbitmq_management
} // namespace kestrel

#else

namespace kestrel {
namespace rabbitmq_management {

void package_t::send() {
  ;
}

void package_t::send_proc(void *, bool) {
  ;
}

} // namespace rabbitmq_management
} // namespace kestrel

#endif
