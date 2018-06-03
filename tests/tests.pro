# Main qmake configuration file for unit tests

TEMPLATE = subdirs

# Add new test cases here:
SUBDIRS += \
	../milo/mconfig/tst_mconfig \
	../milo/mlog/tst_mlog \
    tst_gibs


# Please use convenience includes:
# testConfig.pri - contains general configs
# helpers/helpers.pri - contains property tester, great for testing QObjects

# Tests can be run in Qt Creator (enable AutoTest plugin, then run tests from
# panel 8). Tests can be run from command line. Just run:
# $ make check
# In build directory (where root Makefile is).
