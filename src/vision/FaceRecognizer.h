#pragma once // 防止头文件被重复包含。
#include "storage/DatabaseManager.h" // 引入数据库管理器。
#include "vision/FaceFeatureExtractor.h" // 引入特征提取器。
#include "vision/FaceTypes.h" // 引入识别类型。
#include "vision/LivenessDetector.h" // 引入活体检测器。
#include <QImage> // 引入 Qt 图像类型。
#include <QObject> // 引入 Qt 对象基类。
#include <QVector> // 引入 Qt 动态数组类型。
class FaceRecognizer : public QObject { // 定义人脸识别器。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit FaceRecognizer(QObject* parent = nullptr); // 构造识别器。
    void setCosineThreshold(double threshold); // 设置余弦阈值。
    void setEuclideanThreshold(double threshold); // 设置欧氏阈值。
    void setQualityThreshold(double threshold); // 设置质量阈值。
    void setUseDlib(bool enabled); // 设置是否启用 dlib。
    QVector<FaceDetection> detectFaces(const QImage& image) const; // 检测图像中的人脸。
    RecognitionResult recognize(const QImage& image); // 识别单张图像中的人脸。
    bool enrollPerson(Person person, const QImage& image); // 从当前图像录入人员特征。
    QVector<Person> enrolledPersons() const; // 获取已注册人员列表。
signals: // 声明信号。
    void logMessage(const QString& level, const QString& message); // 发出识别日志。
private: // 声明私有工具。
    FaceDetection buildFallbackDetection(const QImage& image) const; // 构造降级检测结果。
    double evaluateQuality(const QImage& face) const; // 评估图像质量。
    RecognitionResult matchAgainstDatabase(const QImage& face, const FaceDetection& detection); // 执行特征匹配。
    FaceFeatureExtractor extractor_; // 保存特征提取器。
    LivenessDetector liveness_; // 保存活体检测器。
    double cosineThreshold_ = 0.82; // 保存余弦阈值。
    double euclideanThreshold_ = 0.75; // 保存欧氏阈值。
    double qualityThreshold_ = 0.35; // 保存质量阈值。
    bool useDlib_ = false; // 保存是否启用 dlib。
}; // 结束人脸识别器定义。
