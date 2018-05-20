#!/bin/bash

# Bail on errors.
# set -e

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
LOG="$PWD/compilation-summary.log"
DETAILS="$PWD/compilation-details.log"

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

cleanUp() {
  rm -f .ibs.cache *.o moc_* .qmake* Makefile
}

# Clear log file
echo "" > $LOG
echo "" > $DETAILS

# Remove build dir
rm -rf build/

for dir in ../testData/* ; do
  SOURCE=../../../testData/$dir
  CURRENT=build/$dir
  # echo "Source: $SOURCE destination: $CURRENT"
  mkdir -p $CURRENT
  cd $CURRENT
  echo "Entered: $dir"
  # echo "Cleanup"
  # TODO: also remove moc def file
  cleanUp
  echo "IBS compiling: $dir/main.cpp" | tee --append $LOG $DETAILS # >/dev/null
  RESULT=$(/usr/bin/time --append --output=$LOG --format="%E %U %S" $IBSEXE -j $JOBS --qt-dir $QTDIR $SOURCE/main.cpp >> $DETAILS 2>&1)

  if [ ! -z $RESULT ]; then
    echo "FAILED" >> $LOG
  fi

  if [ -f "$QMAKEEXE" ]; then
    echo "QMAKE compiling: $dir" | tee --append $LOG $DETAILS # >/dev/null
    RESULT=$(/usr/bin/time --append --output=$LOG --format="%E %U %S" sh -c "$QMAKEEXE $SOURCE/ && make -j $JOBS" >> $DETAILS 2>&1)

    if [ ! -z $RESULT ]; then
      echo "FAILED" >> $LOG
    fi
  fi
  cleanUp
  echo "Finished: $dir"
  cd ../..
done

cat $LOG

# Remove build dir
rm -rf build/

echo "Done"
