#!/bin/bash

# This script allows updating the submodules and copying the updates files to the libs folder.
# Usage : cd path/to/libs_submodules && ./sync.sh

echo "Updating SCLang..."
git submodule update ../libs_submodules/supercollider

echo "Configuring libSCLang..."
mkdir -p ./supercollider/build
cd ./supercollider/build
cmake -G "Unix Makefiles" -D CMAKE_C_COMPILER=/usr/bin/clang -D CMAKE_CXX_COMPILER=/usr/bin/clang++ -DSC_IDE=OFF -DSC_QT=OFF -DNATIVE=OFF -DINSTALL_HELP=OFF -DENABLE_TESTSUITE=OFF -DNO_X11=ON -DSCLANG_SERVER=OFF -DNOVA_SIMD=OFF -DLIBSCSYNTH=OFF -DSC_VIM=OFF -DSYSTEM_YAMLCPP=OFF -DFORTIFY=ON ..

echo "Building libSCLang..."
cmake --build . --target libsclang  --config Release
cd ../../

echo "Copying SCLang..."
mkdir -p ../libs/supercollider/include
mkdir -p ../libs/supercollider/src
mkdir -p ../libs/supercollider/lib/osx

#cp -R ./supercollider/src ../libs/supercollider/
cp -R ./supercollider/include/common ../libs/supercollider/include/
cp -R ./supercollider/include/lang ../libs/supercollider/include/
cp -R ./supercollider/include/plugin_interface ../libs/supercollider/include/
cp ./supercollider/build/lang/libsclang.a ../libs/supercollider/lib/osx/libsclang.a
cp ./supercollider/build/external_libraries/hidapi/mac/libhidapi.a ../libs/supercollider/lib/osx/libhidapi.a
cp ./supercollider/build/external_libraries/hidapi/hidapi_parser/libhidapi_parser.a ../libs/supercollider/lib/osx/libhidapi_parser.a
cp ./supercollider/build/external_libraries/libboost_regex_lib.a ../libs/supercollider/lib/osx/libboost_regex_lib.a
cp ./supercollider/build/external_libraries/libboost_thread_lib.a ../libs/supercollider/lib/osx/libboost_thread_lib.a
cp ./supercollider/build/external_libraries/libyaml.a ../libs/supercollider/lib/osx/libyaml.a
cp ./supercollider/platform/mac/lib/scUBlibsndfile.a ../libs/supercollider/lib/osx/scUBlibsndfile.a
cp ./supercollider/COPYING ../libs/supercollider/COPYING

echo "Copying SCClassLibrary..."
#mkdir -p ../libs/supercollider/data
#cp -R ./supercollider/SCClassLibrary ../libs/supercollider/data/
mkdir -p ../data
cp -R ./supercollider/SCClassLibrary ../data/

