#!/bin/bash

set -e

wickedc test.wckd -o test.funk

rm -rf emcc_build
mkdir emcc_build
cd emcc_build
emcc -O2 ../../src/*.c ../../src/libvm/*.c ../../src/libvm/instructions/*.c -s USE_SDL=2 -o funky.html \
    -I../../include \
    --embed-file ../test.funk@test.funk \
    --pre-js ../init.js \
    -s WASM=0 -s PRECISE_F32=1



    #-s EMULATE_FUNCTION_POINTER_CASTS=1
    #-s ALIASING_FUNCTION_POINTERS=0
