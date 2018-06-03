CONFIG += tests

TEMPLATE = subdirs

SUBDIRS += gibs \
    qtheadermapper

tests {
    !android {
        CONFIG(debug, debug|release) {
            message("Running test suite")
            SUBDIRS += tests \
        }
    }
}
