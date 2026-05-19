#pragma once // 防止头文件被重复包含。
#include "domain/Person.h" // 引入人员信息结构。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QMetaType> // 引入 Qt 元类型支持。
#include <QPointF> // 引入浮点坐标点类型。
#include <QRect> // 引入矩形区域类型。
#include <QString> // 引入 Qt 字符串类型。
#include <QVector> // 引入 Qt 动态数组类型。
enum class RecognitionStatus { // 定义识别状态枚举。
    Accepted, // 表示识别成功且允许通行。
    Denied, // 表示识别成功但权限拒绝。
    Stranger, // 表示未匹配到库内人员。
    Spoof, // 表示活体检测未通过。
    LowQuality // 表示图像质量不足。
}; // 结束识别状态枚举。
struct FaceDetection { // 定义人脸检测结果。
    QRect rect; // 保存人脸矩形区域。
    QVector<QPointF> landmarks; // 保存人脸关键点。
    double quality = 0.0; // 保存图像质量得分。
    double eyeAspectRatio = 0.0; // 保存眼睛纵横比。
    bool blinkDetected = false; // 保存是否检测到眨眼。
}; // 结束人脸检测结果结构。
struct RecognitionResult { // 定义完整识别结果。
    RecognitionStatus status = RecognitionStatus::Stranger; // 保存识别状态。
    Person person; // 保存匹配到的人员。
    FaceDetection detection; // 保存检测到的人脸信息。
    double cosine = 0.0; // 保存余弦相似度。
    double euclidean = 0.0; // 保存欧氏距离。
    double score = 0.0; // 保存综合评分。
    QString message; // 保存面向界面的结果说明。
    QDateTime createdAt; // 保存识别发生时间。
}; // 结束识别结果结构。
Q_DECLARE_METATYPE(RecognitionResult) // 将识别结果声明为 Qt 元类型。
