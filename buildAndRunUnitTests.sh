#!/bin/bash
set -euo pipefail

# Variables - adjust these as needed for your project
BUILD_DIR="build"
UNIT_TEST_EXECUTABLE="unit_tests"  # Change this if your test binary has a different name
COVERAGE_EXECUTABLE="unit_tests_coverage"
GTEST_FILTER=""
MODE="unit-tests"  # Default mode

# Parse arguments
while getopts "t:m:" opt; do
    case $opt in
        t)
            # If a test filter is provided, set it for Google Test
            # Example usage: -t "TestSuite.TestName"
            # (Use * as a wildcard for multiple tests, e.g. SycnchronizerState*)
            GTEST_FILTER="--gtest_filter=${OPTARG}"
            ;;
        m)
            # Set the mode (unit-tests or coverage)
            MODE=${OPTARG}
            ;;
        *)
            echo "Usage: $0 [-t test_filter] [-m mode]"
            echo "Modes: unit-tests (default), coverage"
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
if [ "$MODE" == "coverage" ]; then
    echo "Running CMake configuration for coverage..."
    cmake -DCMAKE_BUILD_TYPE=Coverage ..
else
    echo "Running CMake configuration for unit tests..."
    cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

# Build the appropriate target
if [ "$MODE" == "coverage" ]; then
    echo "Building the unit_tests_coverage target..."
    make "$COVERAGE_EXECUTABLE"
else
    echo "Building the unit tests target..."
    make "$UNIT_TEST_EXECUTABLE"
fi

# Run the appropriate executable
if [ "$MODE" == "coverage" ]; then
    echo "Running coverage tests..."
    if [ -x "./${COVERAGE_EXECUTABLE}" ]; then
        time ./${COVERAGE_EXECUTABLE} ${GTEST_FILTER} || true
    else
        echo "Coverage executable '${COVERAGE_EXECUTABLE}' not found or not executable."
        exit 1
    fi

    # Generate coverage report
    if command -v lcov &> /dev/null && command -v genhtml &> /dev/null; then
        echo
        echo "Generating coverage report..."
        lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
        echo
        lcov --remove coverage.info '/usr/*' --output-file coverage.info --rc lcov_branch_coverage=1

        # Ensure the coverage directory exists
        echo "Ensuring coverage directory exists..."
        mkdir -p coverage

        genhtml coverage.info --output-directory coverage
        echo "Coverage report generated in ${BUILD_DIR}/coverage/index.html"
    else
        echo "lcov or genhtml not found. Please install them to generate coverage reports."
    fi
else
    echo "Running unit tests..."
    if [ -x "./${UNIT_TEST_EXECUTABLE}" ]; then
        time ./${UNIT_TEST_EXECUTABLE} ${GTEST_FILTER} || true
    else
        echo "Unit test executable '${UNIT_TEST_EXECUTABLE}' not found or not executable."
        echo "Try to run: ctest --output-on-failure"
    fi
fi

echo 
echo "Script completed."