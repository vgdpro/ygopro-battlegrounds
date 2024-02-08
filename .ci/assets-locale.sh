#!/bin/bash
set -x
set -o errexit
# ygopro-database
apt update && apt -y install wget git libarchive-tools
git clone --depth=1 https://code.mycard.moe/mycard/ygopro-database
cp -rf ./ygopro-database/locales/$TARGET_LOCALE/* .
# ygopro-images
mkdir pics
# skip it in develop branch
wget -O - https://cdn01.moecube.com/images/ygopro-images-${TARGET_LOCALE}.zip | bsdtar -C pics -xf -
