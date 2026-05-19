#pragma once // 防止头文件被重复包含。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QMetaType> // 引入 Qt 元类型声明支持。
#include <QRect> // 引入矩形区域类型。
#include <QString> // 引入 Qt 字符串类型。
struct RecognizedFaceRecord { // 定义每一次人脸识别落库记录。
    QString id; // 保存识别记录唯一编号。
    qint64 frameId = 0; // 保存对应的视频帧序号。
    QString personId; // 保存匹配到的人员编号。
    QString personName; // 保存匹配到的人员姓名。
    QString team; // 保存人员班组。
    QString status; // 保存识别状态文本。
    bool accessAllowed = false; // 保存本次是否允许通行。
    QString decision; // 保存最终通行判定说明。
    QString message; // 保存识别算法返回的原始说明。
    double score = 0.0; // 保存综合识别得分。
    double cosine = 0.0; // 保存余弦相似度。
    double euclidean = 0.0; // 保存欧氏距离。
    double quality = 0.0; // 保存图像质量得分。
    bool blinkDetected = false; // 保存是否检测到眨眼活体。
    QRect faceRect; // 保存人脸在画面中的矩形区域。
    QString snapshotPath; // 保存本地抓拍图片路径。
    QString deviceId; // 保存设备编号。
    QDateTime createdAt; // 保存识别发生时间。
}; // 结束识别记录结构定义。
Q_DECLARE_METATYPE(RecognizedFaceRecord) // 将识别记录结构声明为 Qt 元类型。
