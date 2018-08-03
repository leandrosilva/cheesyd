#!/usr/bin/env sh

# static library
# g++ -c json11.cpp -o json11.o
# ar rcs json11.a json11.o
# mv json11.a ../../lib

# shared library
g++ -shared -o ../../lib/libjson11.so -fPIC json11.cpp
cp ../../lib/libjson11.so /usr/local/lib
/sbin/ldconfig -v

# usage as static
# g++ -std=c++11 -Wall main.cpp json11.a

# usage as shared
# g++ -std=c++1 -Wall -ljson11 main.cpp