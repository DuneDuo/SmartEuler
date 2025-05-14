QT       += core gui widgets network
CONFIG   += c++17
QT += core network
QT += multimedia
QT += svg
QT += concurrent
CONFIG += console
CONFIG -= app_bundle
CONFIG += opencv_optimize
CONFIG += debug
DEFINES += QT_DEBUG
QMAKE_CXXFLAGS += -Od  # MSVC禁用优化
QMAKE_CXXFLAGS += /Zi    # 生成PDB文件
QMAKE_CXXFLAGS_RELEASE -= -MT
QMAKE_CXXFLAGS_RELEASE += -MD
QMAKE_CXXFLAGS_DEBUG -= -MTd
QMAKE_CXXFLAGS_DEBUG += -MDd
QMAKE_LFLAGS += /DEBUG:FULL    # 启用调试链接
QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
QMAKE_LFLAGS_RELEASE += /INCREMENTAL:NO
TARGET = SmartEuler
TEMPLATE = app
QMAKE_CXXFLAGS += /utf-8

# OpenCV配置（需修改为你的安装路径）
# OpenCV配置（必须严格区分Debug/Release）
win32 {
    OPENCV_PATH = D:/Application/opencv/build
    QMAKE_CXXFLAGS += /openmp
    INCLUDEPATH += $$OPENCV_PATH/include
    CONFIG(debug, debug|release) {
        LIBS += -L$$OPENCV_PATH/x64/vc16/lib -lopencv_world480d
    } else {
        LIBS += -L$$OPENCV_PATH/x64/vc16/lib -lopencv_world480
    }
}

# 确保生成moc文件
CONFIG += c++11



SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
