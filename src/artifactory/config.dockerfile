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

ARG CARMA_IMAGE=ghcr.io/tst-race/race-in-the-box/race-in-the-box
ARG CARMA_RIB_VERSION

FROM $CARMA_IMAGE:add-ci

RUN : \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -q -y update \
  && apt-get -q -y install \
       autoconf \
       automake \
       g++-8 \
       g++-8-aarch64-linux-gnu \
       gawk \
       gcc-8-aarch64-linux-gnu \
       libcurl4-openssl-dev \
       libgmp-dev \
       librabbitmq-dev \
       libssl-dev \
       libtool \
       make \
       perl \
       wget \
  && update-alternatives \
       --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 \
       --slave /usr/bin/g++ g++ /usr/bin/g++-8 \
;

RUN : \
  && for x in /usr/bin/aarch64-linux-gnu-*-8; do \
       y=${x%-8}; \
       x=${x##*/}; \
       ln -f -s ${x?} ${y?} || exit $?; \
     done \
;

ARG DOCKERFILE_THREADS
ENV DOCKERFILE_THREADS=${DOCKERFILE_THREADS?}

#-----------------------------------------------------------------------
# Build and install CMake
#-----------------------------------------------------------------------
#
# Any version that's already installed might be too old.
#

COPY cmake.tar.gz /x/

RUN : \
  && mkdir /x/cmake \
  && cd /x/cmake \
  && tar xzf ../cmake.tar.gz \
  && cd * \
  && ./bootstrap \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

#-----------------------------------------------------------------------

ARG CARMA_AMD64_CFLAGS='-O2 -march=x86-64'
ARG CARMA_AMD64_CXXFLAGS='-O2 -march=x86-64'

ARG CARMA_ARM64_CFLAGS='-O2'
ARG CARMA_ARM64_CXXFLAGS='-O2'
ARG CARMA_ARM64_CPPFLAGS='-I/arm64stage/include'
ARG CARMA_ARM64_LDFLAGS='-L/arm64stage/lib -L/arm64stage/lib/aarch64-linux-gnu'

COPY openssl.tar.gz /x/

RUN : \
  && mkdir /x/openssl-arm64 \
  && cd /x/openssl-arm64 \
  && tar xzf ../openssl.tar.gz \
  && cd * \
  && ./Configure \
       --cross-compile-prefix=aarch64-linux-gnu- \
       --prefix=/arm64stage \
       no-buildtest-c++ \
       no-dso \
       no-external-tests \
       no-shared \
       no-tests \
       no-unit-test \
       threads \
       linux-aarch64 \
  && make -j${DOCKERFILE_THREADS?} \
  && make install_sw \
;

COPY curl.tar.gz /x/

RUN : \
  && mkdir /x/curl-arm64 \
  && cd /x/curl-arm64 \
  && tar xzf ../curl.tar.gz \
  && cd * \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       --with-openssl \
       --without-brotli \
       --without-libpsl \
       --without-zlib \
       --without-zstd \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CPPFLAGS="${CARMA_ARM64_CPPFLAGS?}" \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       LDFLAGS="${CARMA_ARM64_LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY gmp.tar.any /x/

RUN : \
  && mkdir /x/gmp-amd64 \
  && cd /x/gmp-amd64 \
  && tar xf ../gmp.tar.any \
  && cd * \
  && ./configure \
       --disable-shared \
       --enable-static \
       --with-pic \
       CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

RUN : \
  && mkdir /x/gmp-arm64 \
  && cd /x/gmp-arm64 \
  && tar xf ../gmp.tar.any \
  && cd * \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       --with-pic \
       CC=aarch64-linux-gnu-gcc \
       CC_FOR_BUILD=cc \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CXX=aarch64-linux-gnu-g++ \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY ntl.tar.gz /x/

RUN : \
  && mkdir /x/ntl-amd64 \
  && cd /x/ntl-amd64 \
  && tar xzf ../ntl.tar.gz \
  && cd */src \
  && ./configure \
       CPPFLAGS='-I/usr/local/include' \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       LDFLAGS='-L/usr/local/lib' \
       NATIVE=off \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

RUN : \
  && mkdir /x/ntl-arm64 \
  && cd /x/ntl-arm64 \
  && tar xzf ../ntl.tar.gz \
  && cd */src \
  && sed -i 's|`\./GenConfigInfo`|"(gcc,2011,x86,linux)"|' DoConfig \
  && sed -i 's|return Exec("Check[^"]*")|return 1|' DoConfig \
  && ./configure \
       CXX=aarch64-linux-gnu-g++ \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?} -ffp-contract=off" \
       DEF_PREFIX=/arm64stage \
       NATIVE=off \
       NTL_GF2X_LIB=off \
       NTL_GMP_LIP=on \
       NTL_SAFE_VECTORS=on \
       NTL_STD_CXX11=on \
       NTL_THREADS=on \
       SHARED=off \
       TUNE=generic \
  && sed -i 's|if \./CheckFeatures|if test -f Check$f.have|' MakeCheckFeatures \
  && touch CheckALIGNED_ARRAY.have \
  && touch CheckBUILTIN_CLZL.have \
  && touch CheckCHRONO_TIME.have \
  && touch CheckCOPY_TRAITS1.have \
  && touch CheckCOPY_TRAITS2.have \
  && touch CheckLL_TYPE.have \
  && touch CheckPOSIX_TIME.have \
  && make \
       CPPFLAGS='-I/usr/local/include' \
       CXX=g++ \
       CXXFLAGS= \
       DEF_PREFIX=/usr/local \
       LDFLAGS='-L/usr/local/lib' \
       setup-phase \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY rabbitmq.tar.gz /x/

RUN : \
  && mkdir /x/rabbitmq-arm64 \
  && cd /x/rabbitmq-arm64 \
  && tar xzf ../rabbitmq.tar.gz \
  && cd * \
  && extra_flags= \
  && extra_flags="${extra_flags?} ${CARMA_ARM64_CPPFLAGS?}" \
  && extra_flags="${extra_flags?} ${CARMA_ARM64_LDFLAGS?}" \
  && /usr/local/bin/cmake \
       -D BUILD_API_DOCS=OFF \
       -D BUILD_EXAMPLES=OFF \
       -D BUILD_SHARED_LIBS=OFF \
       -D BUILD_STATIC_LIBS=ON \
       -D BUILD_TESTING=OFF \
       -D BUILD_TESTS=OFF \
       -D BUILD_TOOLS=OFF \
       -D BUILD_TOOLS_DOCS=OFF \
       -D CMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
       -D CMAKE_CXX_FLAGS="${CARMA_ARM64_CXXFLAGS?} ${extra_flags?}" \
       -D CMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
       -D CMAKE_C_FLAGS="${CARMA_ARM64_CFLAGS?} ${extra_flags?}" \
       -D CMAKE_INSTALL_PREFIX=/arm64stage \
       -D ENABLE_SSL_SUPPORT=ON \
       -D OPENSSL_CRYPTO_LIBRARY=/arm64stage/lib/libcrypto.a \
       -D OPENSSL_INCLUDE_DIR=/arm64stage/include \
       -D OPENSSL_SSL_LIBRARY=/arm64stage/lib/libssl.a \
       -D OPENSSL_USE_STATIC_LIBS=TRUE \
       -D RUN_SYSTEM_TESTS=OFF \
       . \
  && /usr/local/bin/cmake \
       --build . \
       --config Release \
       --target install \
;

COPY sqlite.tar.any /x/

RUN : \
  && mkdir /x/sqlite-amd64 \
  && cd /x/sqlite-amd64 \
  && tar xf ../sqlite.tar.any \
  && cd * \
  && ./configure \
       --disable-shared \
       --enable-static \
       CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include" \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && rm /usr/local/lib/libsqlite3.la \
;

RUN : \
  && mkdir /x/sqlite-arm64 \
  && cd /x/sqlite-arm64 \
  && tar xf ../sqlite.tar.any \
  && cd * \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CPPFLAGS="-I/arm64stage/include" \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       LDFLAGS="${CARMA_ARM64_LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && rm /arm64stage/lib/libsqlite3.la \
;

COPY libsodium.gitbundle /x/

RUN : \
  && mkdir /x/libsodium-amd64 \
  && cd /x/libsodium-amd64 \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && ./autogen.sh \
  && ./configure \
       --disable-pie \
       --disable-shared \
       --enable-static \
       --with-pic \
       CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include" \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib" \
  && make -j${DOCKERFILE_THREADS?} V=1 \
  && make V=1 install \
;

RUN : \
  && mkdir /x/libsodium-arm64 \
  && cd /x/libsodium-arm64 \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && ./autogen.sh \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-pie \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       --with-pic \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CPPFLAGS="-I/arm64stage/include" \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       LDFLAGS="${CARMA_ARM64_LDFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} V=1 \
  && make V=1 install \
;

COPY sst.tar.gz /x/

RUN : \
  && mkdir /x/sst-amd64 \
  && cd /x/sst-amd64 \
  && tar xzf ../sst.tar.gz \
  && cd * \
  && ./configure \
       --disable-shared \
       --enable-static \
       --with-build-groups=cpp \
       --without-avx-512-f \
       CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include" \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib" \
       LIB_CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

RUN : \
  && mkdir /x/sst-arm64 \
  && cd /x/sst-arm64 \
  && tar xzf ../sst.tar.gz \
  && cd * \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       --with-build-groups=cpp \
       --without-avx-512-f \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CPPFLAGS="-I/arm64stage/include" \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       LDFLAGS="${CARMA_ARM64_LDFLAGS?}" \
       LIB_CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY \
  carma.tar.gz \
  config.dockerfile.make1 \
/x/

RUN ( : \
  && mkdir /x/carma-amd64 \
  && cd /x/carma-amd64 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && ./configure \
       --disable-shared \
       --enable-static \
       CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include" \
       CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib" \
       LIB_CFLAGS="${CARMA_AMD64_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_AMD64_CXXFLAGS?}" \
  && . ../../config.dockerfile.make1 \
)

RUN ( : \
  && mkdir /x/carma-arm64 \
  && cd /x/carma-arm64 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=aarch64-linux-gnu \
       --prefix=/arm64stage \
       CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       CPPFLAGS="-I/arm64stage/include" \
       CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
       LDFLAGS="${CARMA_ARM64_LDFLAGS?}" \
       LIB_CFLAGS="${CARMA_ARM64_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_ARM64_CXXFLAGS?}" \
  && . ../../config.dockerfile.make1 \
)
