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

ARG CARMA_IMAGE=ghcr.io/tst-race/race-core/race-sdk
ARG CARMA_RACE_VERSION

FROM $CARMA_IMAGE:main-x64host-linux-aarch64

ARG CARMA_NDK_ARCH
ARG CARMA_NDK_ARCH2

RUN : \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -q -y update \
  && apt-get -q -y install \
       autoconf \
       automake \
       gawk \
       libtool \
;

ARG DOCKERFILE_THREADS
ENV DOCKERFILE_THREADS=${DOCKERFILE_THREADS?}

ARG CARMA_X64_CC=clang
ARG CARMA_X64_CFLAGS='-O2 -fPIC -march=x86-64'
ARG CARMA_X64_CXX=clang++
ARG CARMA_X64_CXXFLAGS='-O2 -fPIC -march=x86-64'

ARG CARMA_ARM64_CC=aarch64-linux-gnu-gcc
ARG CARMA_ARM64_CFLAGS='-O2 -fPIC'
ARG CARMA_ARM64_CXX=aarch64-linux-gnu-g++
ARG CARMA_ARM64_CXXFLAGS='-O2 -fPIC'

COPY ntl.tar.gz /x/
RUN : \
  && rm -f -r /usr/local/lib/libntl* \
  && rm -f -r /usr/local/include/NTL \
  && mkdir /x/ntl \
  && cd /x/ntl \
  && tar xzf ../ntl.tar.gz \
  && cd */src \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && sed -i 's|`\./GenConfigInfo`|"(clang,2011,x86,linux)"|' DoConfig \
  && sed -i 's|return Exec("Check[^"]*")|return 1|' DoConfig \
  && ./configure \
       CXX="${CARMA_CXX?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?} -ffp-contract=off" \
       DEF_PREFIX=/usr/local \
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
       CPPFLAGS= \
       CXX=clang++ \
       CXXFLAGS= \
       DEF_PREFIX=/usr/local \
       setup-phase \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY rabbitmq.tar.gz /x/
RUN : \
  && mkdir /x/rabbitmq \
  && cd /x/rabbitmq \
  && tar xzf ../rabbitmq.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && cmake \
       -D BUILD_API_DOCS=OFF \
       -D BUILD_EXAMPLES=OFF \
       -D BUILD_SHARED_LIBS=OFF \
       -D BUILD_STATIC_LIBS=ON \
       -D BUILD_TESTING=OFF \
       -D BUILD_TESTS=OFF \
       -D BUILD_TOOLS=OFF \
       -D BUILD_TOOLS_DOCS=OFF \
       -D CMAKE_CXX_COMPILER="${CARMA_CXX?}" \
       -D CMAKE_CXX_FLAGS="${CARMA_CXXFLAGS?}" \
       -D CMAKE_C_COMPILER="${CARMA_CC?}" \
       -D CMAKE_C_FLAGS="${CARMA_CFLAGS?}" \
       -D CMAKE_INSTALL_PREFIX=/usr/local \
       -D ENABLE_SSL_SUPPORT=ON \
       -D RUN_SYSTEM_TESTS=OFF \
       . \
  && cmake --build . --config Release --target install \
;

COPY sqlite.tar.any /x/
RUN : \
  && mkdir /x/sqlite \
  && cd /x/sqlite \
  && tar xf ../sqlite.tar.any \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
       unset build_triplet; \
       unset host_triplet; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
       build_triplet=x86_64-pc-linux-gnu; \
       host_triplet=aarch64-pc-linux-gnu; \
     esac \
  && ./configure \
       ${build_triplet+--build="${build_triplet?}"} \
       ${host_triplet+--host="${host_triplet?}"} \
       --disable-shared \
       --enable-static \
       CC="${CARMA_CC?}" \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include -I/linux/${CARMA_NDK_ARCH?}/include" \
       LDFLAGS="-L/usr/local/lib -L/linux/${CARMA_NDK_ARCH?}/lib" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY libsodium.gitbundle /x/
RUN : \
  && mkdir /x/libsodium \
  && cd /x/libsodium \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
       unset build_triplet; \
       unset host_triplet; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
       build_triplet=x86_64-pc-linux-gnu; \
       host_triplet=aarch64-pc-linux-gnu; \
     esac \
  && ./autogen.sh \
  && ./configure \
       ${build_triplet+--build="${build_triplet?}"} \
       ${host_triplet+--host="${host_triplet?}"} \
       --disable-pie \
       --disable-shared \
       --enable-static \
       --with-pic \
       CC="${CARMA_CC?}" \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include -I/linux/${CARMA_NDK_ARCH?}/include" \
       CXX="${CARMA_CXX?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib -L/linux/${CARMA_NDK_ARCH?}/lib" \
  && make -j${DOCKERFILE_THREADS?} V=1 \
  && make V=1 install \
;

COPY sst.tar.gz /x/
RUN : \
  && mkdir /x/sst \
  && cd /x/sst \
  && tar xzf ../sst.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
       unset build_triplet; \
       unset host_triplet; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
       build_triplet=x86_64-pc-linux-gnu; \
       host_triplet=aarch64-pc-linux-gnu; \
     esac \
  && ./configure \
       ${build_triplet+--build="${build_triplet?}"} \
       ${host_triplet+--host="${host_triplet?}"} \
       --disable-shared \
       --enable-static \
       --with-build-groups=cpp \
       --without-avx-512-f \
       CC="${CARMA_CC?}" \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include -I/linux/${CARMA_NDK_ARCH?}/include" \
       CXX="${CARMA_CXX?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib -L/linux/${CARMA_NDK_ARCH?}/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY \
  carma.tar.gz \
  linux.dockerfile.make1 \
  linux.dockerfile.make2 \
/x/

RUN ( : \
  && mkdir /x/carma-1 \
  && cd /x/carma-1 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
       unset build_triplet; \
       unset host_triplet; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
       build_triplet=x86_64-pc-linux-gnu; \
       host_triplet=aarch64-pc-linux-gnu; \
     esac \
  && ./configure \
       ${build_triplet+--build="${build_triplet?}"} \
       ${host_triplet+--host="${host_triplet?}"} \
       --disable-shared \
       --enable-static \
       --without-mock-sdk \
       CC="${CARMA_CC?}" \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include -I/linux/${CARMA_NDK_ARCH?}/include" \
       CXX="${CARMA_CXX?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib -L/linux/${CARMA_NDK_ARCH?}/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && . ../../linux.dockerfile.make1 \
)

RUN ( : \
  && mkdir /x/carma-2 \
  && cd /x/carma-2 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CC=${CARMA_X64_CC?}; \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXX=${CARMA_X64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
       unset build_triplet; \
       unset host_triplet; \
     ;; *) \
       CARMA_CC=${CARMA_ARM64_CC?}; \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXX=${CARMA_ARM64_CXX?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
       build_triplet=x86_64-pc-linux-gnu; \
       host_triplet=aarch64-pc-linux-gnu; \
     esac \
  && ./configure \
       ${build_triplet+--build="${build_triplet?}"} \
       ${host_triplet+--host="${host_triplet?}"} \
       --disable-static \
       --enable-shared \
       --without-mock-sdk \
       CC="${CARMA_CC?}" \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I/usr/local/include -I/linux/${CARMA_NDK_ARCH?}/include" \
       CXX="${CARMA_CXX?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L/usr/local/lib -L/linux/${CARMA_NDK_ARCH?}/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && . ../../linux.dockerfile.make2 \
)
