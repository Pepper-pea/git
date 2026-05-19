# 智慧工地人脸识别实名制考勤系统

这是一个 Qt6/C++ 终端软件项目，覆盖摄像头采集、OpenCV 图像预处理、dlib/OpenCV 人脸检测、眨眼活体、SQLite 离线库、考勤规则、闸机声光联动、MQTT/HTTP 云端同步、看门狗和 OTA 维护等模块。

推荐构建方式：

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="你的Qt安装目录"
cmake --build build --config Release
```

也可以使用 qmake：

```powershell
qmake SmartSiteFaceAttendance.pro
nmake
```

如果要启用 OpenCV 或 dlib，可在 qmake 时传入 `OPENCV_INCLUDEPATH`、`OPENCV_LIBS`、`DLIB_INCLUDEPATH`、`DLIB_LIBS`。

如果本机缺少 OpenCV、dlib 或 Qt MQTT，项目会自动使用降级实现：摄像头可用演示画面，MQTT 会退化为 HTTP/本地缓存，核心业务流程仍可运行。

完整目录说明见 [docs/PROJECT_SUMMARY.md](docs/PROJECT_SUMMARY.md)。
