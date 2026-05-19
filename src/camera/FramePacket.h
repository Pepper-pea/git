#pragma once // 防止头文件被重复包含。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QImage> // 引入 Qt 图像类型。
#include <QMetaType> // 引入 Qt 元类型注册支持。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/core.hpp> // 引入 OpenCV Mat 类型。
#endif // 结束 OpenCV 判断。
struct FramePacket { // 定义跨线程传递的视频帧数据包。
    qint64 frameId = 0; // 保存帧序号。
    QDateTime capturedAt; // 保存采集时间。
    QImage image; // 保存用于界面显示和识别的 Qt 图像。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    cv::Mat mat; // 保存用于算法预处理的 OpenCV 图像。
#endif // 结束 OpenCV 判断。
    bool isValid() const { // 判断当前帧是否有效。
        return !image.isNull(); // 返回 Qt 图像是否非空。
    } // 结束有效性判断函数。
}; // 结束视频帧数据包结构定义。
Q_DECLARE_METATYPE(FramePacket) // 将视频帧数据包声明为 Qt 元类型。
