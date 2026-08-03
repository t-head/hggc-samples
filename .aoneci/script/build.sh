source $AONE_CI_WORKSPACE/opt/PPU_SDK/envsetup.sh
cd $AONE_CI_SOURCE
mkdir -p build && cd build
cmake ..
make
