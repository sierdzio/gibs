#!/bin/bash

# Bail on errors.
set -e

if [ "${1}" = "-h" ] || [ "${1}" = "--help" ]; then
  echo "Usage: run-compilation-tests.sh -q qt-directory -i ibs-exe-path "
  echo "[-j jobs] [-m qmake-path]"
  echo ""
  echo "Compiles all tests located in ibs/testData using -j jobs, -q Qt directory,"
  echo "-i ibs path and optional -m qmake path."
  echo ""
  echo "Where skin is optional. If specified, results will be limited to specified"
  echo "skin and global changes."
  exit
fi

JOBS="1"
QTDIR=""
IBSEXE=""
QMAKEEXE=""

while getopts "j:q:i:m:" opt ;
do
  case $opt in
  j) JOBS=$OPTARG
    ;;
  q) QTDIR=$OPTARG
    ;;
  i) IBSEXE=$OPTARG
    ;;
  m) QMAKEEXE=$OPTARG
    ;;
  :)
    echo "Option -$OPTARG requires an argument."
    exit 1
    ;;
  esac
done

for dir in ../testData/* ; do
  cd $dir
  echo "Entered: $dir"
  echo "Cleanup"
  # TODO: also remove moc def file
  rm -f .ibs.cache *.o moc_* .qmake* Makefile
  echo "IBS compiling: $dir/main.cpp"
  time $IBSEXE -j $JOBS --qt-dir $QTDIR main.cpp
  if [ -f "$QMAKEEXE" ]; then
    echo "QMAKE compiling: $dir/main.cpp $QMAKEEXE $JOBS"
    time sh -c "$QMAKEEXE && make -j $JOBS"
  fi
  rm -f .ibs.cache *.o moc_* .qmake* Makefile
  echo "Finished: $dir"
  cd ..
done

echo "Done"
