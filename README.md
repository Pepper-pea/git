# 智慧工地人脸识别实名制考勤系统

这是一个 Qt6/C++ 终端软件项目，覆盖摄像头采集、OpenCV 图像预处理、dlib/OpenCV 人脸检测、录入活体动作、SQLite 离线库、考勤规则、闸机声光联动、MQTT/HTTP 云端同步、看门狗和 OTA 维护等模块。

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

项目已打包 OpenCV 4.8.0 MSVC x64 Release 运行库，使用 Qt Creator 的 Release 构建时会自动启用 OpenCV。Debug 构建不会混用这个 Release 版 OpenCV；如果 Debug 也要启用 OpenCV，需要本地额外配置 Debug 版 OpenCV 或使用 Git LFS 托管较大的 Debug DLL。dlib 20.0.1 源码已放在项目内，qmake/CMake 会自动编译并启用 68 点关键点接口。

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

在 Qt Creator 中运行后，先点击“启动”，等左侧摄像头画面出现。右侧“人员录入”区域填写姓名、班组、角色、证件号等信息，点击“录入当前人脸”，程序会随机要求眨眼、向左转头、向右转头或张嘴，动作通过后自动提取人脸特征并保存到 `persons` 表。转头方向按使用者本人左右理解，不按屏幕画面左右理解。之后普通识别流程不要求眨眼，会直接按人脸匹配结果、名单类型和重复打卡规则决定是否开门，识别明细会继续写入 `recognized_faces` 表。

如果日志显示“使用演示画面”，请在 Qt 安装维护工具中确认已安装 Qt Multimedia 模块，并检查 Windows 设置里的摄像头权限。如果日志显示“图像质量不足”，请让人脸尽量位于画面中央，并保证光线充足。

## 项目内 OpenCV

OpenCV 已放在项目内：

- `third_party/opencv/include/opencv2`
- `third_party/opencv/x64/vc16/lib/opencv_world480.lib`
- `third_party/opencv/x64/vc16/bin/opencv_world480.dll`

请在 Qt Creator 中切到 `Release` 后清理项目、重新运行 qmake、重新构建。构建完成后，`opencv_world480.dll` 会自动复制到可执行程序目录。

## 项目内 dlib

dlib 已放在项目内：

- `third_party/dlib/dlib`
- `third_party/dlib/dlib/all/source.cpp`
- `third_party/dlib/LICENSE.txt`

qmake/CMake 会自动加入 dlib 头文件、编译 `dlib/all/source.cpp`，并定义 `SMARTSITE_HAS_DLIB`。Windows 下会同时链接 dlib 需要的 `ws2_32` 和 `winmm` 系统库。

## 真实模型文件

当前识别流程要求真实使用 OpenCV Haar 和 dlib 68 点模型。两个模型均已随项目放到 `models`：

- `haarcascade_frontalface_default.xml`
- `shape_predictor_68_face_landmarks.dat`

程序启动时会按 `config/device_config.json` 中的 `models.faceCascade` 和 `models.shapePredictor68` 加载模型。缺少 Haar 模型时无法检测人脸；缺少 dlib 68 点模型时无法完成 68 点关键点和活体流程。

`config/device_config.json` 中的 `liveness.requireForRecognition` 默认是 `false`，表示普通门禁识别不强制眨眼。录入活体动作使用同一配置里的 `enrollmentChallengeTimeoutSeconds`、`mouthOpenRatio`、`turnOffsetRatio` 和 `stableFrames` 控制。

注意：项目内的 dlib 68 点模型来自官方 `davisking/dlib-models` 仓库。该模型对应训练数据带有非商业用途限制；如果项目要用于正式商业场景，请替换为许可匹配的模型或重新训练模型。

完整目录说明见 [docs/PROJECT_SUMMARY.md](docs/PROJECT_SUMMARY.md)。
