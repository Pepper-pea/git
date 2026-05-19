#pragma once // 防止头文件被重复包含。
#include "domain/AttendanceRecord.h" // 引入考勤记录结构。
#include "storage/DatabaseManager.h" // 引入数据库管理器。
#include "vision/FaceTypes.h" // 引入识别结果结构。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QHash> // 引入哈希容器。
#include <QObject> // 引入 Qt 对象基类。
#include <QSet> // 引入集合容器。
#include <QString> // 引入 Qt 字符串类型。
class AttendanceManager : public QObject { // 定义考勤业务管理器。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit AttendanceManager(QObject* parent = nullptr); // 构造考勤管理器。
    void setDeviceId(const QString& deviceId); // 设置设备编号。
    void setDuplicateSeconds(int seconds); // 设置重复打卡时间窗口。
    void setScoreThresholds(double cosine, double euclidean); // 设置识别阈值。
    void setRequiredLiveness(bool enabled); // 设置是否强制活体检测。
    AttendanceRecord createRecord(const RecognitionResult& result, const QString& direction, const QString& snapshotPath); // 生成考勤记录。
    bool shouldAllow(const RecognitionResult& result) const; // 判断是否允许通行。
    QString explainDecision(const RecognitionResult& result) const; // 生成判定说明。
    bool isDuplicate(const QString& personId, const QString& direction) const; // 判断是否重复打卡。
    void rememberAttendance(const AttendanceRecord& record); // 记录最近一次打卡。
    void clearCache(); // 清空缓存。
signals: // 声明信号。
    void recordCreated(const AttendanceRecord& record); // 通知生成了新记录。
    void logMessage(const QString& level, const QString& message); // 通知生成日志消息。
private: // 声明私有工具。
    QString buildResultText(const RecognitionResult& result) const; // 根据识别结果生成状态文本。
    QString buildRecordId() const; // 生成记录编号。
    QString buildSnapshotFileName(const RecognitionResult& result) const; // 生成抓拍文件名。
    QString deviceId_; // 保存设备编号。
    int duplicateSeconds_ = 60; // 保存重复打卡时间窗口。
    double cosineThreshold_ = 0.82; // 保存余弦阈值。
    double euclideanThreshold_ = 0.75; // 保存欧氏距离阈值。
    bool requireLiveness_ = true; // 保存是否强制活体检测。
    mutable QHash<QString, QDateTime> lastAttendance_; // 保存最近考勤时间。
}; // 结束考勤管理器定义。
