#!/bin/bash

podman rmi lt-runner
podman build -f scripts/dockerfiles/run -t lt-runner .
podman run --cpus="0.5" --rm -it lt-runner