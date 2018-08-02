#!/usr/bin/env sh

docker container start cheesyd_debug
docker container exec cheesyd_debug /usr/sbin/sshd -p 1122