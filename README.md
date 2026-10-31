# Overview

SwingAnalyzer is a desktop application for frame-accurate, side-by-side comparison of two golf swing videos. It provides precise synchronization, independent and joint playback controls, and responsive visual feedback to support coaching, analysis, and self-review workflows.

Functionality
- Load and manage two independent video channels (left/right)
- SYNC mode for aligning swings via per-channel play/pause/step/seek
- PLAYBACK mode for synchronized joint playback with shared controls
- Frame offset calculation and framerate ratio handling
- Variable playback rate and single/multi-frame stepping
- Scrollbar updates and position callbacks for real-time GUI feedback
- Threaded playback to maintain smooth rendering and UI responsiveness

Use cases
- Coach–athlete review: align and compare two swings from different sessions
- Before/after comparison: evaluate changes from drills or equipment adjustments
- Reference vs. current: compare against a model swing or previous best
- Club/lie/ball position experiments: assess impact across variations

Building

The project uses CMake and is built via the provided shell scripts from the repository root.

Prerequisites
- CMake and a C++ toolchain (make, g++/clang++)
- OpenCV, GTK/Goocanvas and Google Test available on the system
- `lcov` and `genhtml` (optional, only needed for coverage reports)

Main application

Build the `swing_analyzer` target using `build.sh`:

```bash
./build.sh                # Debug build (default)
./build.sh -b Release     # Release build
```

The script creates a `build/` directory if needed, runs the CMake configuration with the chosen build type, and builds only the `swing_analyzer` target.

Unit tests

Build and run the Google Test suite using `buildAndRunUnitTests.sh`:

```bash
./buildAndRunUnitTests.sh                          # build & run all unit tests
./buildAndRunUnitTests.sh -t "SynchronizerState*"   # run tests matching a filter
./buildAndRunUnitTests.sh -m coverage              # build & run with coverage, generate report
```

The script configures a Debug build, builds the `unit_tests` target, and runs the test executable. Use `-t` to pass a `--gtest_filter` expression (`*` works as a wildcard) and `-m coverage` to build the `unit_tests_coverage` target instead and generate an HTML coverage report in `build/coverage/index.html`.

Requirements
- Functional
    - Load videos per channel and report load status
    - Independent channel control in SYNC; shared control in PLAYBACK
    - Frame-accurate seek, step, and synchronized position updates
    - Observer-driven GUI updates for position and playback state
    - User-selectable playback rate and toggle between modes
- Non-functional
    - Smooth playback for typical HD footage using threaded timing control
    - Deterministic state transitions via explicit state machines
    - Pluggable video backends and GUI updaters (OpenCV decoder, GTK/Goocanvas)
    - Safe concurrency for frame buffers and playback control
    - Clear separation of concerns (GUI, controller, engine, video components)
