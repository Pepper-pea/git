#pragma once // 防止头文件被重复包含。
#include "vision/FaceTypes.h" // 引入人脸检测结果类型。
#include <QDateTime> // 引入 Qt 日期时间类型。
class LivenessDetector { // 定义基于眨眼 EAR 的活体检测器。
public: // 声明公共接口。
    void setEarThreshold(double threshold); // 设置眼睛纵横比闭眼阈值。
    bool update(FaceDetection& detection); // 输入检测结果并更新活体判断。
    double calculateEar(const QVector<QPointF>& landmarks) const; // 根据 68 点关键点计算 EAR。
private: // 声明私有数据。
    double earThreshold_ = 0.21; // 保存 EAR 闭眼阈值。
    int closedFrames_ = 0; // 保存连续闭眼帧数。
    QDateTime lastBlinkAt_; // 保存最近一次眨眼时间。
    double distance(const QPointF& left, const QPointF& right) const; // 计算两个关键点距离。
}; // 结束活体检测器定义。
