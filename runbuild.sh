#!/usr/bin/env bash

if [ -z "$JAVA_HOME" ]; then
	printf "No JAVA_HOME detected! "
	printf "Setup JAVA_HOME before build: export JAVA_HOME=/path/to/java\\n"
	exit 1
fi

if [ -z "$COIN_DEPS" ]; then
  printf "NO COIN_DEPS detected!"
	printf "Setup COIN_DEPS before build: export COIN_DEPS=path/to/depslib\\n"
	exit 1
fi

EXT=so
TARGET_OS=`uname -s`
case "$TARGET_OS" in
    Darwin)
        EXT=dylib
        ;;
    Linux)
        EXT=so
        ;;
    *)
        echo "Unknown platform!" >&2
        exit 1
esac

git submodule init && git submodule update
PROJECT_NAME=raiblocks
OKLIBRARY_NAME=lib${PROJECT_NAME}.${EXT}

if [ -d ./ok-build ];then
	rm -rf ./ok-build
fi
if [ -f ./${OKLIBRARY_NAME} ];then
	rm -f ./${OKLIBRARY_NAME}
fi

mkdir ok-build
cd ok-build
cmake -DCMAKE_BUILD_TYPE=Release -DOKLIBRARY_NAME=${PROJECT_NAME} -DACTIVE_NETWORK=rai_live_network -DBOOST_ROOT=${COIN_DEPS}/boost ../ok-wallet
[ $? -ne 0 ] && exit 1

make -j 2
[ $? -ne 0 ] && exit 1

cp ./lib${PROJECT_NAME}.${EXT} ../
nm lib${PROJECT_NAME}.${EXT} |grep "[ _]Java_com_okcoin"
