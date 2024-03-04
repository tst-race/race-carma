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
ARG CARMA_NDK_API_LEVEL=29
ARG CARMA_NDK_BASE=/opt/android/ndk/default
ARG CARMA_NDK_ROOT=$CARMA_NDK_BASE/toolchains/llvm/prebuilt/linux-x86_64
ENV CARMA_NDK_PREFIX=/android/$CARMA_NDK_ARCH
ARG CARMA_NDK_TRIPLET=$CARMA_NDK_ARCH2-linux-android
ARG CARMA_NDK_TRIPLET_A=$CARMA_NDK_TRIPLET$CARMA_NDK_API_LEVEL

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

ARG CARMA_X64_CFLAGS='-O2 -fPIC -march=x86-64'
ARG CARMA_X64_CXXFLAGS='-O2 -fPIC -march=x86-64'

ARG CARMA_ARM64_CFLAGS='-O2 -fPIC'
ARG CARMA_ARM64_CXXFLAGS='-O2 -fPIC'

COPY gmp.tar.any /x/
RUN : \
  && mkdir /x/gmp \
  && cd /x/gmp \
  && tar xf ../gmp.tar.any \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       --with-pic \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CC_FOR_BUILD=cc \
       CFLAGS="${CARMA_CFLAGS?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

COPY ntl.tar.gz /x/
RUN : \
  && mkdir /x/ntl \
  && cd /x/ntl \
  && tar xf ../ntl.tar.gz \
  && cd */src \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && sed -i 's|`\./GenConfigInfo`|"(clang,2011,x86,linux)"|' DoConfig \
  && sed -i 's|return Exec("Check[^"]*")|return 1|' DoConfig \
  && ./configure \
       CPPFLAGS="-I$CARMA_NDK_PREFIX/include" \
       CXX=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang++ \
       CXXFLAGS="${CARMA_CXXFLAGS?} -ffp-contract=off" \
       DEF_PREFIX=$CARMA_NDK_PREFIX \
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

COPY libsodium.gitbundle /x/
RUN : \
  && mkdir /x/libsodium \
  && cd /x/libsodium \
  && git clone ../libsodium.gitbundle libsodium \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./autogen.sh \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-pie \
       --disable-shared \
       --enable-static \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       --with-pic \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CFLAGS="${CARMA_CFLAGS?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} V=1 \
  && make V=1 install \
;

COPY rabbitmq.tar.gz /x/
RUN : \
  && mkdir /x/rabbitmq \
  && cd /x/rabbitmq \
  && tar xzf ../rabbitmq.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && cmake \
       -D ANDROID_ABI=${CARMA_NDK_ARCH?} \
       -D ANDROID_PLATFORM=${CARMA_NDK_API_LEVEL?} \
       -D ANDROID_TOOLCHAIN=clang \
       -D BUILD_API_DOCS=OFF \
       -D BUILD_EXAMPLES=OFF \
       -D BUILD_SHARED_LIBS=OFF \
       -D BUILD_STATIC_LIBS=ON \
       -D BUILD_TESTING=OFF \
       -D BUILD_TESTS=OFF \
       -D BUILD_TOOLS=OFF \
       -D BUILD_TOOLS_DOCS=OFF \
       -D CMAKE_CXX_FLAGS="${CARMA_CXXFLAGS?}" \
       -D CMAKE_C_FLAGS="${CARMA_CFLAGS?}" \
       -D CMAKE_INSTALL_PREFIX=${CARMA_NDK_PREFIX?} \
       -D CMAKE_TOOLCHAIN_FILE=${CARMA_NDK_BASE?}/build/cmake/android.toolchain.cmake \
       -D ENABLE_SSL_SUPPORT=ON \
       -D OPENSSL_CRYPTO_LIBRARY=/android/${CARMA_NDK_ARCH?}/lib/libcrypto.so \
       -D OPENSSL_INCLUDE_DIR=/android/${CARMA_NDK_ARCH?}/include \
       -D OPENSSL_SSL_LIBRARY=/android/${CARMA_NDK_ARCH?}/lib/libssl.so \
       -D OPENSSL_USE_STATIC_LIBS=FALSE \
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
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CFLAGS="${CARMA_CFLAGS?}" \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
  && rm -f \
       "$CARMA_NDK_PREFIX"/lib/libsqlite3.la \
;

COPY sst.tar.gz /x/
RUN : \
  && mkdir /x/sst \
  && cd /x/sst \
  && tar xf ../sst.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       --with-build-groups=cpp \
       --with-lib-crypto-or-die \
       --with-lib-gmp-or-die \
       --with-lib-ntl-or-die \
       --without-avx-512-f \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I$CARMA_NDK_PREFIX/include" \
       CXX=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang++ \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L$CARMA_NDK_PREFIX/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && make -j${DOCKERFILE_THREADS?} \
  && make install \
;

# Libtool sometimes outputs "//lib/libcurl.la" into libsst.la for some
# reason. I can't figure out why, so I'll just fix it up here.
RUN : \
  && sed -i \
       "s|//lib/libcurl.la|$CARMA_NDK_PREFIX/lib/libcurl.la|g" \
       $CARMA_NDK_PREFIX/lib/libsst.la \
;

COPY \
  android.dockerfile.make1 \
  android.dockerfile.make2 \
  carma.tar.gz \
/x/

ENV CARMA_RACE_SDK_LIBS="-lraceSdkCommon -lthrift -lyaml-cpp -llog"
ENV CARMA_LIBS_1="$CARMA_RACE_SDK_LIBS -lsst -lcurl -lrabbitmq -lssl -lcrypto -lsodium -lntl -lgmp -lsqlite3"
ENV CARMA_LIBS_2="$CARMA_NDK_PREFIX/lib/libcarma.a $CARMA_LIBS_1"

RUN ( : \
  && mkdir /x/carma-1 \
  && cd /x/carma-1 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-shared \
       --enable-static \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       --without-mock-sdk \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I$CARMA_NDK_PREFIX/include" \
       CXX=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang++ \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L$CARMA_NDK_PREFIX/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && . ../../android.dockerfile.make1 \
)

RUN ( : \
  && mkdir /x/carma-2 \
  && cd /x/carma-2 \
  && tar xf ../carma.tar.gz \
  && cd * \
  && case ${CARMA_NDK_ARCH?} in x86_64) \
       CARMA_CFLAGS=${CARMA_X64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_X64_CXXFLAGS?}; \
     ;; *) \
       CARMA_CFLAGS=${CARMA_ARM64_CFLAGS?}; \
       CARMA_CXXFLAGS=${CARMA_ARM64_CXXFLAGS?}; \
     esac \
  && ./configure \
       --build=x86_64-pc-linux-gnu \
       --disable-static \
       --enable-shared \
       --host=$CARMA_NDK_TRIPLET \
       --prefix=$CARMA_NDK_PREFIX \
       --without-mock-sdk \
       CC=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang \
       CFLAGS="${CARMA_CFLAGS?}" \
       CPPFLAGS="-I$CARMA_NDK_PREFIX/include" \
       CXX=$CARMA_NDK_ROOT/bin/$CARMA_NDK_TRIPLET_A-clang++ \
       CXXFLAGS="${CARMA_CXXFLAGS?}" \
       EXE_CFLAGS="${CARMA_CFLAGS?}" \
       EXE_CXXFLAGS="${CARMA_CXXFLAGS?}" \
       LDFLAGS="-L$CARMA_NDK_PREFIX/lib" \
       LIB_CFLAGS="${CARMA_CFLAGS?}" \
       LIB_CXXFLAGS="${CARMA_CXXFLAGS?}" \
  && . ../../android.dockerfile.make2 \
)
