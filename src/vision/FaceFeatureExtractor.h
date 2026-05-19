#pragma once // 防止头文件被重复包含。
#include <QImage> // 引入 Qt 图像类型。
#include <QRect> // 引入矩形区域类型。
#include <QVector> // 引入 Qt 动态数组类型。
class FaceFeatureExtractor { // 定义人脸特征提取器。
public: // 声明公共接口。
    QVector<double> extract(const QImage& image, const QRect& faceRect) const; // 从图像和人脸区域提取特征向量。
    static double cosineSimilarity(const QVector<double>& left, const QVector<double>& right); // 计算两个特征向量的余弦相似度。
    static double euclideanDistance(const QVector<double>& left, const QVector<double>& right); // 计算两个特征向量的欧氏距离。
private: // 声明私有工具函数。
    static QVector<double> normalize(QVector<double> values); // 对特征向量做 L2 归一化。
}; // 结束人脸特征提取器定义。
