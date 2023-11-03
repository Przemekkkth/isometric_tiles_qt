greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    src/fontmanager.cpp \
    src/gamescene.cpp \
    src/main.cpp \
    src/pixmapmanager.cpp \
    src/view.cpp

HEADERS += \
    src/resourceholder.h \
    src/fontmanager.h \
    src/gamescene.h \
    src/pixmapmanager.h \
    src/utils.h \
    src/view.h

RESOURCES += \
    resource.qrc
