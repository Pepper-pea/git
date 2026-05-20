#include "vision/LivenessDetector.h" // 引入活体检测器声明。
#include <QtMath> // 引入 Qt 数学函数。
void LivenessDetector::setEarThreshold(double threshold) { // 实现 EAR 阈值设置函数。
    earThreshold_ = threshold; // 保存新的闭眼阈值。
} // 结束 EAR 阈值设置函数。
void LivenessDetector::setMouthOpenThreshold(double threshold) { // 实现张嘴比例阈值设置函数。
    mouthOpenThreshold_ = threshold; // 保存新的张嘴阈值。
} // 结束张嘴阈值设置函数。
void LivenessDetector::setTurnOffsetThreshold(double threshold) { // 实现转头偏移阈值设置函数。
    turnOffsetThreshold_ = threshold; // 保存新的转头阈值。
} // 结束转头阈值设置函数。
void LivenessDetector::setStableFrameCount(int frames) { // 实现连续通过帧数设置函数。
    stableFrameCount_ = qMax(1, frames); // 保存帧数并保证至少为一帧。
} // 结束连续帧数设置函数。
void LivenessDetector::reset() { // 实现活体检测状态重置函数。
    closedFrames_ = 0; // 清空闭眼帧计数。
    stableFrames_ = 0; // 清空动作稳定帧计数。
    lastBlinkAt_ = QDateTime(); // 清空最近眨眼时间。
} // 结束状态重置函数。
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
bool LivenessDetector::updateChallenge(FaceDetection& detection, EnrollmentChallengeType challenge) { // 实现录入动作挑战判断函数。
    if(detection.landmarks.size() < 68) { // 判断是否缺少 dlib 68 点关键点。
        stableFrames_ = 0; // 缺少关键点时重置稳定帧。
        return false; // 返回未通过。
    } // 结束关键点检查。
    if(challenge == EnrollmentChallengeType::Blink) { // 判断是否为眨眼动作。
        return update(detection); // 复用 EAR 眨眼检测。
    } // 结束眨眼动作判断。
    bool passedCurrentFrame = false; // 保存当前帧是否满足动作。
    if(challenge == EnrollmentChallengeType::OpenMouth) { // 判断是否为张嘴动作。
        detection.mouthOpenRatio = calculateMouthOpenRatio(detection.landmarks); // 计算嘴部张开比例。
        passedCurrentFrame = detection.mouthOpenRatio >= mouthOpenThreshold_; // 判断当前帧是否张嘴。
    } else { // 处理转头动作。
        detection.headTurnOffset = calculateHeadTurnOffset(detection.landmarks); // 计算鼻尖偏移。
        passedCurrentFrame = qAbs(detection.headTurnOffset) >= turnOffsetThreshold_; // 判断是否有足够明显的转头动作，避免镜像画面导致方向反向。
    } // 结束动作判断。
    stableFrames_ = passedCurrentFrame ? stableFrames_ + 1 : 0; // 更新连续通过帧数。
    return stableFrames_ >= stableFrameCount_; // 达到要求后返回通过。
} // 结束录入动作挑战判断函数。
bool LivenessDetector::measurePose(FaceDetection& detection) const { // 实现无状态姿态测量函数。
    if(detection.landmarks.size() < 68) { // 判断是否缺少 dlib 68 点关键点。
        return false; // 关键点不足时无法测量姿态。
    } // 结束关键点检查。
    detection.mouthOpenRatio = calculateMouthOpenRatio(detection.landmarks); // 计算嘴部张开比例。
    detection.headTurnOffset = calculateHeadTurnOffset(detection.landmarks); // 计算头部左右偏移。
    return true; // 返回测量成功。
} // 结束无状态姿态测量函数。
double LivenessDetector::calculateEar(const QVector<QPointF>& landmarks) const { // 实现 EAR 计算函数。
    if(landmarks.size() < 68) { // 判断关键点数量是否不足。
        return 0.0; // 返回零表示无法计算。
    } // 结束关键点数量检查。
    const double leftEye = (distance(landmarks[37], landmarks[41]) + distance(landmarks[38], landmarks[40])) / (2.0 * distance(landmarks[36], landmarks[39])); // 计算左眼 EAR。
    const double rightEye = (distance(landmarks[43], landmarks[47]) + distance(landmarks[44], landmarks[46])) / (2.0 * distance(landmarks[42], landmarks[45])); // 计算右眼 EAR。
    return (leftEye + rightEye) / 2.0; // 返回双眼 EAR 平均值。
} // 结束 EAR 计算函数。
double LivenessDetector::calculateMouthOpenRatio(const QVector<QPointF>& landmarks) const { // 实现张嘴比例计算函数。
    if(landmarks.size() < 68) { // 判断关键点数量是否不足。
        return 0.0; // 返回零表示无法计算。
    } // 结束关键点数量检查。
    const double mouthWidth = distance(landmarks[60], landmarks[64]); // 计算嘴部横向宽度。
    if(mouthWidth <= 0.0) { // 判断分母是否有效。
        return 0.0; // 返回零表示无法计算。
    } // 结束分母检查。
    return (distance(landmarks[62], landmarks[66]) + distance(landmarks[63], landmarks[65])) / (2.0 * mouthWidth); // 返回嘴部张开比例。
} // 结束张嘴比例计算函数。
double LivenessDetector::calculateHeadTurnOffset(const QVector<QPointF>& landmarks) const { // 实现转头偏移计算函数。
    if(landmarks.size() < 68) { // 判断关键点数量是否不足。
        return 0.0; // 返回零表示无法计算。
    } // 结束关键点数量检查。
    const double eyeWidth = qAbs(landmarks[45].x() - landmarks[36].x()); // 计算双眼外角水平距离。
    if(eyeWidth <= 0.0) { // 判断分母是否有效。
        return 0.0; // 返回零表示无法计算。
    } // 结束分母检查。
    const double eyeCenterX = (landmarks[36].x() + landmarks[45].x()) / 2.0; // 计算双眼中心横坐标。
    return (landmarks[30].x() - eyeCenterX) / eyeWidth; // 返回鼻尖相对双眼中心的横向偏移。
} // 结束转头偏移计算函数。
double LivenessDetector::distance(const QPointF& left, const QPointF& right) const { // 实现两点距离计算函数。
    const double dx = left.x() - right.x(); // 计算横向差值。
    const double dy = left.y() - right.y(); // 计算纵向差值。
    return qSqrt(dx * dx + dy * dy); // 返回欧氏距离。
} // 结束两点距离计算函数。
