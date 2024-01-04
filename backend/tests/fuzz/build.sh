#!/bin/bash -eu
# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

# build project
mkdir build
cd build
cmake -DZINT_STATIC=ON -DZINT_SHARED=OFF \
    -DZINT_USE_PNG=OFF -DZINT_USE_QT=OFF -DZINT_FRONTEND=OFF \
    -DCMAKE_BUILD_TYPE=Debug \
    ..
make -j$(nproc)

# build fuzzers
bd=$SRC/zint/backend

fd=$bd/tests/fuzz
fn=fuzz_data
$CC $CFLAGS -fsanitize=fuzzer-no-link -I${bd} $fd/${fn}.c -c -o $WORK/${fn}.o
$CXX $CXXFLAGS $WORK/${fn}.o -o $OUT/${fn} \
    $LIB_FUZZING_ENGINE -lm -lzint -Lbackend
cp $fd/${fn}_seed_corpus.zip $OUT

fn=fuzz_gs1
$CC $CFLAGS -fsanitize=fuzzer-no-link -I${bd} $fd/${fn}.c -c -o $WORK/${fn}.o
$CXX $CXXFLAGS $WORK/${fn}.o -o $OUT/${fn} \
    $LIB_FUZZING_ENGINE -lm -lzint -Lbackend
cp $fd/${fn}_seed_corpus.zip $OUT
cp $fd/${fn}.dict $OUT
