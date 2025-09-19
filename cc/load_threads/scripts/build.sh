#!/bin/bash

# g++ -static-libstdc++ -static-libgcc -O3 load_threads_v1.cc -o load_threads.bin
# taskset -c 0 ./load_threads.bin

rm load_threads.bin
podman rmi lt-builder
podman build -f scripts/dockerfiles/build -t lt-builder .
CONTAINER_ID=$(podman create lt-builder)
podman cp $CONTAINER_ID:/build/load_threads.bin ./load_threads.bin
podman rm $CONTAINER_ID