#!/bin/bash
rm game
common_link_flags="-lSDL2 -lGL -ldl -l:libglad.a "
#to compile it yourself you need dev libs for glad, also change your paths
gladdlinkpath="-L/home/epic/Development/libs/glad/dynamic"
gladdlinkpath="-L/home/epic/Development/libs/glad/static"
lib_include="-I/home/epic/Development/libs -I. -I/home/epic/Development/libs/glad/include"

o_options="-g"
# o_options="-O3 -DNDEBUG"

g++ src/main.cpp -std=c++11 -fno-exceptions \
    $common_link_flags \
    $gladdlinkpath \
    -lpthread \
    -o game \
    $o_options \
    -DAV_GL_GLAD \
    $lib_include && echo "Build completed successfully"
