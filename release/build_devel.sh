#!/usr/bin/bash -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"

TARGET_DIR=/tmp/releasepilot
SOURCE_DIR="$(git rev-parse --show-toplevel)"

# set git identity
source $DIR/identity.sh

echo "[-] Setting up repo T=$SECONDS"
if [ ! -d "$TARGET_DIR" ]; then
  mkdir -p $TARGET_DIR
  cd $TARGET_DIR
  git init
  git remote add origin git@github.com:commaai/openpilot.git
fi

echo "[-] git prune T=$SECONDS"
cd $TARGET_DIR
git gc

echo "[-] bringing master-ci and devel in sync T=$SECONDS"
git fetch origin master-ci
git fetch origin devel

git checkout -f --track origin/master-ci
git reset --hard master-ci
git checkout master-ci
git reset --hard origin/devel
git clean -xdf

# remove everything except .git
echo "[-] erasing old openpilot T=$SECONDS"
find . -maxdepth 1 -not -path './.git' -not -name '.' -not -name '..' -exec rm -rf '{}' \;

# do the files copy
echo "[-] copying files T=$SECONDS"
cd $SOURCE_DIR
cp -pR --parents $(cat release/files_common) $TARGET_DIR/
cp -pR --parents $(cat release/files_tici) $TARGET_DIR/

# append source hash and build date to version
GIT_HASH=$(git --git-dir=$SOURCE_DIR/.git rev-parse --short HEAD)
DATETIME=$(date '+%Y-%m-%dT%H:%M:%S')
VERSION=$(cat selfdrive/common/version.h | awk -F\" '{print $2}')
echo "#define COMMA_VERSION \"$VERSION-$GIT_HASH-$DATETIME\"" > selfdrive/common/version.h

# test files
if [ ! -z "$DEVEL_TEST" ]; then
  cp -pR --parents tools/ $TARGET_DIR/
fi

# in the directory
cd $TARGET_DIR

rm -f panda/board/obj/panda.bin.signed

echo "[-] committing version $VERSION T=$SECONDS"
git add -f .
git status
git commit -a -m "openpilot v$VERSION release"

if [ ! -z "$CI_PUSH" ]; then
  echo "[-] Pushing to $CI_PUSH T=$SECONDS"
  git remote set-url origin git@github.com:commaai/openpilot.git
  git push -f origin master-ci:$CI_PUSH
fi

echo "[-] done T=$SECONDS"
