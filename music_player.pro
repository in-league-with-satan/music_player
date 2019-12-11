QT += \
    core \
    gui \
    widgets \
    multimedia \
    network \
    sql \
    svg

TARGET = music_player
TEMPLATE = app
CONFIG += c++14

DESTDIR = $$PWD/bin


LINK_OPT=shared
BUILD_OPT=release

static {
    LINK_OPT=static
}

CONFIG(debug, debug|release):{
    BUILD_OPT=debug

} else {
    DEFINES += QT_NO_DEBUG_OUTPUT
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/rcc


include($$PWD/externals/3rdparty/ffmpeg/ffmpeg.pri)

exists($$PWD/externals/3rdparty/hotkey/qhotkey.pri) {
    include($$PWD/externals/3rdparty/hotkey/qhotkey.pri)
    DEFINES += LIB_HOTKEY
}

INCLUDEPATH += \
    $$PWD/src

SOURCES += \
    $$PWD/src/*.cpp

HEADERS += \
    $$PWD/src/*.h

RESOURCES += \
    $$PWD/images.qrc
