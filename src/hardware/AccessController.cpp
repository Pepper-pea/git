#include "hardware/AccessController.h" // 引入硬件控制器声明。
#include <QtGlobal> // 引入 Qt 基础工具函数。
AccessController::AccessController(QObject* parent) : QObject(parent) { // 实现构造函数。
    closeTimer_.setSingleShot(true); // 设置关闸定时器为单次触发。
    buzzerTimer_.setSingleShot(true); // 设置蜂鸣器定时器为单次触发。
    connect(&closeTimer_, &QTimer::timeout, this, &AccessController::closeGate); // 连接自动关闸动作。
    connect(&buzzerTimer_, &QTimer::timeout, this, [this]() { setBuzzerState(false); }); // 连接蜂鸣器自动关闭动作。
} // 结束构造函数。
void AccessController::setDeviceId(const QString& deviceId) { // 实现设备编号设置函数。
    deviceId_ = deviceId; // 保存设备编号。
} // 结束设备编号设置函数。
void AccessController::setSimulatedMode(bool enabled) { // 实现模拟模式设置函数。
    simulatedMode_ = enabled; // 保存模拟模式开关。
    emit statusMessage(QStringLiteral("INFO"), enabled ? QStringLiteral("硬件控制处于模拟模式") : QStringLiteral("硬件控制处于真实接线模式")); // 输出模式日志。
} // 结束模拟模式设置函数。
void AccessController::setOpenDurationMs(int durationMs) { // 实现开闸时长设置函数。
    openDurationMs_ = qMax(500, durationMs); // 保存不低于 500 毫秒的开闸时长。
} // 结束开闸时长设置函数。
void AccessController::setBuzzerDurationMs(int durationMs) { // 实现蜂鸣时长设置函数。
    buzzerDurationMs_ = qMax(100, durationMs); // 保存不低于 100 毫秒的蜂鸣时长。
} // 结束蜂鸣时长设置函数。
bool AccessController::isGateOpen() const { // 实现闸机状态查询函数。
    return gateOpen_; // 返回闸机开启状态。
} // 结束闸机状态查询函数。
QString AccessController::gateStateText() const { // 实现闸机状态文本函数。
    return gateOpen_ ? QStringLiteral("闸机开启") : QStringLiteral("闸机关闭"); // 返回适合界面显示的状态文本。
} // 结束闸机状态文本函数。
void AccessController::openGate(const QString& reason) { // 实现开闸函数。
    gateOpen_ = true; // 标记闸机已打开。
    setLightState(QStringLiteral("green")); // 设置绿灯通行状态。
    setBuzzerState(false); // 确保蜂鸣器关闭。
    closeTimer_.start(openDurationMs_); // 启动自动关闸计时。
    emit gateStateChanged(gateStateText()); // 通知闸机状态变化。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("%1 开闸：%2").arg(deviceId_, reason)); // 输出开闸日志。
} // 结束开闸函数。
void AccessController::denyAccess(const QString& reason) { // 实现拒绝通行函数。
    closeGate(); // 确保闸机处于关闭状态。
    setLightState(QStringLiteral("red")); // 设置红灯拒绝状态。
    setBuzzerState(true); // 启动蜂鸣器。
    buzzerTimer_.start(buzzerDurationMs_); // 启动蜂鸣器自动停止计时。
    emit statusMessage(QStringLiteral("WARN"), QStringLiteral("%1 拒绝通行：%2").arg(deviceId_, reason)); // 输出拒绝日志。
} // 结束拒绝通行函数。
void AccessController::closeGate() { // 实现关闸函数。
    if(!gateOpen_) { // 判断闸机是否已经关闭。
        return; // 已关闭时直接返回。
    } // 结束关闭状态判断。
    gateOpen_ = false; // 标记闸机已关闭。
    setLightState(QStringLiteral("idle")); // 恢复默认灯光状态。
    emit gateStateChanged(gateStateText()); // 通知闸机状态变化。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("%1 闸机已关闭").arg(deviceId_)); // 输出关闸日志。
} // 结束关闸函数。
void AccessController::emergencyStop() { // 实现紧急停止函数。
    closeTimer_.stop(); // 停止自动关闸计时器。
    buzzerTimer_.stop(); // 停止蜂鸣计时器。
    gateOpen_ = false; // 强制关闭闸机。
    setLightState(QStringLiteral("red")); // 设置红灯告警状态。
    setBuzzerState(true); // 启动蜂鸣器提醒。
    emit gateStateChanged(gateStateText()); // 通知闸机状态变化。
    emit statusMessage(QStringLiteral("ERROR"), QStringLiteral("%1 已触发紧急停止").arg(deviceId_)); // 输出紧急停止日志。
} // 结束紧急停止函数。
void AccessController::resetIndicators() { // 实现指示器重置函数。
    setLightState(QStringLiteral("idle")); // 恢复待机灯光。
    setBuzzerState(false); // 关闭蜂鸣器。
    emit statusMessage(QStringLiteral("INFO"), QStringLiteral("%1 声光状态已复位").arg(deviceId_)); // 输出复位日志。
} // 结束指示器重置函数。
void AccessController::setLightState(const QString& state) { // 实现灯光状态设置函数。
    if(lightState_ == state) { // 判断灯光状态是否未变化。
        return; // 未变化时直接返回。
    } // 结束状态判断。
    lightState_ = state; // 保存新的灯光状态。
    Q_UNUSED(simulatedMode_); // 保留真实 GPIO 或串口输出扩展点。
    emit lightStateChanged(lightState_); // 通知灯光状态变化。
} // 结束灯光状态设置函数。
void AccessController::setBuzzerState(bool enabled) { // 实现蜂鸣器状态设置函数。
    if(buzzerEnabled_ == enabled) { // 判断蜂鸣器状态是否未变化。
        return; // 未变化时直接返回。
    } // 结束状态判断。
    buzzerEnabled_ = enabled; // 保存蜂鸣器状态。
    Q_UNUSED(simulatedMode_); // 保留真实蜂鸣器输出扩展点。
    emit buzzerStateChanged(buzzerEnabled_); // 通知蜂鸣器状态变化。
} // 结束蜂鸣器状态设置函数。
