#
# Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an "AS
# IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language
# governing permissions and limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#

ARG BASE_NAME

FROM ${BASE_NAME?} AS base

SHELL ["/bin/bash", "-c"]

ARG DOCKERFILE_THREADS
ENV DOCKERFILE_THREADS=${DOCKERFILE_THREADS?}

ARG COMMON_FLAGS="-O2 -pthread -fexceptions -sPTHREAD_POOL_SIZE=4 -sFETCH -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=50MB -s MAXIMUM_MEMORY=1000MB"
ARG MY_CPPFLAGS="-I/x/stage/include"
ARG MY_CFLAGS="${COMMON_FLAGS?}"
ARG MY_CXXFLAGS="${COMMON_FLAGS?}"
ARG MY_LDFLAGS="-L/x/stage/lib ${COMMON_FLAGS?}"

COPY kestrel.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && if test -f /x/kestrel.tar.gz.old; then : \
       && cd /x/kestrel \
       && tar xzf /x/kestrel.tar.gz \
            --newer-mtime /x/kestrel.tar.gz.old \
            --strip-components 1 \
       && mv -f /x/kestrel.tar.gz /x/kestrel.tar.gz.old \
     ; else : \
       && mkdir /x/kestrel \
       && cd /x/kestrel \
       && tar xzf /x/kestrel.tar.gz \
            --strip-components 1 \
       && mv -f /x/kestrel.tar.gz /x/kestrel.tar.gz.old \
       && emconfigure ./configure \
            --build=x86_64-pc-linux-gnu \
            --disable-shared \
            --enable-static \
            --host=none \
            --prefix=/x/stage \
            CFLAGS="${MY_CFLAGS?}" \
            CPPFLAGS="${MY_CPPFLAGS?}" \
            CXXFLAGS="${MY_CXXFLAGS?}" \
            LDFLAGS="${MY_LDFLAGS?}" \
     ; fi \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && cd /x/kestrel \
  && emcc \
       -Isrc/core \
       -Isrc/misc \
       -Isrc/sdk \
       ${MY_CPPFLAGS} \
       ${MY_CXXFLAGS} \
       ${MY_LDFLAGS} \
       -o src/web/kestrel.js \
       src/web/kestrel.cpp \
       -lembind \
       -lidbfs.js \
       -lcarma \
       -lsst \
       -lntl \
       -lsodium \
       -lsqlite3 \
       -lgmp \
       -lcrypto \
;

RUN : \
  && mkdir -p /x/out \
  && cp \
       /x/kestrel/src/web/*.css \
       /x/kestrel/src/web/*.html \
       /x/kestrel/src/web/*.js \
       /x/kestrel/src/web/*.svg \
       /x/kestrel/src/web/*.wasm \
       /x/out \
;

FROM scratch
COPY --from=base / /
CMD ["/bin/sh", "-c"]
