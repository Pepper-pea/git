#include "monitor/SystemMonitor.h" // 引入系统监控器声明。
#include <QDateTime> // 引入日期时间。
#include <QJsonArray> // 引入 JSON 数组。
#include <QtGlobal> // 引入 Qt 基础工具函数。
SystemMonitor::SystemMonitor(QObject* parent) : QObject(parent) { // 实现构造函数。
    logTimer_.setInterval(logIntervalMs_); // 设置心跳定时器间隔。
    healthTimer_.setInterval(healthIntervalMs_); // 设置健康检查定时器间隔。
    connect(&logTimer_, &QTimer::timeout, this, &SystemMonitor::reportHeartbeat); // 连接心跳定时器。
    connect(&healthTimer_, &QTimer::timeout, this, &SystemMonitor::checkHealth); // 连接健康检查定时器。
} // 结束构造函数。
void SystemMonitor::setDeviceId(const QString& deviceId) { // 实现设备编号设置函数。
    deviceId_ = deviceId; // 保存设备编号。
} // 结束设备编号设置函数。
void SystemMonitor::setLogIntervalMs(int intervalMs) { // 实现心跳间隔设置函数。
    logIntervalMs_ = qMax(1000, intervalMs); // 保存不低于 1 秒的间隔。
    logTimer_.setInterval(logIntervalMs_); // 同步定时器间隔。
} // 结束心跳间隔设置函数。
void SystemMonitor::setHealthIntervalMs(int intervalMs) { // 实现健康检查间隔设置函数。
    healthIntervalMs_ = qMax(1000, intervalMs); // 保存不低于 1 秒的间隔。
    healthTimer_.setInterval(healthIntervalMs_); // 同步定时器间隔。
} // 结束健康检查间隔设置函数。
void SystemMonitor::setWatchdogEnabled(bool enabled) { // 实现看门狗设置函数。
    watchdogEnabled_ = enabled; // 保存看门狗开关。
} // 结束看门狗设置函数。
void SystemMonitor::attachCloudClient(CloudClient* client) { // 实现云端客户端绑定函数。
    cloudClient_ = client; // 保存云端客户端指针。
    if(cloudClient_) { // 判断云端客户端是否存在。
        connect(this, &SystemMonitor::heartbeatPrepared, cloudClient_, [client](const QJsonObject& heartbeat) { client->publishStatus(heartbeat); }); // 将心跳转给云端上传。
    } // 结束云端客户端判断。
} // 结束云端客户端绑定函数。
void SystemMonitor::attachAccessController(AccessController* controller) { // 实现硬件控制器绑定函数。
    accessController_ = controller; // 保存硬件控制器指针。
} // 结束硬件控制器绑定函数。
void SystemMonitor::start() { // 实现监控启动函数。
    logTimer_.start(); // 启动心跳定时器。
    healthTimer_.start(); // 启动健康检查定时器。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("系统监控已启动")); // 输出启动日志。
} // 结束监控启动函数。
void SystemMonitor::stop() { // 实现监控停止函数。
    logTimer_.stop(); // 停止心跳定时器。
    healthTimer_.stop(); // 停止健康检查定时器。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("系统监控已停止")); // 输出停止日志。
} // 结束监控停止函数。
void SystemMonitor::reportHeartbeat() { // 实现心跳上报函数。
    ++heartbeatCounter_; // 增加心跳计数。
    const QJsonObject heartbeat = buildHeartbeat(); // 构建心跳数据。
    emit heartbeatPrepared(heartbeat); // 通知心跳数据。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("已生成第 %1 次心跳").arg(heartbeatCounter_)); // 输出心跳日志。
} // 结束心跳上报函数。
void SystemMonitor::checkHealth() { // 实现健康检查函数。
    const bool cloudReady = cloudClient_ != nullptr && cloudClient_->isOnline(); // 判断云端状态。
    const bool hardwareReady = accessController_ != nullptr; // 判断硬件控制器状态。
    if(watchdogEnabled_ && (!cloudReady || !hardwareReady)) { // 判断看门狗条件。
        emit watchdogTriggered(QStringLiteral("云端或硬件状态异常")); // 输出看门狗触发信号。
    } // 结束看门狗判断。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("健康检查完成：cloud=%1 hardware=%2").arg(cloudReady ? QStringLiteral("ok") : QStringLiteral("down"), hardwareReady ? QStringLiteral("ok") : QStringLiteral("down"))); // 输出健康检查日志。
} // 结束健康检查函数。
void SystemMonitor::refreshFromDatabase() { // 实现数据库刷新函数。
    const QVector<AttendanceRecord> records = DatabaseManager::instance().loadAttendanceRecords(20); // 读取最近记录。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("最近考勤记录 %1 条").arg(records.size())); // 输出刷新日志。
} // 结束数据库刷新函数。
QJsonObject SystemMonitor::buildHeartbeat() const { // 实现心跳构建函数。
    QJsonObject object; // 创建 JSON 容器。
    object.insert(QStringLiteral("deviceId"), deviceId_); // 写入设备编号。
    object.insert(QStringLiteral("heartbeatCount"), heartbeatCounter_); // 写入心跳计数。
    object.insert(QStringLiteral("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)); // 写入当前时间。
    object.insert(QStringLiteral("watchdogEnabled"), watchdogEnabled_); // 写入看门狗状态。
    object.insert(QStringLiteral("cloudOnline"), cloudClient_ && cloudClient_->isOnline()); // 写入云端在线状态。
    object.insert(QStringLiteral("gateState"), accessController_ ? accessController_->gateStateText() : QStringLiteral("unknown")); // 写入闸机状态。
    return object; // 返回心跳对象。
} // 结束心跳构建函数。
