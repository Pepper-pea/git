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

项目已打包 OpenCV 4.8.0 MSVC x64 Release 运行库，使用 Qt Creator 的 Release 构建时会自动启用 OpenCV。Debug 构建不会混用这个 Release 版 OpenCV；如果 Debug 也要启用 OpenCV，需要本地额外配置 Debug 版 OpenCV 或使用 Git LFS 托管较大的 Debug DLL。dlib 仍需在 qmake 时传入 `DLIB_INCLUDEPATH`、`DLIB_LIBS`。

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

## 项目内 OpenCV

OpenCV 已放在项目内：

- `third_party/opencv/include/opencv2`
- `third_party/opencv/x64/vc16/lib/opencv_world480.lib`
- `third_party/opencv/x64/vc16/bin/opencv_world480.dll`

请在 Qt Creator 中切到 `Release` 后清理项目、重新运行 qmake、重新构建。构建完成后，`opencv_world480.dll` 会自动复制到可执行程序目录。

## 真实模型文件

当前识别流程要求真实使用 OpenCV Haar 和 dlib 68 点模型。Haar 模型已随项目放到 `models/haarcascade_frontalface_default.xml`。dlib 模型仍需放到 `D:\MyGitWarehouse\git\models`：

- `shape_predictor_68_face_landmarks.dat`

可以运行脚本下载缺失模型：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\download_models.ps1
```

程序启动时会按 `config/device_config.json` 中的 `models.faceCascade` 和 `models.shapePredictor68` 加载模型。缺少 Haar 模型时无法检测人脸；缺少 dlib 68 点模型时无法完成 68 点关键点和活体流程。

完整目录说明见 [docs/PROJECT_SUMMARY.md](docs/PROJECT_SUMMARY.md)。
