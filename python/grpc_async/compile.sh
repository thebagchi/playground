#!/bin/bash

# Activate the virtual environment
source venv/bin/activate

# Create pb directory if it doesn't exist
mkdir -p pb

# Compile the proto file
python -m grpc_tools.protoc --proto_path=. --python_out=./pb --grpc_python_out=./pb sample.proto

echo "Proto compilation completed."