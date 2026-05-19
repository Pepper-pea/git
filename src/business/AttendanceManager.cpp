#include "business/AttendanceManager.h" // 引入考勤管理器声明。
#include <QUuid> // 引入唯一编号生成。
AttendanceManager::AttendanceManager(QObject* parent) : QObject(parent) { // 实现构造函数。
} // 结束构造函数。
void AttendanceManager::setDeviceId(const QString& deviceId) { // 实现设备编号设置函数。
    deviceId_ = deviceId; // 保存设备编号。
} // 结束设备编号设置函数。
void AttendanceManager::setDuplicateSeconds(int seconds) { // 实现重复窗口设置函数。
    duplicateSeconds_ = qMax(1, seconds); // 设置最小为 1 秒。
} // 结束重复窗口设置函数。
void AttendanceManager::setScoreThresholds(double cosine, double euclidean) { // 实现阈值设置函数。
    cosineThreshold_ = cosine; // 保存余弦阈值。
    euclideanThreshold_ = euclidean; // 保存欧氏距离阈值。
} // 结束阈值设置函数。
void AttendanceManager::setRequiredLiveness(bool enabled) { // 实现活体要求设置函数。
    requireLiveness_ = enabled; // 保存活体开关。
} // 结束活体要求设置函数。
AttendanceRecord AttendanceManager::createRecord(const RecognitionResult& result, const QString& direction, const QString& snapshotPath) { // 实现考勤记录生成函数。
    AttendanceRecord record; // 创建考勤记录对象。
    record.id = buildRecordId(); // 生成记录编号。
    record.personId = result.person.id; // 写入人员编号。
    record.personName = result.person.name; // 写入人员姓名。
    record.team = result.person.team; // 写入班组。
    record.direction = direction; // 写入通行方向。
    record.result = buildResultText(result); // 写入结果文本。
    record.score = result.score; // 写入综合得分。
    record.deviceId = deviceId_; // 写入设备编号。
    record.snapshotPath = snapshotPath; // 写入抓拍路径。
    record.uploaded = false; // 设置默认未上传。
    record.createdAt = QDateTime::currentDateTime(); // 设置创建时间。
    emit recordCreated(record); // 发出记录创建信号。
    emit logMessage(QStringLiteral("INFO"), QStringLiteral("生成考勤记录 %1 %2").arg(record.personName, record.result)); // 发出日志消息。
    return record; // 返回考勤记录。
} // 结束考勤记录生成函数。
bool AttendanceManager::shouldAllow(const RecognitionResult& result) const { // 实现通行判断函数。
    if(result.status != RecognitionStatus::Accepted) { // 判断识别状态是否通过。
        return false; // 返回不允许通行。
    } // 结束状态判断。
    if(requireLiveness_ && !result.detection.blinkDetected) { // 判断是否强制活体而且未通过。
        return false; // 返回不允许通行。
    } // 结束活体判断。
    if(result.person.listType == QStringLiteral("black")) { // 判断是否黑名单。
        return false; // 返回不允许通行。
    } // 结束黑名单判断。
    if(isDuplicate(result.person.id, QStringLiteral("in"))) { // 判断是否重复打卡。
        return false; // 返回不允许通行。
    } // 结束重复判断。
    return true; // 返回允许通行。
} // 结束通行判断函数。
QString AttendanceManager::explainDecision(const RecognitionResult& result) const { // 实现判定说明函数。
    if(result.status == RecognitionStatus::Spoof) { // 判断是否活体失败。
        return QStringLiteral("活体检测未通过"); // 返回说明文本。
    } // 结束活体判断。
    if(result.status == RecognitionStatus::LowQuality) { // 判断是否图像质量不足。
        return QStringLiteral("图像质量不足"); // 返回说明文本。
    } // 结束质量判断。
    if(result.status == RecognitionStatus::Stranger) { // 判断是否陌生人。
        return QStringLiteral("未匹配到库内人员"); // 返回说明文本。
    } // 结束陌生人判断。
    if(result.person.listType == QStringLiteral("black")) { // 判断是否黑名单。
        return QStringLiteral("黑名单禁止通行"); // 返回说明文本。
    } // 结束黑名单判断。
    if(isDuplicate(result.person.id, QStringLiteral("in"))) { // 判断是否重复打卡。
        return QStringLiteral("重复打卡限制中"); // 返回说明文本。
    } // 结束重复判断。
    return QStringLiteral("允许通行"); // 返回默认说明。
} // 结束判定说明函数。
bool AttendanceManager::isDuplicate(const QString& personId, const QString& direction) const { // 实现重复打卡判断函数。
    Q_UNUSED(direction); // 当前版本未区分方向，仅预留接口。
    const QDateTime lastTime = lastAttendance_.value(personId); // 读取最近打卡时间。
    if(!lastTime.isValid()) { // 判断是否没有历史记录。
        return false; // 返回不是重复打卡。
    } // 结束历史记录判断。
    return lastTime.secsTo(QDateTime::currentDateTime()) < duplicateSeconds_; // 判断是否在重复窗口内。
} // 结束重复打卡判断函数。
void AttendanceManager::rememberAttendance(const AttendanceRecord& record) { // 实现最近考勤记忆函数。
    lastAttendance_.insert(record.personId, record.createdAt); // 记录最近打卡时间。
} // 结束记忆函数。
void AttendanceManager::clearCache() { // 实现缓存清理函数。
    lastAttendance_.clear(); // 清空最近考勤映射。
} // 结束缓存清理函数。
QString AttendanceManager::buildResultText(const RecognitionResult& result) const { // 实现结果文本构建函数。
    switch(result.status) { // 按识别状态分支。
    case RecognitionStatus::Accepted: return QStringLiteral("通过"); // 通过状态。
    case RecognitionStatus::Denied: return QStringLiteral("拒绝"); // 拒绝状态。
    case RecognitionStatus::Stranger: return QStringLiteral("陌生人"); // 陌生人状态。
    case RecognitionStatus::Spoof: return QStringLiteral("活体失败"); // 活体失败状态。
    case RecognitionStatus::LowQuality: return QStringLiteral("图像质量不足"); // 质量不足状态。
    } // 结束分支。
    return QStringLiteral("未知"); // 返回兜底文本。
} // 结束结果文本构建函数。
QString AttendanceManager::buildRecordId() const { // 实现记录编号生成函数。
    return QStringLiteral("REC-%1-%2").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz")), QUuid::createUuid().toString(QUuid::Id128).left(8)); // 生成带时间和随机段的编号。
} // 结束记录编号生成函数。
QString AttendanceManager::buildSnapshotFileName(const RecognitionResult& result) const { // 实现抓拍文件名生成函数。
    Q_UNUSED(result); // 当前版本文件名仅依赖时间和设备编号。
    return QStringLiteral("%1_%2.jpg").arg(deviceId_, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz"))); // 生成抓拍文件名。
} // 结束抓拍文件名生成函数。
