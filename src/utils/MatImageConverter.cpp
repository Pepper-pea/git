#include "utils/MatImageConverter.h" // 引入 Mat 与 QImage 转换工具声明。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/imgproc.hpp> // 引入 OpenCV 色彩空间转换函数。
QImage MatImageConverter::matToQImage(const cv::Mat& mat) { // 实现 Mat 到 QImage 的转换。
    if(mat.empty()) { // 判断 Mat 是否为空。
        return QImage(); // 返回空图像表示转换失败。
    } // 结束空 Mat 判断。
    if(mat.type() == CV_8UC1) { // 判断是否为单通道灰度图。
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8); // 按灰度格式包装图像。
        return image.copy(); // 返回深拷贝避免 Mat 生命周期问题。
    } // 结束灰度图处理。
    if(mat.type() == CV_8UC3) { // 判断是否为三通道 BGR 图。
        cv::Mat rgb; // 创建 RGB 临时图。
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB); // 将 BGR 转为 RGB。
        QImage image(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888); // 按 RGB 格式包装图像。
        return image.copy(); // 返回深拷贝避免临时变量失效。
    } // 结束三通道图处理。
    if(mat.type() == CV_8UC4) { // 判断是否为四通道 BGRA 图。
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32); // 按 ARGB 格式包装图像。
        return image.copy(); // 返回深拷贝避免 Mat 生命周期问题。
    } // 结束四通道图处理。
    cv::Mat converted; // 创建格式转换后的临时图。
    mat.convertTo(converted, CV_8UC3); // 将未知格式尽量转成 8 位三通道。
    return matToQImage(converted); // 递归使用标准三通道转换流程。
} // 结束 Mat 到 QImage 转换函数。
cv::Mat MatImageConverter::qImageToMat(const QImage& image) { // 实现 QImage 到 Mat 的转换。
    if(image.isNull()) { // 判断 QImage 是否为空。
        return cv::Mat(); // 返回空 Mat 表示转换失败。
    } // 结束空图像判断。
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888); // 将图像统一转成 RGB888。
    cv::Mat rgb(rgbImage.height(), rgbImage.width(), CV_8UC3, const_cast<uchar*>(rgbImage.bits()), static_cast<size_t>(rgbImage.bytesPerLine())); // 用 QImage 像素创建 Mat 视图。
    cv::Mat bgr; // 创建 BGR 输出图。
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR); // 将 RGB 转为 OpenCV 常用的 BGR。
    return bgr.clone(); // 返回深拷贝保证内存安全。
} // 结束 QImage 到 Mat 转换函数。
#endif // 结束 OpenCV 判断。
