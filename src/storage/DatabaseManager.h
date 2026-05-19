#pragma once // 防止头文件被重复包含。
#include "domain/AttendanceRecord.h" // 引入考勤记录结构。
#include "domain/Person.h" // 引入人员结构。
#include "domain/RecognizedFaceRecord.h" // 引入识别人脸记录结构。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QByteArray> // 引入二进制数组类型。
#include <QJsonArray> // 引入 JSON 数组类型。
#include <QMutex> // 引入互斥锁。
#include <QSqlDatabase> // 引入 SQLite 数据库连接类型。
#include <QString> // 引入 Qt 字符串类型。
#include <QStringList> // 引入字符串列表类型。
#include <QVector> // 引入 Qt 动态数组类型。
class DatabaseManager { // 定义 SQLite 数据库管理器。
public: // 声明公共接口。
    static DatabaseManager& instance(); // 获取单例对象。
    bool open(const QString& databasePath); // 打开或创建数据库。
    bool initialize(); // 初始化表结构和默认数据。
    bool upsertPerson(const Person& person); // 插入或更新人员信息。
    bool removePerson(const QString& personId); // 删除指定人员。
    QVector<Person> loadPersons(const QString& listType = QString()) const; // 查询人员列表。
    bool saveAttendanceRecord(const AttendanceRecord& record); // 保存考勤记录。
    QVector<AttendanceRecord> loadAttendanceRecords(int limit = 200) const; // 查询考勤记录。
    bool saveRecognizedFace(const RecognizedFaceRecord& record); // 保存每一次人脸识别记录。
    QVector<RecognizedFaceRecord> loadRecognizedFaces(int limit = 200) const; // 查询最近识别记录。
    QVector<Person> loadWhitelist() const; // 查询白名单人员。
    QVector<Person> loadBlacklist() const; // 查询黑名单人员。
    bool markUploaded(const QString& recordId, bool uploaded); // 更新记录上传状态。
    bool appendLog(const QString& level, const QString& message); // 追加设备运行日志。
    QStringList loadLogs(int limit = 200) const; // 查询最近日志列表。
    QString lastError() const; // 返回最近一次数据库错误信息。
private: // 声明私有构造。
    DatabaseManager() = default; // 默认构造函数私有化。
    bool ensureConnection() const; // 确保数据库连接有效。
    static QByteArray featureToBlob(const QVector<double>& feature); // 将特征向量转成二进制。
    static QVector<double> blobToFeature(const QByteArray& blob); // 将二进制转回特征向量。
    mutable QMutex mutex_; // 保护数据库操作的互斥锁。
    QSqlDatabase database_; // SQLite 数据库连接。
    QString lastError_; // 保存最近一次错误。
}; // 结束数据库管理器定义。
