#include "comm/CloudClient.h" // 引入云端客户端声明。
#include <QDateTime> // 引入日期时间类型。
#include <QJsonDocument> // 引入 JSON 文档类型。
#include <QNetworkReply> // 引入网络响应类型。
#include <QNetworkRequest> // 引入网络请求类型。
#include <QVariant> // 引入动态属性类型。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
#include <QMqttClient> // 引入 MQTT 客户端。
#include <QMqttTopicName> // 引入 MQTT 主题名称。
#endif // 结束 MQTT 判断。
CloudClient::CloudClient(QObject* parent) : QObject(parent) { // 实现构造函数。
    connect(&network_, &QNetworkAccessManager::finished, this, &CloudClient::handleReplyFinished); // 连接 HTTP 完成信号。
} // 结束构造函数。
void CloudClient::setDeviceId(const QString& deviceId) { // 实现设备编号设置函数。
    deviceId_ = deviceId; // 保存设备编号。
} // 结束设备编号设置函数。
void CloudClient::configure(const QJsonObject& config) { // 实现通信配置函数。
    const QJsonObject mqtt = config.value(QStringLiteral("mqtt")).toObject(); // 读取 MQTT 配置。
    const QJsonObject http = config.value(QStringLiteral("http")).toObject(); // 读取 HTTP 配置。
    mqttHost_ = mqtt.value(QStringLiteral("host")).toString(QStringLiteral("127.0.0.1")); // 保存 MQTT 主机。
    mqttPort_ = static_cast<quint16>(mqtt.value(QStringLiteral("port")).toInt(1883)); // 保存 MQTT 端口。
    mqttClientId_ = mqtt.value(QStringLiteral("clientId")).toString(deviceId_ + QStringLiteral("-terminal")); // 保存 MQTT 客户端编号。
    mqttUsername_ = mqtt.value(QStringLiteral("username")).toString(); // 保存 MQTT 用户名。
    mqttPassword_ = mqtt.value(QStringLiteral("password")).toString(); // 保存 MQTT 密码。
    statusTopic_ = mqtt.value(QStringLiteral("statusTopic")).toString(); // 保存状态主题。
    recordTopic_ = mqtt.value(QStringLiteral("recordTopic")).toString(); // 保存记录主题。
    commandTopic_ = mqtt.value(QStringLiteral("commandTopic")).toString(); // 保存指令主题。
    recordUrl_ = QUrl(http.value(QStringLiteral("recordUrl")).toString()); // 保存记录上传地址。
    statusUrl_ = QUrl(http.value(QStringLiteral("statusUrl")).toString()); // 保存状态上传地址。
    configUrl_ = QUrl(http.value(QStringLiteral("configUrl")).toString()); // 保存远程配置地址。
    otaUrl_ = QUrl(http.value(QStringLiteral("otaUrl")).toString()); // 保存 OTA 地址。
    emit logMessage(QStringLiteral("INFO"), QStringLiteral("云端通信参数已加载")); // 输出配置日志。
} // 结束通信配置函数。
bool CloudClient::isOnline() const { // 实现在线状态查询函数。
    return online_; // 返回当前在线状态。
} // 结束在线状态查询函数。
void CloudClient::connectToCloud() { // 实现云端连接函数。
    bool mqttRequested = !mqttHost_.isEmpty(); // 判断是否配置了 MQTT。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
    if(mqttRequested) { // 判断是否需要 MQTT 连接。
        ensureMqttClient(); // 确保 MQTT 客户端存在。
        mqttClient_->setHostname(mqttHost_); // 设置 MQTT 主机。
        mqttClient_->setPort(mqttPort_); // 设置 MQTT 端口。
        mqttClient_->setClientId(mqttClientId_); // 设置 MQTT 客户端编号。
        mqttClient_->setUsername(mqttUsername_); // 设置 MQTT 用户名。
        mqttClient_->setPassword(mqttPassword_); // 设置 MQTT 密码。
        mqttClient_->connectToHost(); // 发起 MQTT 连接。
        emit logMessage(QStringLiteral("INFO"), QStringLiteral("正在连接 MQTT：%1:%2").arg(mqttHost_).arg(mqttPort_)); // 输出连接日志。
    } // 结束 MQTT 连接判断。
#else // 处理没有 MQTT 模块的构建。
    if(mqttRequested) { // 判断配置中是否存在 MQTT。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("当前构建未启用 Qt MQTT，已使用 HTTP 通道作为降级同步")); // 输出降级日志。
    } // 结束 MQTT 配置判断。
#endif // 结束 MQTT 判断。
    online_ = statusUrl_.isValid() || recordUrl_.isValid() || mqttRequested; // 根据配置标记通信通道可用。
    emit onlineChanged(online_); // 通知在线状态变化。
} // 结束云端连接函数。
void CloudClient::disconnectFromCloud() { // 实现断开连接函数。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
    if(mqttClient_) { // 判断 MQTT 客户端是否存在。
        mqttClient_->disconnectFromHost(); // 断开 MQTT 连接。
    } // 结束 MQTT 客户端判断。
#endif // 结束 MQTT 判断。
    online_ = false; // 标记离线状态。
    emit onlineChanged(false); // 通知离线状态。
    emit logMessage(QStringLiteral("INFO"), QStringLiteral("云端通信已断开")); // 输出断开日志。
} // 结束断开连接函数。
void CloudClient::publishStatus(const QJsonObject& status) { // 实现状态发布函数。
    QJsonObject body = status; // 复制状态对象。
    body.insert(QStringLiteral("deviceId"), deviceId_); // 写入设备编号。
    body.insert(QStringLiteral("sentAt"), QDateTime::currentDateTime().toString(Qt::ISODate)); // 写入发送时间。
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact); // 序列化状态 JSON。
    const bool mqttSent = publishMqtt(statusTopic_, payload); // 尝试通过 MQTT 发布状态。
    if(statusUrl_.isValid()) { // 判断 HTTP 状态地址是否有效。
        postJson(statusUrl_, body, QStringLiteral("status")); // 通过 HTTP 上传状态。
    } else if(mqttSent) { // 判断是否仅通过 MQTT 发送成功。
        emit logMessage(QStringLiteral("INFO"), QStringLiteral("设备状态已通过 MQTT 发布")); // 输出 MQTT 成功日志。
    } // 结束状态上传判断。
} // 结束状态发布函数。
void CloudClient::uploadRecord(const AttendanceRecord& record) { // 实现单条记录上传函数。
    QJsonObject body = record.toJson(); // 将记录转换为 JSON。
    body.insert(QStringLiteral("deviceId"), record.deviceId.isEmpty() ? deviceId_ : record.deviceId); // 确保写入设备编号。
    body.insert(QStringLiteral("uploadedAt"), QDateTime::currentDateTime().toString(Qt::ISODate)); // 写入上传时间。
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact); // 序列化记录 JSON。
    const bool mqttSent = publishMqtt(recordTopic_, payload); // 尝试通过 MQTT 发布记录。
    if(recordUrl_.isValid()) { // 判断 HTTP 记录地址是否有效。
        postJson(recordUrl_, body, QStringLiteral("attendance"), record.id); // 通过 HTTP 上传记录。
    } else { // 处理没有 HTTP 地址的情况。
        emit recordUploadFinished(record.id, mqttSent); // 用 MQTT 发布结果作为上传结果。
        emit logMessage(mqttSent ? QStringLiteral("INFO") : QStringLiteral("WARN"), mqttSent ? QStringLiteral("考勤记录已通过 MQTT 发布") : QStringLiteral("考勤记录未上传：未配置 HTTP 地址且 MQTT 不可用")); // 输出上传结果日志。
    } // 结束 HTTP 地址判断。
} // 结束单条记录上传函数。
void CloudClient::uploadPendingRecords(const QVector<AttendanceRecord>& records) { // 实现批量上传函数。
    int count = 0; // 创建待上传计数。
    for(const AttendanceRecord& record : records) { // 遍历传入记录。
        if(record.uploaded) { // 判断记录是否已经上传。
            continue; // 跳过已上传记录。
        } // 结束上传状态判断。
        uploadRecord(record); // 上传当前记录。
        ++count; // 增加上传计数。
    } // 结束记录遍历。
    emit logMessage(QStringLiteral("INFO"), QStringLiteral("已提交 %1 条待上传记录").arg(count)); // 输出批量上传日志。
} // 结束批量上传函数。
void CloudClient::requestRemoteConfig() { // 实现远程配置请求函数。
    if(!configUrl_.isValid()) { // 判断配置地址是否有效。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("远程配置地址未配置")); // 输出地址缺失日志。
        return; // 直接返回。
    } // 结束配置地址判断。
    QNetworkReply* reply = network_.get(buildJsonRequest(configUrl_)); // 发起远程配置 GET 请求。
    reply->setProperty("action", QStringLiteral("config")); // 标记请求动作。
} // 结束远程配置请求函数。
void CloudClient::requestOtaManifest() { // 实现 OTA 清单请求函数。
    if(!otaUrl_.isValid()) { // 判断 OTA 地址是否有效。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("OTA 地址未配置")); // 输出地址缺失日志。
        return; // 直接返回。
    } // 结束 OTA 地址判断。
    QNetworkReply* reply = network_.get(buildJsonRequest(otaUrl_)); // 发起 OTA 清单 GET 请求。
    reply->setProperty("action", QStringLiteral("ota")); // 标记请求动作。
} // 结束 OTA 清单请求函数。
void CloudClient::handleReplyFinished(QNetworkReply* reply) { // 实现 HTTP 响应处理函数。
    const QString action = reply->property("action").toString(); // 读取请求动作。
    const QString recordId = reply->property("recordId").toString(); // 读取记录编号。
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); // 读取 HTTP 状态码。
    const QByteArray body = reply->readAll(); // 读取响应内容。
    const bool success = reply->error() == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300; // 判断请求是否成功。
    if(action == QStringLiteral("attendance")) { // 判断是否为考勤上传响应。
        emit recordUploadFinished(recordId, success); // 通知记录上传结果。
    } // 结束考勤响应判断。
    if(success && action == QStringLiteral("config")) { // 判断是否收到远程配置。
        emit remoteConfigReceived(QJsonDocument::fromJson(body).object()); // 发出远程配置。
    } // 结束远程配置判断。
    if(success && action == QStringLiteral("ota")) { // 判断是否收到 OTA 清单。
        emit otaManifestReceived(QJsonDocument::fromJson(body).object()); // 发出 OTA 清单。
    } // 结束 OTA 清单判断。
    emit logMessage(success ? QStringLiteral("INFO") : QStringLiteral("WARN"), success ? QStringLiteral("HTTP %1 同步成功").arg(action) : QStringLiteral("HTTP %1 同步失败：%2").arg(action, reply->errorString())); // 输出 HTTP 日志。
    reply->deleteLater(); // 延迟释放响应对象。
} // 结束 HTTP 响应处理函数。
QNetworkRequest CloudClient::buildJsonRequest(const QUrl& url) const { // 实现 JSON 请求构造函数。
    QNetworkRequest request(url); // 创建网络请求对象。
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json")); // 设置 JSON 内容类型。
    request.setRawHeader("X-Device-Id", deviceId_.toUtf8()); // 设置设备编号请求头。
    return request; // 返回网络请求对象。
} // 结束 JSON 请求构造函数。
void CloudClient::postJson(const QUrl& url, const QJsonObject& body, const QString& action, const QString& recordId) { // 实现 JSON POST 函数。
    QNetworkReply* reply = network_.post(buildJsonRequest(url), QJsonDocument(body).toJson(QJsonDocument::Compact)); // 发送 JSON 请求。
    reply->setProperty("action", action); // 标记请求动作。
    reply->setProperty("recordId", recordId); // 标记记录编号。
} // 结束 JSON POST 函数。
bool CloudClient::publishMqtt(const QString& topic, const QByteArray& payload) { // 实现 MQTT 发布函数。
    if(topic.isEmpty()) { // 判断主题是否为空。
        return false; // 返回发布失败。
    } // 结束主题判断。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
    if(!mqttClient_ || mqttClient_->state() != QMqttClient::Connected) { // 判断 MQTT 是否已连接。
        return false; // 返回发布失败。
    } // 结束连接状态判断。
    const qint32 packetId = mqttClient_->publish(QMqttTopicName(topic), payload); // 发布 MQTT 消息。
    return packetId >= 0; // 返回发布是否成功排队。
#else // 处理没有 MQTT 模块的构建。
    Q_UNUSED(payload); // 避免未使用参数警告。
    return false; // 返回发布失败。
#endif // 结束 MQTT 判断。
} // 结束 MQTT 发布函数。
void CloudClient::ensureMqttClient() { // 实现 MQTT 客户端创建函数。
#ifdef SMARTSITE_HAS_MQTT // 判断是否启用 Qt MQTT。
    if(mqttClient_) { // 判断客户端是否已经存在。
        return; // 已存在时直接返回。
    } // 结束客户端判断。
    mqttClient_ = new QMqttClient(this); // 创建 MQTT 客户端。
    connect(mqttClient_, &QMqttClient::connected, this, [this]() { online_ = true; emit onlineChanged(true); emit logMessage(QStringLiteral("INFO"), QStringLiteral("MQTT 已连接")); }); // 连接成功回调。
    connect(mqttClient_, &QMqttClient::disconnected, this, [this]() { online_ = false; emit onlineChanged(false); emit logMessage(QStringLiteral("WARN"), QStringLiteral("MQTT 已断开")); }); // 断开连接回调。
#else // 处理没有 MQTT 模块的构建。
    emit logMessage(QStringLiteral("WARN"), QStringLiteral("Qt MQTT 模块不可用")); // 输出模块不可用日志。
#endif // 结束 MQTT 判断。
} // 结束 MQTT 客户端创建函数。
