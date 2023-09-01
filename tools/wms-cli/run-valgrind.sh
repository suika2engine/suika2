#!/bin/bash

set -eu

for f in tests/*.scr; do
    echo "Running $f ..."
    valgrind --leak-check=full --error-exitcode=1 ./wms $f
done

echo ''
echo 'All tests are passed!'
