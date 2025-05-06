#!/bin/bash
set -x
set -o errexit
# ygopro-database
apt update && apt -y install wget git libarchive-tools
git clone --depth=1 https://code.moenext.com/mycard/ygopro-database
cp -rf ./ygopro-database/locales/$TARGET_LOCALE/* .
# ygopro-images
mkdir pics
if [[ "$CI_COMMIT_REF_NAME" == *"develop"* || "$CI_COMMIT_REF_NAME" == *".pre"* ]]; then
  echo "This is a pre-release, skipping download."
else
  wget -O - https://cdn02.moecube.com:444/images/ygopro-images-${TARGET_LOCALE}.zip | bsdtar -C pics -xf -
fi
