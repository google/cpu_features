#!/bin/bash

set -e
set -x

###############################################################################
# Ensures qemu is compiled in $HOME/qemu.
# Input:
# - QEMU_VERSION, the version of qemu to use.
# - QEMU_ARCHES, the list of architectures qemu should support.
function setup_qemu() {
  local VERSION=${QEMU_VERSION:=2.11.0}
  local ARCHES=${QEMU_ARCHES:=arm aarch64 i386 x86_64 mipsel}
  local TARGETS=${QEMU_TARGETS:=$(echo $ARCHES | sed 's#$# #;s#\([^ ]*\) #\1-softmmu \1-linux-user #g')}

  if echo "$VERSION $TARGETS" | cmp --silent $HOME/qemu/.build -; then
    echo "qemu $VERSION up to date!"
    return 0
  fi

  echo "VERSION: $VERSION"
  echo "TARGETS: $TARGETS"

  cd $HOME
  rm -rf qemu

  # Checking for a tarball before downloading makes testing easier :-)
  test -f "qemu-$VERSION.tar.xz" || wget "http://wiki.qemu-project.org/download/qemu-$VERSION.tar.xz"
  tar -xJf "qemu-$VERSION.tar.xz"
  cd "qemu-$VERSION"

  ./configure \
    --prefix="$HOME/qemu" \
    --target-list="$TARGETS" \
    --disable-docs \
    --disable-sdl \
    --disable-gtk \
    --disable-gnutls \
    --disable-gcrypt \
    --disable-nettle \
    --disable-curses \
    --static

  make -j4
  make install

  echo "$VERSION $TARGETS" > $HOME/qemu/.build
}

###############################################################################
# Ensures the linaro toolchain is available in $HOME/toolchains.
# Input:
# - LINARO_URL, the url of the of the x86_64 gcc tarball.
function get_linaro_toolchain_folder() {
  local LINARO_URL_NO_HTTPS=${LINARO_URL#https:}
  local ARCHIVE_NAME=${LINARO_URL_NO_HTTPS##/*/}
  local TOOLCHAIN_NAME=${ARCHIVE_NAME%.tar.*}
  local TOOLCHAIN_HOME=${HOME}/toolchains
  local TOOLCHAIN_FOLDER="${TOOLCHAIN_HOME}/${TOOLCHAIN_NAME}"
  if [[  ! -d "${TOOLCHAIN_FOLDER}" ]] ; then
    mkdir -p "${TOOLCHAIN_HOME}"
    cd "${TOOLCHAIN_HOME}"
    wget ${LINARO_URL}
    tar -xJf ${ARCHIVE_NAME}
    rm ${ARCHIVE_NAME}
  fi
  echo ${TOOLCHAIN_FOLDER}
}

###############################################################################

SCRIPT_FOLDER=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
PROJECT_FOLDER="${SCRIPT_FOLDER}/.."
cd ${PROJECT_FOLDER}

BUILD_DIR="${PROJECT_FOLDER}/cmake_build"
CMAKE_PATH="${PATH}"
CMAKE_ARGS="-H. -B${BUILD_DIR}"
CMAKE_ARGS+=" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON"

#
# Setup toolchain if necessary.
#
if [[ -n "${LINARO_URL}" ]]; then
  # Cross compilation
  : "${TARGET:?Need to set TARGET non-empty}"
  TOOLCHAIN=`get_linaro_toolchain_folder`
  CMAKE_TOOLCHAIN_FILE=cmake/${TARGET}.cmake
  if [[ ! -f ${CMAKE_TOOLCHAIN_FILE} ]]; then
      echo "Missing cmake toolchain file : $CMAKE_TOOLCHAIN_FILE"
      exit 1
  fi
  CMAKE_ARGS+=" -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
  CMAKE_PATH="${TOOLCHAIN}/bin:${PATH}"
fi

# Generate makefile
PATH=${CMAKE_PATH} cmake ${CMAKE_ARGS}
# Compile
PATH=${CMAKE_PATH} cmake --build ${BUILD_DIR} --target all

#
# Tests
#
if [[ -n "${QEMU_ARCH}" ]]; then
  # Run tests
  QEMU_PATH="${HOME}/qemu/bin:${PATH}"
  : "${QEMU_ARCH:?Need to set QEMU_ARCH non-empty}"
  setup_qemu
  QEMU="qemu-${QEMU_ARCH} -L ${TOOLCHAIN}/${TARGET}/libc"
  for test_binary in ${BUILD_DIR}/test/*_test; do
    PATH=${QEMU_PATH} ${QEMU} ${test_binary}
  done
  # Run demo program
  PATH=${QEMU_PATH} ${QEMU} ${BUILD_DIR}/list_cpu_features
else
  # Run tests
  CTEST_OUTPUT_ON_FAILURE=1 cmake --build ${BUILD_DIR} --target test
  # Run demo program
  ${BUILD_DIR}/list_cpu_features
fi
