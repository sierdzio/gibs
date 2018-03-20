CONFIG += tests

TEMPLATE = subdirs

SUBDIRS += ibs \
    qtheadermapper

tests {
    !android {
        CONFIG(debug, debug|release) {
            message("Running test suite")
            SUBDIRS += tests \
        }
    }
}
