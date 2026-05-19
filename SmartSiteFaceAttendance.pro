# qmake 项目文件，用于 Qt Creator 或命令行 qmake 构建。
QT += core widgets sql network

# 如果当前 Qt 安装包含 MQTT 模块，就自动启用 MQTT。
qtHaveModule(mqtt) {
    QT += mqtt
    DEFINES += SMARTSITE_HAS_MQTT
}

# 使用 C++17，并打开常规警告。
CONFIG += c++17
CONFIG += warn_on
TEMPLATE = app
TARGET = SmartSiteFaceAttendance

# 项目头文件搜索路径。
INCLUDEPATH += $$PWD/src

# 可选 OpenCV：qmake 时传入 OPENCV_INCLUDEPATH 和 OPENCV_LIBS 即可启用。
!isEmpty(OPENCV_INCLUDEPATH) {
    DEFINES += SMARTSITE_HAS_OPENCV
    INCLUDEPATH += $$OPENCV_INCLUDEPATH
}
!isEmpty(OPENCV_LIBS) {
    LIBS += $$OPENCV_LIBS
}

# 可选 dlib：qmake 时传入 DLIB_INCLUDEPATH 和 DLIB_LIBS 即可启用。
!isEmpty(DLIB_INCLUDEPATH) {
    DEFINES += SMARTSITE_HAS_DLIB
    INCLUDEPATH += $$DLIB_INCLUDEPATH
}
!isEmpty(DLIB_LIBS) {
    LIBS += $$DLIB_LIBS
}

# 程序源文件。
SOURCES += \
    src/main.cpp \
    src/camera/CameraWorker.cpp \
    src/utils/MatImageConverter.cpp \
    src/vision/FaceFeatureExtractor.cpp \
    src/vision/LivenessDetector.cpp \
    src/vision/FaceRecognizer.cpp \
    src/storage/DatabaseManager.cpp \
    src/business/AttendanceManager.cpp \
    src/hardware/AccessController.cpp \
    src/comm/CloudClient.cpp \
    src/monitor/SystemMonitor.cpp \
    src/ui/MainWindow.cpp

# 程序头文件。
HEADERS += \
    src/camera/FramePacket.h \
    src/camera/CameraWorker.h \
    src/domain/Person.h \
    src/domain/AttendanceRecord.h \
    src/utils/MatImageConverter.h \
    src/vision/FaceTypes.h \
    src/vision/FaceFeatureExtractor.h \
    src/vision/LivenessDetector.h \
    src/vision/FaceRecognizer.h \
    src/storage/DatabaseManager.h \
    src/business/AttendanceManager.h \
    src/hardware/AccessController.h \
    src/comm/CloudClient.h \
    src/monitor/SystemMonitor.h \
    src/ui/MainWindow.h
