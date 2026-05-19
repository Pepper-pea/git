# 模型文件目录

可把以下模型放到本目录：

- `shape_predictor_68_face_landmarks.dat`：dlib 68 点关键点模型，用于 EAR 眨眼活体检测。
- `haarcascade_frontalface_default.xml`：OpenCV Haar 人脸检测模型，用于没有 dlib 时的降级检测。

没有模型时，系统会启用演示级检测逻辑，方便先跑通考勤、数据库、通信和界面流程。
