#!/bin/bash

DEFAULT_ARG="/home/tim/Desktop/PXL_20240329_005944373.TS.mp4"
ARGUMENT="${1:-$DEFAULT_ARG}"
build/gtk_gstreamer_test "$ARGUMENT"
