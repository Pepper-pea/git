#pragma once // 防止头文件被重复包含。
#include <QDateTime> // 引入 Qt 日期时间类型。
#include <QJsonObject> // 引入 JSON 对象类型。
#include <QMetaType> // 引入 Qt 元类型声明支持。
#include <QString> // 引入 Qt 字符串类型。
struct AttendanceRecord { // 定义考勤记录结构。
    QString id; // 保存记录唯一编号。
    QString personId; // 保存人员编号。
    QString personName; // 保存人员姓名。
    QString team; // 保存人员班组。
    QString direction; // 保存通行方向。
    QString result; // 保存考勤结果。
    double score = 0.0; // 保存识别得分。
    QString deviceId; // 保存设备编号。
    QString snapshotPath; // 保存现场抓拍路径。
    bool uploaded = false; // 保存是否已上传云端。
    QDateTime createdAt; // 保存记录创建时间。
    QJsonObject toJson() const { // 将考勤记录转换成 JSON。
        QJsonObject object; // 创建 JSON 对象容器。
        object.insert(QStringLiteral("id"), id); // 写入记录编号。
        object.insert(QStringLiteral("personId"), personId); // 写入人员编号。
        object.insert(QStringLiteral("personName"), personName); // 写入人员姓名。
        object.insert(QStringLiteral("team"), team); // 写入班组名称。
        object.insert(QStringLiteral("direction"), direction); // 写入通行方向。
        object.insert(QStringLiteral("result"), result); // 写入考勤结果。
        object.insert(QStringLiteral("score"), score); // 写入识别得分。
        object.insert(QStringLiteral("deviceId"), deviceId); // 写入设备编号。
        object.insert(QStringLiteral("snapshotPath"), snapshotPath); // 写入抓拍路径。
        object.insert(QStringLiteral("uploaded"), uploaded); // 写入上传状态。
        object.insert(QStringLiteral("createdAt"), createdAt.toString(Qt::ISODate)); // 写入创建时间。
        return object; // 返回 JSON 对象。
    } // 结束 JSON 转换函数。
}; // 结束考勤记录结构定义。
Q_DECLARE_METATYPE(AttendanceRecord) // 将考勤记录结构声明为 Qt 元类型。
