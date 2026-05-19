#include "vision/LivenessDetector.h" // 引入活体检测器声明。
#include <QtMath> // 引入 Qt 数学函数。
void LivenessDetector::setEarThreshold(double threshold) { // 实现 EAR 阈值设置函数。
    earThreshold_ = threshold; // 保存新的闭眼阈值。
} // 结束 EAR 阈值设置函数。
bool LivenessDetector::update(FaceDetection& detection) { // 实现活体状态更新函数。
    if(detection.landmarks.size() >= 68) { // 判断是否具备标准 68 点关键点。
        detection.eyeAspectRatio = calculateEar(detection.landmarks); // 根据关键点计算 EAR。
        if(detection.eyeAspectRatio > 0.0 && detection.eyeAspectRatio < earThreshold_) { // 判断当前帧是否闭眼。
            ++closedFrames_; // 增加连续闭眼帧数。
        } else { // 处理睁眼或关键点异常情况。
            if(closedFrames_ >= 2) { // 判断前面是否形成一次有效闭眼。
                detection.blinkDetected = true; // 标记检测到眨眼。
                lastBlinkAt_ = QDateTime::currentDateTime(); // 记录眨眼发生时间。
            } // 结束有效闭眼判断。
            closedFrames_ = 0; // 重置连续闭眼计数。
        } // 结束闭眼状态判断。
    } // 结束关键点处理。
    if(detection.blinkDetected) { // 判断当前检测结果是否已有眨眼标记。
        lastBlinkAt_ = QDateTime::currentDateTime(); // 刷新最近眨眼时间。
    } // 结束眨眼标记判断。
    if(lastBlinkAt_.isValid() && lastBlinkAt_.secsTo(QDateTime::currentDateTime()) <= 5) { // 判断 5 秒内是否有眨眼。
        return true; // 返回活体通过。
    } // 结束近期眨眼判断。
    return detection.blinkDetected; // 返回当前帧眨眼状态。
} // 结束活体状态更新函数。
double LivenessDetector::calculateEar(const QVector<QPointF>& landmarks) const { // 实现 EAR 计算函数。
    if(landmarks.size() < 68) { // 判断关键点数量是否不足。
        return 0.0; // 返回零表示无法计算。
    } // 结束关键点数量检查。
    const double leftEye = (distance(landmarks[37], landmarks[41]) + distance(landmarks[38], landmarks[40])) / (2.0 * distance(landmarks[36], landmarks[39])); // 计算左眼 EAR。
    const double rightEye = (distance(landmarks[43], landmarks[47]) + distance(landmarks[44], landmarks[46])) / (2.0 * distance(landmarks[42], landmarks[45])); // 计算右眼 EAR。
    return (leftEye + rightEye) / 2.0; // 返回双眼 EAR 平均值。
} // 结束 EAR 计算函数。
double LivenessDetector::distance(const QPointF& left, const QPointF& right) const { // 实现两点距离计算函数。
    const double dx = left.x() - right.x(); // 计算横向差值。
    const double dy = left.y() - right.y(); // 计算纵向差值。
    return qSqrt(dx * dx + dy * dy); // 返回欧氏距离。
} // 结束两点距离计算函数。
