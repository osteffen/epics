#!/bin/bash -e

cd base
make all install -j4

cd ../extensions
make all install -j4
