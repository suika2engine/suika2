#!/bin/bash

set -eu

for f in tests/*.scr; do
    echo -n "Running $f ... "
    ./wms $f > out
    diff $f.out out
    rm out
    echo "passed."
done

echo ''
echo 'All tests are passed!'
