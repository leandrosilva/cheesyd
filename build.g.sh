#!/usr/bin/env sh

g++ -g -std=c++17 -Wall -I./deps/include -lwkhtmltox -l:libhiredis.so.0.13 -ljson11 -o ./bin/cheesyd ./src/*.cpp