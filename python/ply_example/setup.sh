#!/bin/bash

# Check if venv directory exists
if [ ! -d "venv" ]; then
    echo "creating virtual environment..."
    python3 -m venv venv
fi

# Activate the virtual environment
echo "activating virtual environment..."
source venv/bin/activate
