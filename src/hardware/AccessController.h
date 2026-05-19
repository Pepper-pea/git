#pragma once // 防止头文件被重复包含。
#include <QObject> // 引入 Qt 对象基类。
#include <QString> // 引入 Qt 字符串类型。
#include <QTimer> // 引入定时器类型。
class AccessController : public QObject { // 定义闸机、指示灯、蜂鸣器联动控制器。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit AccessController(QObject* parent = nullptr); // 构造硬件控制器。
    void setDeviceId(const QString& deviceId); // 设置设备编号。
    void setSimulatedMode(bool enabled); // 设置是否使用模拟硬件模式。
    void setOpenDurationMs(int durationMs); // 设置闸机保持开启时长。
    void setBuzzerDurationMs(int durationMs); // 设置蜂鸣器鸣叫时长。
    bool isGateOpen() const; // 查询闸机是否打开。
    QString gateStateText() const; // 查询闸机状态文本。
public slots: // 声明槽函数接口。
    void openGate(const QString& reason); // 打开闸机并亮绿灯。
    void denyAccess(const QString& reason); // 拒绝通行并触发红灯蜂鸣。
    void closeGate(); // 关闭闸机。
    void emergencyStop(); // 紧急停止并关闭闸机。
    void resetIndicators(); // 重置灯光和蜂鸣器。
signals: // 声明状态信号。
    void gateStateChanged(const QString& state); // 通知闸机状态变化。
    void lightStateChanged(const QString& state); // 通知灯光状态变化。
    void buzzerStateChanged(bool enabled); // 通知蜂鸣器状态变化。
    void statusMessage(const QString& level, const QString& message); // 输出硬件联动日志。
private: // 声明私有工具函数。
    void setLightState(const QString& state); // 设置当前灯光状态。
    void setBuzzerState(bool enabled); // 设置当前蜂鸣器状态。
    QString deviceId_; // 保存设备编号。
    bool simulatedMode_ = true; // 保存是否使用模拟模式。
    bool gateOpen_ = false; // 保存闸机是否打开。
    bool buzzerEnabled_ = false; // 保存蜂鸣器是否启用。
    QString lightState_ = QStringLiteral("idle"); // 保存灯光状态。
    int openDurationMs_ = 3000; // 保存闸机开启持续时间。
    int buzzerDurationMs_ = 800; // 保存蜂鸣器持续时间。
    QTimer closeTimer_; // 保存自动关闸定时器。
    QTimer buzzerTimer_; // 保存蜂鸣器自动停止定时器。
}; // 结束硬件控制器定义。
