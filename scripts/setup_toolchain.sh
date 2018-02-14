#!/bin/bash
set -e

: "${TOOLCHAIN_NAME:?Need to set TOOLCHAIN_NAME non-empty}"
: "${TARGET:?Need to set TARGET non-empty}"

if [[ -d "$HOME/${TOOLCHAIN_NAME}/${TARGET}" ]] ; then
  echo "toolchain ${TOOLCHAIN_NAME} ${TARGET} exists!"
  exit 0
fi

export ARCHIVE_NAME=${TOOLCHAIN_NAME}.tar.xz

echo "TOOLCHAIN: $TOOLCHAIN_NAME"
echo "TARGET   : $TARGET"

cd $HOME

test -f "${ARCHIVE_NAME}" || wget https://releases.linaro.org/components/toolchain/binaries/latest/${TARGET}/${ARCHIVE_NAME}
tar -xJf ${ARCHIVE_NAME}
