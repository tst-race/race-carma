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

#ifndef KESTREL_SDK_SPAN_T_HPP
#define KESTREL_SDK_SPAN_T_HPP

//
// The usage of OpenTracing in the RACE system can be difficult to
// understand because of the system's distributed, non-hierarchical
// nature. Here are the key points:
//
//    1. The RACE system uses a trace to represent one ClrMsg
//       traveling through the system from one client node to
//       another client node. However, extra traces are sometimes
//       needed to model complex processing steps that belong to
//       multiple ClrMsg's, not a single ClrMsg.
//
//    2. Every span has zero or more parent spans and belongs to
//       exactly one trace.
//
//    3. A trace is really just a span with no parent spans. It
//       can be thought of as belonging to itself.
//
//    4. The only meaning of a span X being a parent of a span Y
//       is that X begins before Y begins. Y may end after X ends
//       and may even begin after X ends.
//
//    5. Considering (4), it is difficult to define when a trace
//       truly ends, as new child spans may be started after the
//       root span has ended. When a test harness of some kind is
//       in use, i.e., when a single process controls all clients
//       and knows when all ClrMsg's are sent and received, it can
//       indeed end the root span when the ClrMsg is received and
//       provide a natural end time for the trace. Otherwise,
//       another strategy must be used, such as designating a
//       particular span name to represent the end of the trace.
//       This applies to all extra traces described in (1).
//
//    6. When a new span is started with one or more parent spans,
//       it inherits the trace of exactly one parent. This parent
//       should be linked to the span with the ChildOf reference
//       type. Other parents should be linked with the FollowsFrom
//       reference type.
//

#include <kestrel/config.h>

#if CARMA_WITH_MOCK_SDK

#include <kestrel/sdk_span_id_t.hpp>
#include <kestrel/sdk_trace_id_t.hpp>

namespace kestrel {

struct sdk_span_t final {
  template<class... Args>
  sdk_span_t(Args &&...) {
  }

  sdk_trace_id_t trace_id() const noexcept {
    return sdk_trace_id_t{};
  }

  sdk_span_id_t span_id() const noexcept {
    return sdk_span_id_t{};
  }
};

} // namespace kestrel

#else // !CARMA_WITH_MOCK_SDK

#include <ClrMsg.h>
#include <EncPkg.h>
#include <OpenTracingHelpers.h>
#include <kestrel/sdk_span_id_t.hpp>
#include <kestrel/sdk_trace_id_t.hpp>
#include <functional>
#include <jaegertracing/SpanContext.h>
#include <memory>
#include <opentracing/propagation.h>
#include <opentracing/span.h>
#include <opentracing/tracer.h>
#include <stdexcept>
#include <utility>

namespace kestrel {

class sdk_span_t final {
  std::unique_ptr<opentracing::Span> span_;
  std::unique_ptr<opentracing::SpanContext> context_;
  sdk_trace_id_t trace_id_;
  sdk_span_id_t span_id_;

  static std::unique_ptr<opentracing::Span> start_span(
      opentracing::Tracer & tracer,
      std::string const & operation_name,
      std::vector<std::reference_wrapper<sdk_span_t const>> const &
          parents) {
    opentracing::StartSpanOptions options;
    opentracing::SpanReferenceType type =
        opentracing::SpanReferenceType::ChildOfRef;
    for (auto const & parent : parents) {
      options.references.emplace_back(type,
                                      parent.get().context_.get());
      type = opentracing::SpanReferenceType::FollowsFromRef;
    }
    std::unique_ptr<opentracing::Span> span =
        tracer.StartSpanWithOptions(operation_name, options);
    if (span == nullptr) {
      throw std::runtime_error("tracer.StartSpanWithOptions() failed");
    }
    return span;
  }

  static std::unique_ptr<opentracing::SpanContext>
  clone_context(opentracing::SpanContext const & context) {
    auto clone = context.Clone();
    if (clone == nullptr) {
      throw std::runtime_error("span.context().Clone() failed");
    }
    return clone;
  }

public:
  // Construct an empty span.
  sdk_span_t()
      : context_(new jaegertracing::SpanContext()),
        trace_id_(traceIdFromContext(*context_)),
        span_id_(spanIdFromContext(*context_)) {
  }

  // Inherit the trace and span IDs of a ClrMsg. We're merely acting as
  // an identifier of an existing span here, not actually starting or
  // owning a span, so span_ is null.
  explicit sdk_span_t(ClrMsg const & message)
      : context_(spanContextFromClrMsg(message)),
        trace_id_(message.getTraceId()),
        span_id_(message.getSpanId()) {
    // A comment in the <opentracing/span.h> header says that the
    // opentracing::SpanContext::Clone function can return a null
    // pointer (instead of throwing an exception, which we'd prefer), so
    // we'll just check every pointer we get back from library-ish calls
    // to be safe.
    if (context_ == nullptr) {
      throw std::runtime_error("spanContextFromClrMsg() failed");
    }
  }

  // Inherit the trace and span IDs of an EncPkg. We're merely acting as
  // an identifier of an existing span here, not actually starting or
  // owning a span, so span_ is null.
  explicit sdk_span_t(EncPkg const & message)
      : context_(spanContextFromEncryptedPackage(message)),
        trace_id_(message.getTraceId()),
        span_id_(message.getSpanId()) {
    if (context_ == nullptr) {
      throw std::runtime_error(
          "spanContextFromEncryptedPackage() failed");
    }
  }

  // Inherit the trace and span IDs of another sdk_span_t. We're merely
  // acting as an identifier of an existing span here, not actually
  // starting or owning a span, so span_ is null.
  sdk_span_t(sdk_span_t const & other)
      : context_(clone_context(*other.context_)),
        trace_id_(traceIdFromContext(*context_)),
        span_id_(spanIdFromContext(*context_)) {
  }
  sdk_span_t & operator=(sdk_span_t const & other) {
    span_.reset(nullptr);
    context_ = clone_context(*other.context_);
    trace_id_ = traceIdFromContext(*context_);
    span_id_ = spanIdFromContext(*context_);
    return *this;
  }

  // Take the content of another sdk_span_t. The moved-from span should
  // only be considered valid for destruction.
  sdk_span_t(sdk_span_t &&) noexcept = default;
  sdk_span_t & operator=(sdk_span_t &&) noexcept = default;

  // Start a new span from a list of parents. The new span will be
  // started inside the first parent's trace using a ChildOf reference
  // and linked to each remaining parent using a FollowsFrom reference.
  // If no parents are given, the new span will implicitly start a new
  // trace. The destructor will finish the span.
  explicit sdk_span_t(
      opentracing::Tracer & tracer,
      std::string const & operation_name,
      std::vector<std::reference_wrapper<sdk_span_t const>> const &
          parents)
      : span_(start_span(tracer, operation_name, parents)),
        context_(clone_context(span_->context())),
        trace_id_(traceIdFromContext(*context_)),
        span_id_(spanIdFromContext(*context_)) {
  }

  // Start a new span with no parents, implicitly starting a new trace.
  // This is a convenience wrapper around the previous constructor.
  explicit sdk_span_t(opentracing::Tracer & tracer,
                      std::string const & operation_name)
      : sdk_span_t(tracer, operation_name, {}) {
  }

  ~sdk_span_t() noexcept {
  }

  sdk_trace_id_t trace_id() const noexcept {
    return trace_id_;
  }

  sdk_span_id_t span_id() const noexcept {
    return span_id_;
  }
};

} // namespace kestrel

#endif // !CARMA_WITH_MOCK_SDK

#endif // #ifndef KESTREL_SDK_SPAN_T_HPP
