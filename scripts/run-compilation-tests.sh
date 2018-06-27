#!/bin/bash

# Bail on errors.
# set -e

if [ "${1}" = "-h" ] || [ "${1}" = "--help" ]; then
  echo "Usage: run-compilation-tests.sh -q qt-directory -i gibs-exe-path "
  echo "[-j jobs] [-m qmake-path] [-d]"
  echo ""
  echo "Compiles all tests located in ibs/testData using -j jobs, -q Qt directory,"
  echo "-i ibs path and optional -m qmake path. -d will produce debug builds."
  echo ""
  echo "Where skin is optional. If specified, results will be limited to specified"
  echo "skin and global changes."
  exit
fi

JOBS="1"
QTDIR=""
GIBSEXE=""
QMAKEEXE=""
LOG="$PWD/compilation-summary.log"
DETAILS="$PWD/compilation-details.log"
CLEANUP="1"
DEBUG=""

while getopts "j:q:i:m:n" opt ;
do
  case $opt in
  j) JOBS=$OPTARG
    ;;
  q) QTDIR=$OPTARG
    ;;
  i) GIBSEXE=$OPTARG
    ;;
  m) QMAKEEXE=$OPTARG
    ;;
  n) CLEANUP=""
    ;;
  d) DEBUG="1"
      ;;
  :)
    echo "Option -$OPTARG requires an argument."
    exit 1
    ;;
  esac
done

cleanUp() {
  rm -f .gibs.cache *.o moc_* .qmake* Makefile
}

# Clear log file
echo "" > $LOG
echo "" > $DETAILS

# Remove build dir
rm -rf build/

for dir in ../samples/* ; do
  SOURCE=../../$dir
  CURRENT=build/$(basename $dir)
  mkdir -p $CURRENT
  cd $CURRENT
  #echo "Source: $SOURCE destination: $CURRENT now in: $PWD"
  echo "Entered: $dir"
  # TODO: also remove moc def file
  cleanUp

  # Use custom compilation steps, if provided
  CUSTOM_PATH="main.cpp"
  CUSTOM_ARGS=""
  QMAKE_CUSTOM_ARGS=""

  if [ -f "$SOURCE/custom-test-run.sh" ]; then
    echo "Extracting custom flags from custom-test-run.sh"
    source "$SOURCE/custom-test-run.sh"
  fi

  #echo "CUSTOM: $CUSTOM_PATH $CUSTOM_ARGS"
  if [ ! -z "$DEBUG" ]; then
    CUSTOM_ARGS+="--debug"
    QMAKE_CUSTOM_ARGS+="-debug"
  fi

  ts=$(date +%s%N)
  $GIBSEXE -j $JOBS --qt-dir $QTDIR $SOURCE/$CUSTOM_PATH $CUSTOM_ARGS >> $DETAILS 2>&1
  EXIT_CODE=$?
  tt=$((($(date +%s%N) - $ts)/1000000))
  echo "GIBS: $dir/main.cpp $tt" | tee --append $LOG $DETAILS # >/dev/null

  if [ "$EXIT_CODE" != "0" ]; then
    echo "GIBS failed with: $EXIT_CODE after $tt"
    exit $EXIT_CODE
  fi

  if [ -f "$QMAKEEXE" ]; then
    ts=$(date +%s%N)
    $QMAKEEXE $QMAKE_CUSTOM_ARGS $SOURCE/ && make -j $JOBS >> $DETAILS 2>&1
    EXIT_CODE=$?
    tt=$((($(date +%s%N) - $ts)/1000000))
    echo "QMAKE: $dir $tt" | tee --append $LOG $DETAILS # >/dev/null

    if [ "$EXIT_CODE" != "0" ]; then
      echo "QMAKE failed with: $EXIT_CODE after $tt"
      exit $EXIT_CODE
    fi
  fi

  if [ ! -z $CLEANUP ]; then
    cleanUp
  fi

  echo "Finished: $dir"
  cd ../..
done

cat $LOG

# Remove build dir
if [ ! -z $CLEANUP ]; then
    rm -rf build/
  fi

echo "Done"
