#!/usr/bin/env sh

g++ -std=c++17 -Wall -I./deps/include -l:libhiredis.so.0.13 -o ./test/seed_data ./test/seed_data.cpp