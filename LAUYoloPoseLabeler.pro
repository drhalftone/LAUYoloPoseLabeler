QT       += core gui widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    lauimage.cpp \
    laucmswidget.cpp \
    lauyoloposelabelerwidget.cpp

HEADERS += \
    lauimage.h \
    laucmswidget.h \
    lauyoloposelabelerwidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:macx {
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder
    INCLUDEPATH    += /usr/local/include/Tiff /opt/intel/ipp/include
    DEPENDPATH     += /usr/local/include/Tiff /opt/intel/ipp/include
    LIBS           += /usr/local/lib/libtiff.dylib /usr/local/lib/liblcms2.dylib

    LIBS += /opt/intel/ipp/lib/libippi.a \
            /opt/intel/ipp/lib/libipps.a \
            /opt/intel/ipp/lib/libippcore.a \
            /opt/intel/ipp/lib/libippcc.a \
            /opt/intel/ipp/lib/libippcv.a \
            /opt/intel/ipp/lib/libippvm.a \
            /opt/intel/ipp/lib/libippdc.a \
            /opt/intel/ipp/lib/libippch.a \
            /opt/intel/lib/libirc.a

    flyseyelens {
        ICON    = FlysEye/hexLensIcon.icns
    }

    lenticularlens {
        ICON    = FlysEye/LauLensIcon.icns
    }
}

win32 {
    INCLUDEPATH += $$quote(C:/usr/include)
    DEPENDPATH  += $$quote(C:/usr/include)
    LIBS +=      -L$$quote(C:/usr/lib) -ltiff -llcms2 -lopengl32

    INCLUDEPATH += $$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/include)
    DEPENDPATH  += $$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/include)
    LIBS +=      -L$$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/lib/intel64) -lippi -lipps -lippcore
}

# security find-identity
#~/Qt/5.15.2/clang_64/bin/macdeployqt 'LAU3DVideoRecorder.app' -codesign='Developer ID Application: Daniel Lau (UJ5B8V852W)' -dmg -appstore-compliant -always-overwrite
