#!/bin/bash

SCRIPT_FOLDER=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
PROJECT_FOLDER="${SCRIPT_FOLDER}/.."
ARCHIVE_FOLDER=~/archives
QEMU_INSTALL=${ARCHIVE_FOLDER}/qemu
DEFAULT_CMAKE_ARGS=" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON"

function extract() {
  case $1 in
    *.tar.bz2)   tar xjf $1    ;;
    *.tar.xz)    tar xJf $1    ;;
    *.tar.gz)    tar xzf $1    ;;
    *)
      echo "don't know how to extract '$1'..."
      exit 1
  esac
}

function unpackifnotexists() {
  mkdir -p ${ARCHIVE_FOLDER}
  cd ${ARCHIVE_FOLDER}
  local URL=$1
  local DESTINATION=`pwd`/$2
  if [[  ! -d "${DESTINATION}" ]] ; then
    local ARCHIVE_NAME=`echo ${URL} | sed 's/.*\///'`
    test -f ${ARCHIVE_NAME} || wget ${URL}
    extract ${ARCHIVE_NAME}
  fi
}


function installqemuifneeded() {
  local VERSION=${QEMU_VERSION:=2.11.1}
  local ARCHES=${QEMU_ARCHES:=arm aarch64 i386 x86_64 mipsel}
  local TARGETS=${QEMU_TARGETS:=$(echo $ARCHES | sed 's#$# #;s#\([^ ]*\) #\1-linux-user #g')}

  if echo "${VERSION} ${TARGETS}" | cmp --silent ${QEMU_INSTALL}/.build -; then
    echo "qemu ${VERSION} up to date!"
    return 0
  fi

  echo "VERSION: ${VERSION}"
  echo "TARGETS: ${TARGETS}"

  rm -rf ${QEMU_INSTALL}

  # Checking for a tarball before downloading makes testing easier :-)
  local QEMU_URL="http://wiki.qemu-project.org/download/qemu-${VERSION}.tar.xz"
  local QEMU_FOLDER="qemu-${VERSION}"
  unpackifnotexists ${QEMU_URL} ${QEMU_FOLDER}
  cd ${QEMU_FOLDER}

  ./configure \
    --prefix="${QEMU_INSTALL}" \
    --target-list="${TARGETS}" \
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

  echo "$VERSION $TARGETS" > ${QEMU_INSTALL}/.build
}

function assert_defined(){
  : ${1?Needs to be defined}
}

function integrate_cross() {
  assert_defined ${GCC_URL}
  assert_defined ${GCC_RELATIVE_FOLDER}
  assert_defined ${SYSROOT_URL}
  assert_defined ${SYSROOT_RELATIVE_FOLDER}

  unpackifnotexists ${GCC_URL} ${GCC_RELATIVE_FOLDER}
  unpackifnotexists ${SYSROOT_URL} ${SYSROOT_RELATIVE_FOLDER}

  local CMAKE_ARGS=${DEFAULT_CMAKE_ARGS}

  # Update cmake args for cross compilation.
  local SYSROOT_FOLDER=${ARCHIVE_FOLDER}/${SYSROOT_RELATIVE_FOLDER}
  local GCC_FOLDER=${ARCHIVE_FOLDER}/${GCC_RELATIVE_FOLDER}
  CMAKE_ARGS+=" -DCMAKE_SYSROOT=${SYSROOT_FOLDER}"
  CMAKE_ARGS+=" -DCMAKE_C_COMPILER=${GCC_FOLDER}/bin/${TARGET}-gcc"
  CMAKE_ARGS+=" -DCMAKE_CXX_COMPILER=${GCC_FOLDER}/bin/${TARGET}-g++"

  cd ${PROJECT_FOLDER}
  cmake -H. -B${BUILD_DIR} ${CMAKE_ARGS}
  cmake --build ${BUILD_DIR} --target all

  if [[ -n "${QEMU_ARCH}" ]]; then
    installqemuifneeded
    QEMU="qemu-${QEMU_ARCH} -L ${SYSROOT_FOLDER}"
    # Run tests
    for test_binary in ${BUILD_DIR}/test/*_test; do ${QEMU} ${test_binary}; done
    # Run demo program
    ${QEMU} ${BUILD_DIR}/list_cpu_features
  fi
}

function integrate_native() {
  local CMAKE_ARGS=${DEFAULT_CMAKE_ARGS}

  cd ${PROJECT_FOLDER}
  cmake -H. -B${BUILD_DIR} ${CMAKE_ARGS}
  cmake --build ${BUILD_DIR} --target all
  
  # Run tests
  for test_binary in ${BUILD_DIR}/test/*_test; do ${test_binary}; done
  # Run demo program
  ${BUILD_DIR}/list_cpu_features
}

function expand_linaro_config() {
  assert_defined ${TARGET}
  local LINARO_ROOT_URL=https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11
  GCC_URL=${LINARO_ROOT_URL}/${TARGET}/gcc-linaro-7.2.1-2017.11-x86_64_${TARGET}.tar.xz
  GCC_RELATIVE_FOLDER=gcc-linaro-7.2.1-2017.11-x86_64_${TARGET}
  SYSROOT_URL=${LINARO_ROOT_URL}/${TARGET}/sysroot-glibc-linaro-2.25-2017.11-${TARGET}.tar.xz
  SYSROOT_RELATIVE_FOLDER=sysroot-glibc-linaro-2.25-2017.11-${TARGET}
}

function expand_environment_and_integrate() {
  assert_defined ${PROJECT_FOLDER}
  assert_defined ${TARGET}

  BUILD_DIR="${PROJECT_FOLDER}/cmake_build/${TARGET}"
  mkdir -p ${BUILD_DIR}
  
  case ${TOOLCHAIN} in
    LINARO)
              expand_linaro_config
              integrate_cross
              ;;
    NATIVE)
              integrate_native
              ;;
    *)        echo "Unknown toolchain '${TOOLCHAIN}'..."
              exit 1
  esac
}

if [ "${CONTINUOUS_INTEGRATION}" = "true" ]; then
  expand_environment_and_integrate
fi
