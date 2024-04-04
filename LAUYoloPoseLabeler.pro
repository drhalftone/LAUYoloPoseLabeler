CONFIG   += c++11
CONFIG   += opencv

DEFINES  += #ZOOMINTOHEAD

QT       += core gui widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    lauimage.cpp \
    laucmswidget.cpp \
    laumemoryobject.cpp \
    laudeepnetworkobject.cpp \
    lauyoloposelabelerwidget.cpp

HEADERS += \
    lauimage.h \
    laucmswidget.h \
    laumemoryobject.h \
    laudeepnetworkobject.h \
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

    opencv {
        DEFINES       += USE_OPENCV ENABLEDEEPNETWORK
        INCLUDEPATH   += /usr/local/opt/opencv/include/opencv4
        DEPENDPATH    += /usr/local/opt/opencv/include/opencv4
        LIBS          += -L/usr/local/opt/opencv/lib -lopencv_core -lopencv_objdetect -lopencv_imgproc \
                          -lopencv_calib3d -lopencv_highgui -lopencv_ml -lopencv_dnn -lopencv_imgcodecs
    }
}

win32 {
    INCLUDEPATH += $$quote(C:/usr/include)
    DEPENDPATH  += $$quote(C:/usr/include)
    LIBS +=      -L$$quote(C:/usr/lib) -ltiff -llcms2 -lopengl32

    INCLUDEPATH += $$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/include)
    DEPENDPATH  += $$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/include)
    LIBS +=      -L$$quote(C:/Program Files (x86)/Intel/oneAPI/ipp/latest/lib/intel64) -lippi -lipps -lippcore

    opencv {
       DEFINES       += USE_OPENCV ENABLEDEEPNETWORK
       INCLUDEPATH   += $$quote(C:/usr/opencv/include)
       DEPENDPATH    += $$quote(C:/usr/opencv/include)
       LIBS          += -L$$quote(C:/usr/opencv/x64/vc16/lib)
       CONFIG(release, debug|release): LIBS += -lopencv_core481 -lopencv_objdetect481 -lopencv_imgproc481 -lopencv_calib3d481 \
                                               -lopencv_highgui481 -lopencv_ml481 -lopencv_face481 -lopencv_dnn481 -lopencv_imgcodecs481
       CONFIG(debug, debug|release):   LIBS += -lopencv_core481d -lopencv_objdetect481d -lopencv_imgproc481d -lopencv_calib3d481d \
                                               -lopencv_highgui481d -lopencv_ml481d -lopencv_face481d -lopencv_dnn481d -lopencv_imgcodecs481d
    }
}

# security find-identity
#~/Qt/5.15.2/clang_64/bin/macdeployqt 'LAU3DVideoRecorder.app' -codesign='Developer ID Application: Daniel Lau (UJ5B8V852W)' -dmg -appstore-compliant -always-overwrite
