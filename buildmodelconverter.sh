#!/bin/bash
common_link_flags="-lGL -ldl -lglad "
lib_include="-I/home/epic/Development/libs -I. -I/home/epic/Development/libs/glad/include"
gladdlinkpath="-L/home/epic/Development/libs/glad/dynamic"
gladdlinkpath="-L/home/epic/Development/libs/glad/static"

g++ modelconverter.cpp -g -std=c++11 -fno-exceptions $lib_include -o avconverter $gladdlinkpath $common_link_flags -lassimp -lpthread

