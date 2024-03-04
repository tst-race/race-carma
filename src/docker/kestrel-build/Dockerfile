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

FROM ubuntu:22.04

RUN : \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -q -y update \
  && apt-get -q -y install \
       autoconf \
       automake \
       build-essential \
       g++ \
       gawk \
       gcc \
       git \
       gnupg \
       gnupg1 \
       jq \
       libcurl4-openssl-dev \
       libgmp-dev \
       libntl-dev \
       librabbitmq-dev \
       libssl-dev \
       libtool \
       m4 \
       make \
       openssl \
       sshpass \
       texinfo \
       texlive \
       wget \
;

ARG DOCKERFILE_THREADS
ENV DOCKERFILE_THREADS=${DOCKERFILE_THREADS?}

COPY sqlite.tar.any /x/
RUN : \
  && mkdir /x/sqlite \
  && cd /x/sqlite \
  && tar xf ../sqlite.tar.any \
  && cd * \
  && CFLAGS= \
  && CFLAGS="$CFLAGS -O0" \
  && CFLAGS="$CFLAGS -fPIC" \
  && CPPFLAGS= \
  && CPPFLAGS="$CPPFLAGS -I/usr/local/include" \
  && CXXFLAGS= \
  && CXXFLAGS="$CXXFLAGS -O0" \
  && CXXFLAGS="$CXXFLAGS -fPIC" \
  && LDFLAGS= \
  && LDFLAGS="$LDFLAGS -L/usr/local/lib" \
  && ./configure \
       --disable-shared \
       --enable-static \
       CFLAGS="${CFLAGS?}" \
       CPPFLAGS="${CPPFLAGS?}" \
       CXXFLAGS="${CXXFLAGS?}" \
       LDFLAGS="${LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && rm /usr/local/lib/libsqlite3.la \
;

COPY libsodium.gitbundle /x/
RUN : \
  && mkdir /x/libsodium \
  && cd /x/libsodium \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && CFLAGS= \
  && CFLAGS="$CFLAGS -O0" \
  && CPPFLAGS= \
  && CPPFLAGS="$CPPFLAGS -I/usr/local/include" \
  && CXXFLAGS= \
  && CXXFLAGS="$CXXFLAGS -O0" \
  && LDFLAGS= \
  && LDFLAGS="$LDFLAGS -L/usr/local/lib" \
  && ./autogen.sh \
  && ./configure \
       --disable-pie \
       --disable-shared \
       --enable-static \
       --with-pic \
       CFLAGS="${CFLAGS?}" \
       CPPFLAGS="${CPPFLAGS?}" \
       CXXFLAGS="${CXXFLAGS?}" \
       LDFLAGS="${LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} V=1 \
  && make V=1 install \
;

COPY sst.tar.gz /x/

RUN : \
  && cd /x \
  && tar xf sst.tar.gz \
  && cd sst-* \
  && CFLAGS= \
  && CFLAGS="$CFLAGS -O0" \
  && CPPFLAGS= \
  && CPPFLAGS="$CPPFLAGS -I/usr/local/include" \
  && CXXFLAGS= \
  && CXXFLAGS="$CXXFLAGS -O0" \
  && LDFLAGS= \
  && LDFLAGS="$LDFLAGS -L/usr/local/lib" \
  && ./configure \
       --build=x86_64-unknown-linux \
       --disable-invalid-argument-checks \
       --disable-static \
       --disable-undefined-behavior-checks \
       --enable-shared \
       --host=x86_64-unknown-linux \
       --prefix=/usr \
       --with-build-groups=cpp \
       CFLAGS="${CFLAGS?}" \
       CPPFLAGS="${CPPFLAGS?}" \
       CXXFLAGS="${CXXFLAGS?}" \
       JAR= \
       JAVAC= \
       JDEPS= \
       LDFLAGS="${LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && make DESTDIR=/x/overlay install \
;

# TODO: These aren't passed in via --build-arg yet.
ARG CARMA_CFLAGS=
ARG CARMA_CXXFLAGS=

COPY carma.tar.gz /x/

RUN : \
  && mkdir /x/carma \
  && cd /x/carma \
  && tar xf ../carma.tar.gz \
  && cd * \
  && CFLAGS= \
  && CFLAGS="$CFLAGS $CARMA_CFLAGS" \
  && CPPFLAGS= \
  && CPPFLAGS="$CPPFLAGS -I/usr/local/include" \
  && CXXFLAGS= \
  && CXXFLAGS="$CXXFLAGS $CARMA_CXXFLAGS" \
  && LDFLAGS= \
  && LDFLAGS="$LDFLAGS -L/usr/local/lib" \
  && ./configure \
       --build=x86_64-unknown-linux \
       --disable-invalid-argument-checks \
       --disable-static \
       --disable-undefined-behavior-checks \
       --enable-shared \
       --host=x86_64-unknown-linux \
       --prefix=/usr \
       CFLAGS="${CFLAGS?}" \
       CPPFLAGS="${CPPFLAGS?}" \
       CXXFLAGS="${CXXFLAGS?}" \
       LDFLAGS="${LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && make DESTDIR=/x/overlay install \
;
