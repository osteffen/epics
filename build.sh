#!/bin/bash -e

DEPS="libreadline-dev re2c libpcre3-dev"

echo "====== Installing dependencies ======="
sudo apt-get install ${DEPS}

cd base
make all install -j4

cd ../extensions
make all install -j4
