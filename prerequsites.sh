git clone --recursive https://github.com/boostorg/boost.git
cd boost
git fetch --tags
git checkout boost-1_64_0
#git submodule init
#git submodule update --init --recursive --remote
#git submodule update --init --recursive --remote --no-fetch --depth=1
./bootstrap.sh
./b2 headers
./b2 link=static cxxflags=-fembed-bitcode
