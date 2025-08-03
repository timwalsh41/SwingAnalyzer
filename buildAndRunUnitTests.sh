#!/bin/bash
set -euo pipefail

# Variables - adjust these as needed for your project
BUILD_DIR="build"
UNIT_TEST_EXECUTABLE="unit_tests"  # Change this if your test binary has a different name
GTEST_FILTER=""

# Parse arguments
while getopts "t:" opt; do
    case $opt in
        t)
            # If a test filter is provided, set it for Google Test
            # Example usage: -t "TestSuite.TestName"
            # (Use * as a wildcard for multiple tests, e.g. SycnchronizerState*)
            GTEST_FILTER="--gtest_filter=${OPTARG}"
            ;;
        *)
            echo "Usage: $0 [-t test_filter]"
            exit 1
            ;;
    esac
done

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: ${BUILD_DIR}"
    mkdir "$BUILD_DIR"
fi

# Change to the build directory
cd "$BUILD_DIR"

# Configure the project with CMake
echo "Running CMake configuration..."
cmake ..

# Build only the unit tests target
echo "Building the unit tests..."
make "${UNIT_TEST_EXECUTABLE}"

# Run the unit tests
echo "Running unit tests..."
if [ -x "./${UNIT_TEST_EXECUTABLE}" ]; then
    time ./${UNIT_TEST_EXECUTABLE} ${GTEST_FILTER}
else
    echo "Unit test executable '${UNIT_TEST_EXECUTABLE}' not found or not executable."
    echo "If using CTest, you can run: ctest --output-on-failure"
fi

echo 
echo "Unit tests completed."