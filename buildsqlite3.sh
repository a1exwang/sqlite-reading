#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR
mkdir -p build/sqlite3 && cd build/sqlite3

../../sqlite/configure
make -j8
