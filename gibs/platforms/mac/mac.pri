INCLUDEPATH += $$PWD

mac:!ios {
    ICON = $$PWD/gibs.icns
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}
