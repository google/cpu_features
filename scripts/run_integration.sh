#!/bin/bash

export BUILD_DIR=cmake_build
export CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -H. -B${BUILD_DIR}"

SCRIPT_FOLDER=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
PROJECT_FOLDER=${SCRIPT_FOLDER}/..
cd ${PROJECT_FOLDER}

if [[ -n "${CROSS_COMPILE}" ]]; then
  # Cross compilation
  : "${TOOLCHAIN_NAME:?Need to set TOOLCHAIN_NAME non-empty}"
  : "${TARGET:?Need to set TARGET non-empty}"
  : "${QEMU_ARCH:?Need to set QEMU_ARCH non-empty}"
  ${SCRIPT_FOLDER}/setup_qemu.sh
  ${SCRIPT_FOLDER}/setup_toolchain.sh
  export TOOLCHAIN=${HOME}/${TOOLCHAIN_NAME}
  export PATH=${TOOLCHAIN}/bin:${HOME}/qemu/bin:${PATH}
  export CMAKE_TOOLCHAIN_FILE=cmake/${TARGET}.cmake
  if [[ ! -f ${CMAKE_TOOLCHAIN_FILE} ]]; then
      echo "Missing cmake toolchain file : $CMAKE_TOOLCHAIN_FILE"
      exit 1
  fi
  # Generate makefile
  cmake ${CMAKE_ARGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  # Compile
  cmake --build ${BUILD_DIR} --target all
  # Run tests
  export QEMU=qemu-${QEMU_ARCH}
  export QEMU_LD_PREFIX=${TOOLCHAIN}/${TARGET}/libc
  export LD_LIBRARY_PATH=${TOOLCHAIN}/${TARGET}/libc
  for test_binary in ${BUILD_DIR}/test/*_test; do
    ${QEMU} ${test_binary}
  done
  # Run demo program
  ${QEMU} ${BUILD_DIR}/list_cpu_features
else
  # Native compilation
  # Generate makefile
  cmake ${CMAKE_ARGS}
  # Compile
  cmake --build ${BUILD_DIR} --target all
  # Run tests
  CTEST_OUTPUT_ON_FAILURE=1 cmake --build ${BUILD_DIR} --target test
  # Run demo program
  ${BUILD_DIR}/list_cpu_features
fi
