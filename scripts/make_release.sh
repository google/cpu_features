#!/usr/bin/env bash

set -e # Fail on error
set -u # Treat unset variables as an error and exit immediately

ACTION='\033[1;90m'
FINISHED='\033[1;96m'
NOCOLOR='\033[0m'
ERROR='\033[0;31m'

echo -e "${ACTION}Checking environnement${NOCOLOR}"
if [[ ! $1 =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]
then
    echo -e "${ERROR}Invalid version number. Aborting. ${NOCOLOR}"
    exit 1
fi

declare -r VERSION=$1
declare -r GIT_TAG="v$1"

BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [[ "${BRANCH}" != "master" ]]
then
    echo -e "${ERROR}Not on master. Aborting. ${NOCOLOR}"
    echo
    exit 1
fi

git fetch
HEADHASH=$(git rev-parse HEAD)
UPSTREAMHASH=$(git rev-parse master@{upstream})

if [[ "${HEADHASH}" != "${UPSTREAMHASH}" ]]
then
    echo -e "${ERROR}Not up to date with origin. Aborting.${NOCOLOR}"
    echo
    exit 1
fi

git update-index -q --refresh
if ! git diff-index --quiet HEAD --
then
    echo -e "${ERROR}Branch has uncommited changes. Aborting.${NOCOLOR}"
    exit 1
fi

if [ ! -z "$(git ls-files --exclude-standard --others)" ]
then
    echo -e "${ERROR}Branch has untracked files. Aborting.${NOCOLOR}"
    exit 1
fi

declare -r LATEST_GIT_TAG=$(git describe --tags)
declare -r LATEST_VERSION=${LATEST_GIT_TAG#"v"}

if ! dpkg --compare-versions "${VERSION}" "gt" "${LATEST_VERSION}"
then
    echo -e "${ERROR}Invalid version ${VERSION} <= ${LATEST_VERSION} (latest). Aborting.${NOCOLOR}"
    exit 1
fi

echo -e "${ACTION}Modifying CMakeLists.txt${NOCOLOR}"
sed -i "s/CpuFeatures VERSION ${LATEST_VERSION}/CpuFeatures VERSION ${VERSION}/g" CMakeLists.txt

echo -e "${ACTION}Commit new revision${NOCOLOR}"
git add CMakeLists.txt
git commit -m"Release ${GIT_TAG}"

echo -e "${ACTION}Create new tag${NOCOLOR}"
git tag ${GIT_TAG}

echo -e "${FINISHED}Local release is ready. Run `git push origin --tags`${NOCOLOR}"
