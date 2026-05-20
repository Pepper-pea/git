# 模型文件目录

本目录保存识别模型：

- `haarcascade_frontalface_default.xml`：OpenCV Haar 人脸检测模型，已随项目打包。
- `shape_predictor_68_face_landmarks.dat`：dlib 68 点关键点模型，用于 EAR 眨眼活体检测，需要自行下载或通过脚本下载。

当前识别流程要求真实加载这两个模型。缺少 Haar 模型时无法检测人脸；缺少 dlib 68 点模型时无法录入人员，也无法通过活体关键点流程。

可在项目根目录运行脚本下载模型：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\download_models.ps1
```

默认配置在 `config/device_config.json`：

```json
"models": {
  "faceCascade": "models/haarcascade_frontalface_default.xml",
  "shapePredictor68": "models/shape_predictor_68_face_landmarks.dat"
}
```
