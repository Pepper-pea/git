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

如果本机缺少 OpenCV、dlib 或 Qt MQTT，项目会自动使用降级实现。摄像头会优先使用 OpenCV；未启用 OpenCV 时，会自动尝试 Qt Multimedia 调用电脑摄像头；两者都不可用时才会显示演示画面。MQTT 会退化为 HTTP/本地缓存，核心业务流程仍可运行。

## 本地 SQLite 数据库

程序会在项目目录下创建 `data/smartsite.sqlite`，Navicat Premium 17 可以直接打开这个文件。打开方式：新建连接，选择 SQLite，然后选择现有数据库文件 `D:\MyGitWarehouse\git\data\smartsite.sqlite`。

主要数据表：

- `persons`：人员库和人脸特征。
- `recognized_faces`：每一次识别人脸的记录，包含姓名、识别状态、通行判定、分数、活体结果、人脸框坐标和抓拍图片路径。
- `attendance_records`：允许通行后生成的考勤记录。
- `device_logs`：设备运行日志。

数据库表结构脚本见 [docs/sqlite_schema.sql](docs/sqlite_schema.sql)。

## 人脸录入和识别

在 Qt Creator 中运行后，先点击“启动”，等左侧摄像头画面出现。右侧“人员录入”区域填写姓名、班组、角色、证件号等信息，点击“录入当前人脸”，程序会从当前画面提取人脸特征并保存到 `persons` 表。之后识别流程会自动使用这个人员库进行匹配，识别明细会继续写入 `recognized_faces` 表。

如果日志显示“使用演示画面”，请在 Qt 安装维护工具中确认已安装 Qt Multimedia 模块，并检查 Windows 设置里的摄像头权限。如果日志显示“图像质量不足”，请让人脸尽量位于画面中央，并保证光线充足。

完整目录说明见 [docs/PROJECT_SUMMARY.md](docs/PROJECT_SUMMARY.md)。
