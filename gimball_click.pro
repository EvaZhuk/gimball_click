QT       += core gui network
#QT += serialbus widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


INCLUDEPATH += $$PWD/c_library_v2
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clickablelabel.cpp \
    main.cpp \
    mainwindow.cpp \
    siyisender.cpp \
    udpstreamer.cpp

HEADERS += \
    clickablelabel.h \
    mainwindow.h \
    siyisender.h \
    udpstreamer.h

FORMS += \
    mainwindow.ui


# ---- OpenCV via pkg-config ----
unix {
    QMAKE_PKGCONFIG_PATH += /usr/local/lib/pkgconfig
    PKGCONFIG += opencv4
}

# ---- Fallback in case pkg-config is ignored by Qt Creator ----
INCLUDEPATH += /usr/local/include/opencv4
#LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc

LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_videoio \
        -lopencv_imgproc \
        -lopencv_tracking \
        -lopencv_features2d


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
