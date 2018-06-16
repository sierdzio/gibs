#!/bin/bash

# Bail on errors.
set -e

if [ "${1}" = "-h" ] || [ "${1}" = "--help" ]; then
  echo "Usage: deploy-linux.sh linuxdeployqt_path qmake_path gibs_desktop_file [verbosity_level]"
  echo ""
  echo "Prepares gibs linux package (AppImage)."
  echo "If given, verbosity_level should be in the same format as linuxdeployqt"
  echo "expects it to be."
  echo ""
  echo "You have to run this script from the same directory where gibs.desktop"
  echo "file is located."
  exit
fi

DEPLOY=$1
QMAKE_PATH=$2
DESKTOP_FILE=$3
VERBOSE=1

if [ -z "$DEPLOY" ]; then
  echo "ERROR: linuxdeployqt binary not specified!"
  exit 1
fi

if [ ! -f "$DEPLOY" ]; then
  echo "ERROR: linuxdeployqt binary is not a file: $DEPLOY"
  exit 2
fi

if [ -z "$QMAKE_PATH" ]; then
  echo "ERROR: qmake path not specified!"
  exit 3
fi

if [ ! -f "$DESKTOP_FILE" ]; then
  echo "ERROR: gibs.desktop not found! $DESKTOP_FILE"
  exit 3
fi

if [ ! -z "$4" ]; then
  VERBOSE=$4
fi

$DEPLOY "$DESKTOP_FILE" -verbose=$VERBOSE -qmake="$QMAKE_PATH" -no-translations -no-copy-copyright-files -appimage
