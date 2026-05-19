cmake_minimum_required(VERSION 3.21) # 指定项目需要的最低 CMake 版本。
project(SmartSiteFaceAttendance VERSION 1.0.0 LANGUAGES CXX) # 定义智慧工地人脸识别实名制考勤系统项目。
set(CMAKE_CXX_STANDARD 17) # 指定使用 C++17 标准。
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 要求编译器必须支持 C++17。
set(CMAKE_AUTOMOC ON) # 自动处理 Qt 的元对象代码生成。
set(CMAKE_AUTORCC ON) # 自动处理 Qt 资源文件。
set(CMAKE_AUTOUIC OFF) # 本项目使用纯代码界面，不启用 ui 文件自动处理。
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network) # 查找 Qt6 核心、界面、数据库和网络模块。
find_package(Qt6 COMPONENTS Mqtt QUIET) # 可选查找 Qt MQTT 模块。
find_package(Qt6 COMPONENTS Multimedia QUIET) # 可选查找 Qt Multimedia 摄像头模块。
find_package(OpenCV QUIET) # 可选查找 OpenCV 图像处理库。
find_package(dlib CONFIG QUIET) # 可选查找 dlib 人脸检测与关键点库。
set(SMART_SITE_SOURCES # 声明项目源码列表。
    src/main.cpp # 程序入口源码。
    src/camera/CameraWorker.cpp # 摄像头采集线程源码。
    src/utils/MatImageConverter.cpp # OpenCV Mat 与 Qt 图像转换源码。
    src/vision/FaceFeatureExtractor.cpp # 人脸特征提取源码。
    src/vision/LivenessDetector.cpp # 眨眼活体检测源码。
    src/vision/FaceRecognizer.cpp # 人脸检测识别源码。
    src/storage/DatabaseManager.cpp # SQLite 数据库管理源码。
    src/business/AttendanceManager.cpp # 考勤业务规则源码。
    src/hardware/AccessController.cpp # 闸机和声光联动源码。
    src/comm/CloudClient.cpp # MQTT 与 HTTP 云端通信源码。
    src/monitor/SystemMonitor.cpp # 看门狗和远程维护源码。
    src/ui/MainWindow.cpp # 主界面源码。
) # 结束源码列表。
add_executable(SmartSiteFaceAttendance ${SMART_SITE_SOURCES}) # 生成智慧工地考勤终端可执行程序。
target_include_directories(SmartSiteFaceAttendance PRIVATE src) # 将 src 目录加入头文件搜索路径。
target_link_libraries(SmartSiteFaceAttendance PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network) # 链接 Qt 基础模块。
if(Qt6Mqtt_FOUND) # 判断当前环境是否提供 Qt MQTT 模块。
    target_compile_definitions(SmartSiteFaceAttendance PRIVATE SMARTSITE_HAS_MQTT) # 启用 MQTT 编译开关。
    target_link_libraries(SmartSiteFaceAttendance PRIVATE Qt6::Mqtt) # 链接 Qt MQTT 模块。
endif() # 结束 MQTT 判断。
if(Qt6Multimedia_FOUND) # 判断当前环境是否提供 Qt Multimedia 模块。
    target_compile_definitions(SmartSiteFaceAttendance PRIVATE SMARTSITE_HAS_QTMULTIMEDIA) # 启用 Qt Multimedia 摄像头采集。
    target_link_libraries(SmartSiteFaceAttendance PRIVATE Qt6::Multimedia) # 链接 Qt Multimedia 模块。
endif() # 结束 Qt Multimedia 判断。
if(OpenCV_FOUND) # 判断当前环境是否提供 OpenCV。
    target_compile_definitions(SmartSiteFaceAttendance PRIVATE SMARTSITE_HAS_OPENCV) # 启用 OpenCV 编译开关。
    target_include_directories(SmartSiteFaceAttendance PRIVATE ${OpenCV_INCLUDE_DIRS}) # 加入 OpenCV 头文件目录。
    target_link_libraries(SmartSiteFaceAttendance PRIVATE ${OpenCV_LIBS}) # 链接 OpenCV 库。
endif() # 结束 OpenCV 判断。
if(dlib_FOUND) # 判断当前环境是否提供 dlib。
    target_compile_definitions(SmartSiteFaceAttendance PRIVATE SMARTSITE_HAS_DLIB) # 启用 dlib 编译开关。
    target_link_libraries(SmartSiteFaceAttendance PRIVATE dlib::dlib) # 链接 dlib 库。
endif() # 结束 dlib 判断。
if(MSVC) # 判断是否使用 Visual Studio 编译器。
    target_compile_options(SmartSiteFaceAttendance PRIVATE /W4 /permissive-) # 为 MSVC 打开较严格的警告。
else() # 处理非 MSVC 编译器。
    target_compile_options(SmartSiteFaceAttendance PRIVATE -Wall -Wextra -pedantic) # 为 GCC 或 Clang 打开常用警告。
endif() # 结束编译器判断。
