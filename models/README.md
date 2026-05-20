# 模型文件目录

本目录保存识别模型：

- `haarcascade_frontalface_default.xml`：OpenCV Haar 人脸检测模型，已随项目打包。
- `shape_predictor_68_face_landmarks.dat`：dlib 68 点关键点模型，用于 EAR 眨眼活体检测，已随项目打包。

当前识别流程要求真实加载这两个模型。缺少 Haar 模型时无法检测人脸；缺少 dlib 68 点模型时无法录入人员，也无法通过活体关键点流程。

注意：`shape_predictor_68_face_landmarks.dat` 来自官方 `davisking/dlib-models` 仓库，其训练数据带有非商业用途限制；正式商业使用前请替换为许可匹配的模型或重新训练模型。

默认配置在 `config/device_config.json`：

```json
"models": {
  "faceCascade": "models/haarcascade_frontalface_default.xml",
  "shapePredictor68": "models/shape_predictor_68_face_landmarks.dat"
}
```
