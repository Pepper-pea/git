# 项目目录说明

## 根目录
- `CMakeLists.txt`：Qt6/C++ 项目构建入口。
- `SmartSiteFaceAttendance.pro`：qmake 项目文件，方便用 Qt Creator 或 qmake 配置。
- `README.md`：项目简介和构建说明。

## config
- `device_config.json`：设备编号、摄像头、阈值、MQTT/HTTP 地址和本地路径配置。

## models
- `README.md`：模型文件说明。

## src/domain
- `Person.h`：人员资料结构和 JSON 序列化。
- `AttendanceRecord.h`：考勤记录结构和 JSON 序列化。
- `RecognizedFaceRecord.h`：每一次识别人脸的本地 SQLite 落库结构。

## src/camera
- `FramePacket.h`：视频帧数据结构。
- `CameraWorker.h/cpp`：摄像头采集线程，优先使用 OpenCV，未启用 OpenCV 时使用 Qt Multimedia，最后降级到演示画面。

## src/utils
- `MatImageConverter.h/cpp`：OpenCV Mat 与 Qt QImage 转换工具。

## src/vision
- `FaceTypes.h`：识别状态、检测结果、识别结果定义。
- `FaceFeatureExtractor.h/cpp`：人脸特征提取与相似度计算。
- `LivenessDetector.h/cpp`：EAR 眨眼活体检测。
- `FaceRecognizer.h/cpp`：人脸检测、识别、人员录入。

## src/storage
- `DatabaseManager.h/cpp`：SQLite 数据库连接、人员库、识别人脸记录、考勤记录、日志管理。

## src/business
- `AttendanceManager.h/cpp`：考勤规则、重复打卡控制、记录生成、通行判定。

## src/hardware
- `AccessController.h/cpp`：闸机、指示灯、蜂鸣器联动控制，支持模拟模式。

## src/comm
- `CloudClient.h/cpp`：MQTT/HTTP 云端同步、状态上报、记录上传、OTA 与配置拉取。

## src/monitor
- `SystemMonitor.h/cpp`：心跳、健康检查、看门狗触发和系统监控。

## src/ui
- `MainWindow.h/cpp`：主界面，负责视频、人员录入、日志、人员库、考勤记录和状态显示。

## src
- `main.cpp`：Qt 应用入口。

## 运行特点
- 有 OpenCV、dlib、Qt MQTT 时使用增强路径。
- 未配置 OpenCV 时，若 Qt 安装包含 Multimedia 模块，也可以直接调用电脑自带摄像头。
- 缺少依赖时会自动降级到演示逻辑，保证主流程仍可打开。
- 本地数据库固定创建在项目目录 `data/smartsite.sqlite`，可用 Navicat Premium 17 以 SQLite 文件方式打开。
- 右侧“人员录入”区域可把当前摄像头画面录入到 `persons` 表，后续识别会直接匹配新人员。
- `recognized_faces` 表保存每次识别结果和抓拍图片路径，抓拍文件保存在 `data/captures`。
