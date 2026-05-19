#pragma once // 防止头文件被重复包含。
#include <QImage> // 引入 Qt 图像类型。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/core.hpp> // 引入 OpenCV Mat 类型。
#endif // 结束 OpenCV 判断。
class MatImageConverter { // 定义 Mat 与 QImage 转换工具类。
public: // 声明公共接口。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    static QImage matToQImage(const cv::Mat& mat); // 将 OpenCV Mat 转换为 Qt QImage。
    static cv::Mat qImageToMat(const QImage& image); // 将 Qt QImage 转换为 OpenCV Mat。
#endif // 结束 OpenCV 判断。
}; // 结束转换工具类定义。
