include($${PWD}/modules/modules.pri)

PROG_NAME     = doer

VERSION_MAJOR = 1
VERSION_MINOR = 2
VERSION_PATCH = 0

VERSION       = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

QT           += core gui network widgets
CONFIG       += c++11
TEMPLATE      = app

DESTDIR       = build

INC_DIR       = inc
SRC_DIR       = src
RES_DIR       = res
FORMS_DIR     = ui

OBJECTS_DIR   = .objs
MOC_DIR       = .mocs
UI_DIR        = .uis
RCC_DIR       = .qrcs

INCLUDEPATH  += $${INC_DIR}

HEADERS      += $${INC_DIR}/mainwindow.hpp

SOURCES      += $${SRC_DIR}/main.cpp \
                $${SRC_DIR}/mainwindow.cpp

FORMS        += $${FORMS_DIR}/mainwindow.ui

RESOURCES     = $${RES_DIR}/resources.qrc

OTHER_FILES  += $${RES_DIR}/images/background.png \
                $${RES_DIR}/images/$${PROG_NAME}.svg \
                $${RES_DIR}/stylesheets/doer.qss

QMAKE_CLEAN  += -r $${DESTDIR}/$${PROG_NAME}

DEFINES      += PROG_NAME=\\\"$${PROG_NAME}\\\"

DEFINES      += "VERSION_MAJOR=$$VERSION_MAJOR" \
                "VERSION_MINOR=$$VERSION_MINOR" \
                "VERSION_PATCH=$$VERSION_PATCH" \

DEFINES      += VERSION=\\\"$${VERSION}\\\"

# CONFIG       += debug

unix:!mac {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    BINDIR  = $${PREFIX}/bin
    DATADIR = $${PREFIX}/share

    OTHER_FILES += dist/$${PROG_NAME}.desktop

    target.path = $${BINDIR}

    INSTALLS += target

    desktop.path = $${DATADIR}/applications
    eval(desktop.files += dist/$${PROG_NAME}.desktop)

    INSTALLS += desktop

    icon.path = $${DATADIR}/icons/hicolor/scalable/apps
    eval(icon.files += $${RES_DIR}/images/$${PROG_NAME}.svg)

    INSTALLS += icon
}
