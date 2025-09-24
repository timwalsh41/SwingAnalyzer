#!/bin/bash
set -euo pipefail

# Default build type
BUILD_TYPE="Debug"

# Parse arguments
while getopts "b:" opt; do
    case $opt in
        b)
            BUILD_TYPE=${OPTARG}
            ;;
        *)
            echo "Usage: $0 [-b build_type]"
            echo "Build types: Debug (default), Release"
            exit 1
            ;;
    esac
done

# Create the build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Change to the build directory
cd build

# Configure the project with CMake
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

# Build only the swing_analyzer target
time make swing_analyzer
