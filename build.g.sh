#!/usr/bin/env sh

g++ -g -std=c++17 -Wall -I./deps/include -lwkhtmltox -l:libhiredis.so.0.13 -o ./bin/cheesyd ./src/*.cpp ./deps/lib/json11.a