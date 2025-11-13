#!/bin/bash

check_binary() {
    if [ -f main.bin ]; then
        ls -lh main.bin
    else
        echo "main.bin not found!!!"
    fi
}

echo -e "\e[1;32m===> Building with C++11 \e[0m"
make clean
make CXX_STANDARD=c++11
check_binary

echo -e "\e[1;32m===> Building with C++14 \e[0m"
make clean
make CXX_STANDARD=c++14
check_binary

echo -e "\e[1;32m===> Building with C++17 \e[0m"
make clean
make CXX_STANDARD=c++17
check_binary

echo -e "\e[1;32m===> Building without SIM_ERROR \e[0m"
make clean
make
check_binary

echo -e "\e[1;32m===> Building with SIM_ERROR \e[0m"
make clean
make SIM_ERROR=1
check_binary