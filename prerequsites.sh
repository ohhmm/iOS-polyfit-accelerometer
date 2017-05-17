cd boost
git submodule init
git submodule update --recursive --remote
./bootstrap.sh
#./b2 link=static cxxflags=-fembed-bitcode
