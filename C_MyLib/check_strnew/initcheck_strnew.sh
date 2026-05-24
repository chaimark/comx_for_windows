#!/bin/bash
sudo apt update
sudo apt install gcc-11-plugin-dev
g++ -I`gcc -print-file-name=plugin`/include -fPIC -shared -fno-rtti -O2 check_stack_escape.cpp -o check_stack_escape.so


