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

FROM ubuntu

RUN : \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -q -y update \
  && apt-get -q -y install \
       autoconf \
       automake \
       bash \
       clang \
       cmake \
       gawk \
       git \
       lbzip2 \
       libgmp-dev \
       libtool \
       make \
       perl \
       python3 \
       wget \
       xz-utils \
;

SHELL ["/bin/bash", "-c"]

RUN : \
  && mkdir x \
  && cd x \
  && git clone https://github.com/emscripten-core/emsdk.git \
  && cd emsdk \
  && ./emsdk install latest \
  && ./emsdk activate latest \
;

ARG DOCKERFILE_THREADS
ENV DOCKERFILE_THREADS=${DOCKERFILE_THREADS?}

ARG COMMON_FLAGS="-O2 -pthread -fexceptions -sPTHREAD_POOL_SIZE=4 -sFETCH -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=50MB -s MAXIMUM_MEMORY=1000MB"
ARG MY_CPPFLAGS="-I/x/stage/include"
ARG MY_CFLAGS="${COMMON_FLAGS?}"
ARG MY_CXXFLAGS="${COMMON_FLAGS?}"
ARG MY_LDFLAGS="-L/x/stage/lib ${COMMON_FLAGS?}"

COPY openssl.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/openssl \
  && cd /x/openssl \
  && tar xzf ../openssl.tar.gz \
  && cd * \
  && ./Configure \
       --prefix=/x/stage \
       AR=/x/emsdk/upstream/bin/llvm-ar \
       CC=/x/emsdk/upstream/emscripten/emcc \
       CFLAGS="${MY_CFLAGS?}" \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXX=/x/emsdk/upstream/emscripten/em++ \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       LD=/x/emsdk/upstream/bin/llvm-ld \
       LDFLAGS="${MY_LDFLAGS?}" \
       RANLIB=/x/emsdk/upstream/bin/llvm-ranlib \
       no-buildtest-c++ \
       no-external-tests \
       no-shared \
       no-tests \
       no-unit-test \
       threads \
       cc \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install_sw \
;

# TODO: libcurl shouldn't be necessary long term?
COPY curl.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/curl \
  && cd /x/curl \
  && tar xzf ../curl.tar.gz \
  && cd * \
  && emconfigure ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=none \
       --prefix=/x/stage \
       --with-openssl \
       --without-brotli \
       --without-libpsl \
       --without-zlib \
       --without-zstd \
       CFLAGS="${MY_CFLAGS?}" \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       LDFLAGS="${MY_LDFLAGS?}" \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

# TODO: rabbitmq-c shouldn't be necessary long term?
COPY rabbitmq.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/rabbitmq \
  && cd /x/rabbitmq \
  && tar xzf ../rabbitmq.tar.gz \
  && cd * \
  && emcmake cmake \
       -D BUILD_API_DOCS=OFF \
       -D BUILD_EXAMPLES=OFF \
       -D BUILD_SHARED_LIBS=OFF \
       -D BUILD_STATIC_LIBS=ON \
       -D BUILD_TESTING=OFF \
       -D BUILD_TESTS=OFF \
       -D BUILD_TOOLS=OFF \
       -D BUILD_TOOLS_DOCS=OFF \
       -D CMAKE_CXX_FLAGS='-pthread -O2 -fexceptions' \
       -D CMAKE_C_FLAGS='-pthread -O2 -fexceptions' \
       -D CMAKE_INSTALL_PREFIX=/x/stage \
       -D CMAKE_LD_FLAGS='-L/x/stage/lib -fexceptions' \
       -D ENABLE_SSL_SUPPORT=ON \
       -D OPENSSL_CRYPTO_LIBRARY=/x/stage/lib/libcrypto.a \
       -D OPENSSL_INCLUDE_DIR=/x/stage/include \
       -D OPENSSL_SSL_LIBRARY=/x/stage/lib/libssl.a \
       -D OPENSSL_USE_STATIC_LIBS=TRUE \
       -D RUN_SYSTEM_TESTS=OFF \
       . \
  && emmake cmake --build . --config Release --target install \
;

COPY libsodium.gitbundle /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/libsodium \
  && cd /x/libsodium \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && sed -i \
       -e 's/ifdef __EMSCRIPTEN__/if 0/g' \
       -e 's/ifndef __EMSCRIPTEN__/if 1/g' \
       src/libsodium/randombytes/randombytes.c \
  && printf '%s\n' \
       'libsodium_la_SOURCES += randombytes/sysrandom/randombytes_sysrandom.c' \
       >>src/libsodium/Makefile.am \
  && ./autogen.sh \
  && emconfigure ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-asm \
       --disable-shared \
       --disable-silent-rules \
       --disable-ssp \
       --enable-static \
       --host=none \
       --prefix=/x/stage \
       CFLAGS="${MY_CFLAGS?}" \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       LDFLAGS="${MY_LDFLAGS?}" \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

COPY gmp.tar.any /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/gmp \
  && cd /x/gmp \
  && tar xf ../gmp.tar.any \
  && cd * \
  && emconfigure ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-assembly \
       --disable-shared \
       --enable-static \
       --host=none \
       --prefix=/x/stage \
       CC_FOR_BUILD=cc \
       CFLAGS="${MY_CFLAGS?}" \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       LDFLAGS="${MY_LDFLAGS?}" \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

COPY ntl.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/ntl \
  && cd /x/ntl \
  && tar xf ../ntl.tar.gz \
  && mv * ntl \
  && cd */src \
  && sed -i \
       -e 's|`\./GenConfigInfo`|"(clang,2011,x86,linux)"|' \
       -e 's|return Exec("Check[^"]*")|return 1|' \
       DoConfig \
  && emconfigure ./configure \
       AR=/x/emsdk/upstream/bin/llvm-ar \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXX=/x/emsdk/upstream/emscripten/em++ \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       DEF_PREFIX=/x/stage \
       LDFLAGS="${MY_LDFLAGS?}" \
       NATIVE=off \
       NTL_GF2X_LIB=off \
       NTL_GMP_LIP=on \
       NTL_SAFE_VECTORS=on \
       NTL_STD_CXX11=on \
       NTL_THREADS=off \
       RANLIB=/x/emsdk/upstream/bin/llvm-ranlib \
       SHARED=off \
       TUNE=generic \
  && sed -i \
       -e '1 s|^|cp GetTime5.cpp GetTime.cpp; exit 0;|' \
       MakeGetTime \
  && sed -i \
       -e '1 s|^|cp GetPID2.cpp GetPID.cpp; exit 0;|' \
       MakeGetPID \
  && sed -i \
       -e 's|if \./CheckFeatures|if test -f Check$f.have|' \
       MakeCheckFeatures \
  && touch \
       CheckALIGNED_ARRAY.have \
       CheckBUILTIN_CLZL.have \
       CheckCHRONO_TIME.have \
       CheckCOPY_TRAITS1.have \
       CheckCOPY_TRAITS2.have \
       CheckLL_TYPE.have \
       CheckPOSIX_TIME.have \
       clobber \
       setup1 \
       setup3 \
;

#
# Enable this RUN instruction and open the resulting page in a browser
# to generate the content of NTL/mach_desc.h. This is the setup1 target
# from the NTL makefile.
#
# RUN : \
#   && . /x/emsdk/emsdk_env.sh \
#   && cd /x/ntl/*/src \
#   && sed -i \
#        -e 's|fopen([^)]*)|stdout|' \
#        MakeDesc.cpp \
#   && emcc \
#        -I. \
#        -I../include \
#        -I/x/stage/include \
#        -L/x/stage/lib \
#        -o MakeDesc.html \
#        MakeDesc.cpp \
#        MakeDescAux.cpp \
#   && mkdir -p /x/out \
#   && cp \
#        MakeDesc.html \
#        MakeDesc.js \
#        MakeDesc.wasm \
#        /x/out \
# ;
#

COPY \
  ntl-setup/wasm32/include/NTL/mach_desc.h \
/x/ntl/ntl/include/NTL/

#
# Enable this RUN instruction and open the resulting page in a browser
# to generate the content of NTL/gmp_aux.h. This is the setup3 target
# from the NTL makefile.
#
# RUN : \
#   && . /x/emsdk/emsdk_env.sh \
#   && cd /x/ntl/*/src \
#   && emcc \
#        -I. \
#        -I../include \
#        -I/x/stage/include \
#        -L/x/stage/lib \
#        -o gmp_aux.html \
#        gen_gmp_aux.cpp \
#        -lgmp \
#   && mkdir -p /x/out \
#   && cp \
#        gmp_aux.html \
#        gmp_aux.js \
#        gmp_aux.wasm \
#        /x/out \
# ;
#

COPY \
  ntl-setup/wasm32/include/NTL/gmp_aux.h \
/x/ntl/ntl/include/NTL/

RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && cd /x/ntl/*/src \
  && make setup-phase \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

COPY sqlite.tar.any /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/sqlite \
  && cd /x/sqlite \
  && tar xf ../sqlite.tar.any \
  && cd * \
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
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;

COPY sst.tar.gz /x/
RUN : \
  && . /x/emsdk/emsdk_env.sh \
  && mkdir /x/sst \
  && cd /x/sst \
  && tar xzf ../sst.tar.gz \
  && cd * \
  && emconfigure ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=none \
       --prefix=/x/stage \
       --with-build-groups=cpp \
       CFLAGS="${MY_CFLAGS?}" \
       CPPFLAGS="${MY_CPPFLAGS?}" \
       CXXFLAGS="${MY_CXXFLAGS?}" \
       LDFLAGS="${MY_LDFLAGS?}" \
  && emmake make -j${DOCKERFILE_THREADS?} \
  && emmake make install \
;
