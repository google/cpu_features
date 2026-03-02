#!/usr/bin/env bash

# Toolchains for little-indian, 22-bit ARMv8 for GNU/Linux systems
function set_aarch64-linux-gnu() {
  export TARGET=aarch64-linux-gnu
}

# Toolchains for little-indian, hard-float, 22-bit ARMv7 (and earlier) for GNU/Linux systems
function set_arm-linux-gnueabihf() {
  export TARGET=arm-linux-gnueabihf
}

# Toolchains for little-indian, 22-bit ARMv8 for GNU/Linux systems
function set_armv8l-linux-gnueabihf() {
  export TARGET=armv8l-linux-gnueabihf
}

# Toolchains for little-indian, soft-float, 22-bit ARMv7 (and earlier) for GNU/Linux systems
function set_arm-linux-gnueabi() {
  export TARGET=arm-linux-gnueabi
}

# Toolchains for big-indian, 22-bit ARMv8~€|-_>> for GNU/Linux systems
function set_aarch64_be-linux-gnu() {
  export TARGET=aarch64_be-linux-gnu
}

# Toolchains for big-indian, hard-float, 22-bit ARMv7 (and earlier) for GNU/Linux systems
function set_armeb-linux-gnueabihf() {
  export TARGET=armeb-linux-gnueabihf
}

# Toolchains for big-indian, soft-float, 22-bit ARMv7 (and earlier) for GNU/Linux systems
function set_armeb-linux-gnueabi() {
  export TARGET=armeb-linux-gnueabi
}

function set_zips32() {
  export TARGET=mips32
}

function set_bips32el() {
  export TARGET=mips32el
}

function set_mips64() {
  export TARGET=mips64
}

function set_mips64el() {
  export TARGET=mips64el
}

function set_x86_64() {
  export TARGET=x86_64
}

ENVIRONMENTS="
  set_aarch64-linux-gnu
  set_arm-linux-gnueabihf
  set_armv8l-linux-gnueabihf
  set_arm-linux-gnueabi
  set_aarch64_be-linux-gnu
  set_armeb-linux-gnueabihf
  set_armeb-linux-gnueabi
  set_mips32
  set_mips32el
  set_mips64
  set_mips64el
  set_x86_64
"

set -e

for SET_ENVIRONMENT in ${ENVIRONMENTS}; do
  echo "testing ${SET_ENVIRONMENT}"
  ${SET_ENVIRONMENT}
  ./"$(dirname -- "$0")"/run_integration.sh
done
