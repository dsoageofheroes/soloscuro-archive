#!/bin/bash
set -e
# One hacked script to automake windows builds.
FILE_NAME=ds1-pre-alpha-latest-windows.zip
INSTALL_PATH=/var/www/dso/downloads/

cd .. 
make clean
CC=x86_64-w64-mingw32-gcc make -f makefile.win mdark
cd builds
rm -rf staging
mkdir staging
cp ../mdark.exe staging/
cp ../*.lua staging/
cp ../*dll staging/
locate SDL2.dll | xargs -I {} cp {} staging/
locate lua53.dll | xargs -I {} cp {} staging/
cp windows/* staging/
cd staging
zip ${FILE_NAME} *
mv ${FILE_NAME} ${INSTALL_PATH}
cd ..
