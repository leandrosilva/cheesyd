#!/usr/bin/env sh

docker exec cheesyd_debug /source_code/build.g.sh
docker exec cheesyd_debug killall gdbserver &> /dev/null
docker exec cheesyd_debug gdbserver :9091 /source_code/bin/cheesyd