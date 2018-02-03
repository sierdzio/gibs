INCLUDEPATH += $$PWD

mac:!ios {
    ICON = $$PWD/ibs.icns
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}
