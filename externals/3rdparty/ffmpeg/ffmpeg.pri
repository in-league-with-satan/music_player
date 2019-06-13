linux {
    GCCFLAGS += -lz

    INCLUDEPATH += $$PWD/include
    LIBS += -L$$PWD/lib

    LIBS += -lavformat -lavcodec -lavutil -lavfilter -lswresample
    LIBS += -ldl -lvorbis -lvorbisenc -logg -lfdk-aac -lopus
}

windows {
    INCLUDEPATH += $$PWD/include
    LIBS += -L$$PWD/lib
    LIBS += -lavformat -lavcodec -lavutil -lavfilter -lswresample
}
