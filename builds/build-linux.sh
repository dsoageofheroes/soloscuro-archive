#!/bin/bash
set -e
# One hacked script to automake windows builds.
FILE_NAME=ds1-pre-alpha-latest-lin64.zip
INSTALL_PATH=/var/www/dso/downloads/

cd .. 
make clean
make -f makefile.sdl mdark
cd builds
rm -rf staging
mkdir -p staging/lua
cp ../mdark staging/
cp ../lua/*.lua staging/lua/
cp ../*so staging/
locate SDL2.so | grep "usr/lib" | xargs -I {} cp {} staging/
locate -b "\liblua5.3.so" | grep "usr/lib" | xargs -I {} cp {} staging/
cp linux/* staging/
cd staging
zip ${FILE_NAME} *
mv ${FILE_NAME} ${INSTALL_PATH}
cd ..
