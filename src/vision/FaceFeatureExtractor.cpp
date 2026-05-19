#include "vision/FaceFeatureExtractor.h" // 引入人脸特征提取器声明。
#include <QColor> // 引入颜色读取工具。
#include <QtMath> // 引入 Qt 数学函数。
QVector<double> FaceFeatureExtractor::extract(const QImage& image, const QRect& faceRect) const { // 实现特征提取函数。
    if(image.isNull() || faceRect.isEmpty()) { // 判断输入图像或人脸区域是否无效。
        return {}; // 返回空特征表示提取失败。
    } // 结束输入检查。
    QRect safeRect = faceRect.intersected(image.rect()); // 将人脸区域限制在图像范围内。
    if(safeRect.isEmpty()) { // 判断裁剪后的区域是否为空。
        return {}; // 返回空特征表示提取失败。
    } // 结束安全区域检查。
    QImage face = image.copy(safeRect).convertToFormat(QImage::Format_Grayscale8).scaled(16, 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation); // 裁剪人脸并缩放为固定大小灰度图。
    QVector<double> values; // 创建原始特征值数组。
    values.reserve(face.width() * face.height()); // 预留特征向量容量。
    for(int y = 0; y < face.height(); ++y) { // 遍历缩放后图像的每一行。
        const uchar* line = face.constScanLine(y); // 取得当前行像素指针。
        for(int x = 0; x < face.width(); ++x) { // 遍历当前行的每一列。
            values.append(static_cast<double>(line[x]) / 255.0); // 将灰度值归一到 0 到 1 后加入向量。
        } // 结束列遍历。
    } // 结束行遍历。
    double mean = 0.0; // 创建均值变量。
    for(double value : values) { // 遍历全部特征值。
        mean += value; // 累加特征值用于计算均值。
    } // 结束均值累加。
    mean = values.isEmpty() ? 0.0 : mean / static_cast<double>(values.size()); // 计算灰度均值。
    for(double& value : values) { // 遍历全部特征值引用。
        value -= mean; // 减去均值以降低光照影响。
    } // 结束去均值处理。
    return normalize(values); // 返回归一化后的特征向量。
} // 结束特征提取函数。
double FaceFeatureExtractor::cosineSimilarity(const QVector<double>& left, const QVector<double>& right) { // 实现余弦相似度计算。
    const int count = qMin(left.size(), right.size()); // 取两个向量的共同长度。
    if(count == 0) { // 判断是否没有可比较维度。
        return 0.0; // 返回零相似度。
    } // 结束空向量判断。
    double dot = 0.0; // 创建点积变量。
    double leftNorm = 0.0; // 创建左向量范数变量。
    double rightNorm = 0.0; // 创建右向量范数变量。
    for(int index = 0; index < count; ++index) { // 遍历每一个维度。
        dot += left[index] * right[index]; // 累加点积。
        leftNorm += left[index] * left[index]; // 累加左向量平方和。
        rightNorm += right[index] * right[index]; // 累加右向量平方和。
    } // 结束维度遍历。
    if(leftNorm <= 0.0 || rightNorm <= 0.0) { // 判断是否存在零向量。
        return 0.0; // 返回零相似度避免除零。
    } // 结束零范数判断。
    return dot / (qSqrt(leftNorm) * qSqrt(rightNorm)); // 返回余弦相似度。
} // 结束余弦相似度函数。
double FaceFeatureExtractor::euclideanDistance(const QVector<double>& left, const QVector<double>& right) { // 实现欧氏距离计算。
    const int count = qMin(left.size(), right.size()); // 取两个向量的共同长度。
    if(count == 0) { // 判断是否没有可比较维度。
        return 999.0; // 返回大距离表示不可匹配。
    } // 结束空向量判断。
    double sum = 0.0; // 创建平方差累加变量。
    for(int index = 0; index < count; ++index) { // 遍历每一个维度。
        const double diff = left[index] - right[index]; // 计算当前维度差值。
        sum += diff * diff; // 累加平方差。
    } // 结束维度遍历。
    return qSqrt(sum); // 返回欧氏距离。
} // 结束欧氏距离函数。
QVector<double> FaceFeatureExtractor::normalize(QVector<double> values) { // 实现 L2 归一化函数。
    double norm = 0.0; // 创建范数变量。
    for(double value : values) { // 遍历每一个特征值。
        norm += value * value; // 累加平方值。
    } // 结束平方累加。
    norm = qSqrt(norm); // 计算向量范数。
    if(norm <= 0.000001) { // 判断范数是否过小。
        return values; // 返回原值避免除零。
    } // 结束范数检查。
    for(double& value : values) { // 遍历每一个特征值引用。
        value /= norm; // 将特征值除以范数完成归一化。
    } // 结束归一化遍历。
    return values; // 返回归一化后的向量。
} // 结束 L2 归一化函数。
