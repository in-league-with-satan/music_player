QT += \
    core \
    gui \
    widgets \
    multimedia

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

}

OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/rcc


include($$PWD/externals/3rdparty/ffmpeg/ffmpeg.pri)

INCLUDEPATH += \
    $$PWD/src

SOURCES += \
    $$PWD/src/*.cpp

HEADERS += \
    $$PWD/src/*.h