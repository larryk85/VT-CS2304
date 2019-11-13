#! /bin/bash

mkdir -p bin &> /dev/null
pushd bin &> /dev/null
cmake ..
make -j8
popd &> /dev/null

./deploy.sh