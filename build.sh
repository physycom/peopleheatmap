#!/bin/bash

if [[ $(hostname) == *"osmino"* ]]; then
  export FLTK_DIR=/disk01/fltk/fltk-1.3.4
  export FLTK_INCLUDE_DIR=/disk01/fltk/fltk-1.3.4/include
  export FLTK_BIN_DIR=/disk01/fltk/fltk-1.3.4/bin
  export FLTK_LIBRARY_SEARCH_PATH=/disk01/fltk/fltk-1.3.4/lib
  export FLTK_LIBRARY_DIR=/disk01/fltk/fltk-1.3.4/lib
fi

if [[ "$OSTYPE" == "darwin"* && "$1" == "gcc" ]]; then
  export CC="/usr/local/bin/gcc-8"
  export CXX="/usr/local/bin/g++-8"
fi

mkdir -p build
cd build
cmake .. -DOVERRIDE_FLTK_INSTALL_DIR:BOOL=TRUE
cmake --build . --target install
cd ..
