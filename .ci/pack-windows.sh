#!/bin/bash
set -x
set -o errexit
# TARGET_LOCALE

apt update && apt -y install tar git
git submodule update --init
mkdir dist replay
tar -zcf dist/ygopro-$CI_COMMIT_REF_NAME-win32-$TARGET_LOCALE.tar.gz --exclude='.git*' ygopro.exe LICENSE README.md lflist.conf strings.conf system.conf cards.cdb script textures deck single pics replay sound bot.conf Bot.exe WindBot
