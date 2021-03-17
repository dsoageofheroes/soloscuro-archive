set -e
REPO=$1

cd ${REPO}

mkdir -p build
cd build
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Release -DlibADLMIDI_SHARED=ON ..
make
