#pragma once // 防止头文件被重复包含。
#include "domain/AttendanceRecord.h" // 引入考勤记录结构。
#include <QJsonObject> // 引入 JSON 对象类型。
#include <QNetworkAccessManager> // 引入网络访问管理器。
#include <QNetworkRequest> // 引入网络请求类型。
#include <QObject> // 引入 Qt 对象基类。
#include <QString> // 引入 Qt 字符串类型。
#include <QUrl> // 引入 URL 类型。
#include <QVector> // 引入动态数组类型。
class QNetworkReply; // 前置声明网络响应对象。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
class QMqttClient; // 前置声明 MQTT 客户端对象。
#endif // 结束 MQTT 判断。
class CloudClient : public QObject { // 定义 MQTT/HTTP 云端通信客户端。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit CloudClient(QObject* parent = nullptr); // 构造云端客户端。
    void setDeviceId(const QString& deviceId); // 设置设备编号。
    void configure(const QJsonObject& config); // 使用配置文件初始化通信参数。
    bool isOnline() const; // 查询当前通信状态。
public slots: // 声明槽函数接口。
    void connectToCloud(); // 建立云端连接。
    void disconnectFromCloud(); // 断开云端连接。
    void publishStatus(const QJsonObject& status); // 上传设备状态。
    void uploadRecord(const AttendanceRecord& record); // 上传单条考勤记录。
    void uploadPendingRecords(const QVector<AttendanceRecord>& records); // 批量上传待同步记录。
    void requestRemoteConfig(); // 请求远程配置。
    void requestOtaManifest(); // 请求 OTA 升级清单。
signals: // 声明通信事件信号。
    void onlineChanged(bool online); // 通知在线状态变化。
    void recordUploadFinished(const QString& recordId, bool success); // 通知记录上传结果。
    void remoteConfigReceived(const QJsonObject& config); // 通知收到远程配置。
    void otaManifestReceived(const QJsonObject& manifest); // 通知收到 OTA 清单。
    void commandReceived(const QString& command); // 通知收到远程指令。
    void logMessage(const QString& level, const QString& message); // 输出通信日志。
private slots: // 声明私有槽函数。
    void handleReplyFinished(QNetworkReply* reply); // 处理 HTTP 响应。
private: // 声明私有工具函数。
    QNetworkRequest buildJsonRequest(const QUrl& url) const; // 创建 JSON 网络请求。
    void postJson(const QUrl& url, const QJsonObject& body, const QString& action, const QString& recordId = QString()); // 发送 JSON POST 请求。
    bool publishMqtt(const QString& topic, const QByteArray& payload); // 发布 MQTT 消息。
    void ensureMqttClient(); // 确保 MQTT 客户端已创建。
    QString deviceId_; // 保存设备编号。
    QString mqttHost_; // 保存 MQTT 主机。
    quint16 mqttPort_ = 1883; // 保存 MQTT 端口。
    QString mqttClientId_; // 保存 MQTT 客户端编号。
    QString mqttUsername_; // 保存 MQTT 用户名。
    QString mqttPassword_; // 保存 MQTT 密码。
    QString statusTopic_; // 保存设备状态主题。
    QString recordTopic_; // 保存考勤记录主题。
    QString commandTopic_; // 保存远程指令主题。
    QUrl statusUrl_; // 保存 HTTP 状态上传地址。
    QUrl recordUrl_; // 保存 HTTP 记录上传地址。
    QUrl configUrl_; // 保存 HTTP 远程配置地址。
    QUrl otaUrl_; // 保存 HTTP OTA 清单地址。
    bool online_ = false; // 保存当前在线状态。
    QNetworkAccessManager network_; // 保存 HTTP 网络管理器。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
    QMqttClient* mqttClient_ = nullptr; // 保存 MQTT 客户端对象。
#endif // 结束 MQTT 判断。
}; // 结束云端客户端定义。
