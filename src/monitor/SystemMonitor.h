#pragma once // 防止头文件被重复包含。
#include "comm/CloudClient.h" // 引入云端客户端。
#include "hardware/AccessController.h" // 引入硬件控制器。
#include "storage/DatabaseManager.h" // 引入数据库管理器。
#include <QJsonObject> // 引入 JSON 对象类型。
#include <QObject> // 引入 Qt 对象基类。
#include <QTimer> // 引入定时器。
class SystemMonitor : public QObject { // 定义系统监控与维护模块。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit SystemMonitor(QObject* parent = nullptr); // 构造系统监控器。
    void setDeviceId(const QString& deviceId); // 设置设备编号。
    void setLogIntervalMs(int intervalMs); // 设置日志上报间隔。
    void setHealthIntervalMs(int intervalMs); // 设置健康检查间隔。
    void setWatchdogEnabled(bool enabled); // 设置看门狗开关。
    void attachCloudClient(CloudClient* client); // 绑定云端客户端。
    void attachAccessController(AccessController* controller); // 绑定硬件控制器。
public slots: // 声明槽函数接口。
    void start(); // 启动监控。
    void stop(); // 停止监控。
    void reportHeartbeat(); // 上报心跳状态。
    void checkHealth(); // 执行健康检查。
    void refreshFromDatabase(); // 从数据库刷新监控统计。
signals: // 声明监控事件。
    void statusMessage(const QString& level, const QString& message); // 输出监控日志。
    void heartbeatPrepared(const QJsonObject& heartbeat); // 通知心跳数据已准备。
    void watchdogTriggered(const QString& reason); // 通知看门狗触发。
private: // 声明私有工具函数。
    QJsonObject buildHeartbeat() const; // 构建心跳 JSON。
    QString deviceId_; // 保存设备编号。
    bool watchdogEnabled_ = true; // 保存看门狗开关。
    int logIntervalMs_ = 10000; // 保存日志上报间隔。
    int healthIntervalMs_ = 5000; // 保存健康检查间隔。
    int heartbeatCounter_ = 0; // 保存心跳次数。
    QTimer logTimer_; // 保存日志/心跳定时器。
    QTimer healthTimer_; // 保存健康检查定时器。
    CloudClient* cloudClient_ = nullptr; // 保存绑定的云端客户端。
    AccessController* accessController_ = nullptr; // 保存绑定的硬件控制器。
}; // 结束系统监控器定义。
