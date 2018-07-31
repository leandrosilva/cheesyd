#!/usr/bin/env sh

g++ -Wall -I./deps/include -lwkhtmltox -l:libhiredis.so.0.13 -o ./bin/cheesyd ./src/cheesyd.cpp ./src/**/*.cpp