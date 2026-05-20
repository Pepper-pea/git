#pragma once // 防止头文件被重复包含。
#include "vision/FaceTypes.h" // 引入人脸检测结果类型。
#include <QDateTime> // 引入 Qt 日期时间类型。
class LivenessDetector { // 定义基于眨眼 EAR 的活体检测器。
public: // 声明公共接口。
    void setEarThreshold(double threshold); // 设置眼睛纵横比闭眼阈值。
    void setMouthOpenThreshold(double threshold); // 设置张嘴比例阈值。
    void setTurnOffsetThreshold(double threshold); // 设置转头偏移阈值。
    void setStableFrameCount(int frames); // 设置动作需要连续通过的帧数。
    void reset(); // 重置活体检测状态。
    bool update(FaceDetection& detection); // 输入检测结果并更新活体判断。
    bool updateChallenge(FaceDetection& detection, EnrollmentChallengeType challenge); // 输入检测结果并判断录入动作是否通过。
    bool measurePose(FaceDetection& detection) const; // 只计算头部和嘴部姿态，不改变活体检测状态。
    double calculateEar(const QVector<QPointF>& landmarks) const; // 根据 68 点关键点计算 EAR。
    double calculateMouthOpenRatio(const QVector<QPointF>& landmarks) const; // 根据 68 点关键点计算张嘴比例。
    double calculateHeadTurnOffset(const QVector<QPointF>& landmarks) const; // 根据 68 点关键点计算转头偏移。
private: // 声明私有数据。
    double earThreshold_ = 0.21; // 保存 EAR 闭眼阈值。
    double mouthOpenThreshold_ = 0.45; // 保存张嘴比例阈值。
    double turnOffsetThreshold_ = 0.18; // 保存转头偏移阈值。
    int stableFrameCount_ = 2; // 保存连续通过帧数要求。
    int closedFrames_ = 0; // 保存连续闭眼帧数。
    int stableFrames_ = 0; // 保存当前动作连续通过帧数。
    QDateTime lastBlinkAt_; // 保存最近一次眨眼时间。
    double distance(const QPointF& left, const QPointF& right) const; // 计算两个关键点距离。
}; // 结束活体检测器定义。
