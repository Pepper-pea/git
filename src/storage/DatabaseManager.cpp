#include "storage/DatabaseManager.h" // 引入数据库管理器声明。
#include <QDateTime> // 引入日期时间。
#include <QDir> // 引入目录处理。
#include <QFileInfo> // 引入文件信息。
#include <QMutexLocker> // 引入互斥锁封装。
#include <QSqlError> // 引入 SQL 错误类型。
#include <QSqlQuery> // 引入 SQL 查询类。
#include <QUuid> // 引入唯一编号生成。
#include <QVariant> // 引入 QVariant。
#include <cstring> // 引入内存拷贝函数。
DatabaseManager& DatabaseManager::instance() { // 实现单例获取函数。
    static DatabaseManager manager; // 创建静态单例对象。
    return manager; // 返回单例引用。
} // 结束单例获取函数。
bool DatabaseManager::open(const QString& databasePath) { // 实现数据库打开函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库连接创建。
    const QString connectionName = QStringLiteral("smart_site_db"); // 定义连接名称。
    if(QSqlDatabase::contains(connectionName)) { // 判断当前连接是否已存在。
        database_ = QSqlDatabase::database(connectionName); // 复用已有连接。
    } else { // 处理首次创建连接。
        database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName); // 创建 SQLite 数据库连接。
    } // 结束连接判断。
    QDir().mkpath(QFileInfo(databasePath).absolutePath()); // 创建数据库所在目录。
    database_.setDatabaseName(databasePath); // 设置数据库文件路径。
    if(!database_.open()) { // 尝试打开数据库。
        lastError_ = database_.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束数据库打开判断。
    return true; // 返回成功。
} // 结束数据库打开函数。
bool DatabaseManager::initialize() { // 实现数据库表初始化函数。
    if(!database_.isOpen() && !database_.open()) { // 判断数据库是否已打开。
        lastError_ = database_.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束数据库打开判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    const char* schema[] = { // 定义建表 SQL 语句集合。
        "CREATE TABLE IF NOT EXISTS persons (id TEXT PRIMARY KEY, name TEXT, team TEXT, role TEXT, card_no TEXT, list_type TEXT, access_level INTEGER, enabled INTEGER, feature BLOB)", // 创建人员表。
        "CREATE TABLE IF NOT EXISTS attendance_records (id TEXT PRIMARY KEY, person_id TEXT, person_name TEXT, team TEXT, direction TEXT, result TEXT, score REAL, device_id TEXT, snapshot_path TEXT, uploaded INTEGER, created_at TEXT)", // 创建考勤表。
        "CREATE TABLE IF NOT EXISTS recognized_faces (id TEXT PRIMARY KEY, frame_id INTEGER, person_id TEXT, person_name TEXT, team TEXT, status TEXT, access_allowed INTEGER, decision TEXT, message TEXT, score REAL, cosine REAL, euclidean REAL, quality REAL, blink_detected INTEGER, face_x INTEGER, face_y INTEGER, face_width INTEGER, face_height INTEGER, snapshot_path TEXT, device_id TEXT, created_at TEXT)", // 创建识别人脸记录表。
        "CREATE INDEX IF NOT EXISTS idx_recognized_faces_created_at ON recognized_faces(created_at)", // 创建识别时间索引。
        "CREATE INDEX IF NOT EXISTS idx_recognized_faces_person_id ON recognized_faces(person_id)", // 创建识别人员索引。
        "CREATE TABLE IF NOT EXISTS device_logs (id INTEGER PRIMARY KEY AUTOINCREMENT, level TEXT, message TEXT, created_at TEXT)" // 创建设备日志表。
    }; // 结束 SQL 数组定义。
    for(const char* statement : schema) { // 遍历每一条建表语句。
        if(!query.exec(QString::fromUtf8(statement))) { // 执行建表语句。
            lastError_ = query.lastError().text(); // 记录错误信息。
            return false; // 返回失败。
        } // 结束 SQL 执行判断。
    } // 结束建表遍历。
    return true; // 返回成功。
} // 结束初始化函数。
bool DatabaseManager::upsertPerson(const Person& person) { // 实现人员写入函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    if(!database_.isOpen() && !database_.open()) { // 判断数据库是否可用。
        lastError_ = database_.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("INSERT INTO persons(id, name, team, role, card_no, list_type, access_level, enabled, feature) VALUES(:id, :name, :team, :role, :card_no, :list_type, :access_level, :enabled, :feature) ON CONFLICT(id) DO UPDATE SET name=excluded.name, team=excluded.team, role=excluded.role, card_no=excluded.card_no, list_type=excluded.list_type, access_level=excluded.access_level, enabled=excluded.enabled, feature=excluded.feature")); // 准备人员插入或更新语句。
    query.bindValue(QStringLiteral(":id"), person.id); // 绑定人员编号。
    query.bindValue(QStringLiteral(":name"), person.name); // 绑定人员姓名。
    query.bindValue(QStringLiteral(":team"), person.team); // 绑定班组。
    query.bindValue(QStringLiteral(":role"), person.role); // 绑定角色。
    query.bindValue(QStringLiteral(":card_no"), person.cardNo); // 绑定卡号。
    query.bindValue(QStringLiteral(":list_type"), person.listType); // 绑定名单类型。
    query.bindValue(QStringLiteral(":access_level"), person.accessLevel); // 绑定通行等级。
    query.bindValue(QStringLiteral(":enabled"), person.enabled ? 1 : 0); // 绑定启用状态。
    query.bindValue(QStringLiteral(":feature"), featureToBlob(person.feature)); // 绑定特征二进制。
    if(!query.exec()) { // 执行写入语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束人员写入函数。
bool DatabaseManager::removePerson(const QString& personId) { // 实现人员删除函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("DELETE FROM persons WHERE id = :id")); // 准备删除语句。
    query.bindValue(QStringLiteral(":id"), personId); // 绑定人员编号。
    if(!query.exec()) { // 执行删除语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束人员删除函数。
QVector<Person> DatabaseManager::loadPersons(const QString& listType) const { // 实现人员查询函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库读取。
    QVector<Person> persons; // 创建人员数组。
    if(!database_.isOpen()) { // 判断数据库是否已打开。
        return persons; // 返回空数组。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    QString sql = QStringLiteral("SELECT id, name, team, role, card_no, list_type, access_level, enabled, feature FROM persons"); // 定义基础查询语句。
    if(!listType.isEmpty()) { // 判断是否指定名单类型。
        sql += QStringLiteral(" WHERE list_type = :list_type"); // 添加名单过滤条件。
    } // 结束条件判断。
    sql += QStringLiteral(" ORDER BY name ASC"); // 添加排序条件。
    query.prepare(sql); // 准备查询语句。
    if(!listType.isEmpty()) { // 判断是否需要绑定名单类型参数。
        query.bindValue(QStringLiteral(":list_type"), listType); // 绑定名单类型。
    } // 结束绑定判断。
    if(!query.exec()) { // 执行查询。
        return persons; // 返回空数组。
    } // 结束 SQL 执行判断。
    while(query.next()) { // 遍历查询结果。
        Person person; // 创建人员对象。
        person.id = query.value(0).toString(); // 读取人员编号。
        person.name = query.value(1).toString(); // 读取人员姓名。
        person.team = query.value(2).toString(); // 读取班组。
        person.role = query.value(3).toString(); // 读取角色。
        person.cardNo = query.value(4).toString(); // 读取卡号。
        person.listType = query.value(5).toString(); // 读取名单类型。
        person.accessLevel = query.value(6).toInt(); // 读取通行等级。
        person.enabled = query.value(7).toInt() != 0; // 读取启用状态。
        person.feature = blobToFeature(query.value(8).toByteArray()); // 读取特征向量。
        persons.append(person); // 追加到结果数组。
    } // 结束结果遍历。
    return persons; // 返回人员数组。
} // 结束人员查询函数。
bool DatabaseManager::saveAttendanceRecord(const AttendanceRecord& record) { // 实现考勤记录保存函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("INSERT INTO attendance_records(id, person_id, person_name, team, direction, result, score, device_id, snapshot_path, uploaded, created_at) VALUES(:id, :person_id, :person_name, :team, :direction, :result, :score, :device_id, :snapshot_path, :uploaded, :created_at) ON CONFLICT(id) DO UPDATE SET person_id=excluded.person_id, person_name=excluded.person_name, team=excluded.team, direction=excluded.direction, result=excluded.result, score=excluded.score, device_id=excluded.device_id, snapshot_path=excluded.snapshot_path, uploaded=excluded.uploaded, created_at=excluded.created_at")); // 准备记录插入或更新语句。
    query.bindValue(QStringLiteral(":id"), record.id); // 绑定记录编号。
    query.bindValue(QStringLiteral(":person_id"), record.personId); // 绑定人员编号。
    query.bindValue(QStringLiteral(":person_name"), record.personName); // 绑定人员姓名。
    query.bindValue(QStringLiteral(":team"), record.team); // 绑定班组。
    query.bindValue(QStringLiteral(":direction"), record.direction); // 绑定通行方向。
    query.bindValue(QStringLiteral(":result"), record.result); // 绑定识别结果。
    query.bindValue(QStringLiteral(":score"), record.score); // 绑定识别得分。
    query.bindValue(QStringLiteral(":device_id"), record.deviceId); // 绑定设备编号。
    query.bindValue(QStringLiteral(":snapshot_path"), record.snapshotPath); // 绑定抓拍路径。
    query.bindValue(QStringLiteral(":uploaded"), record.uploaded ? 1 : 0); // 绑定上传状态。
    query.bindValue(QStringLiteral(":created_at"), record.createdAt.isValid() ? record.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate)); // 绑定创建时间。
    if(!query.exec()) { // 执行写入语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束考勤记录保存函数。
QVector<AttendanceRecord> DatabaseManager::loadAttendanceRecords(int limit) const { // 实现考勤记录查询函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库读取。
    QVector<AttendanceRecord> records; // 创建记录数组。
    if(!database_.isOpen()) { // 判断数据库是否已打开。
        return records; // 返回空数组。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("SELECT id, person_id, person_name, team, direction, result, score, device_id, snapshot_path, uploaded, created_at FROM attendance_records ORDER BY created_at DESC LIMIT :limit")); // 准备查询语句。
    query.bindValue(QStringLiteral(":limit"), limit); // 绑定查询数量。
    if(!query.exec()) { // 执行查询。
        return records; // 返回空数组。
    } // 结束 SQL 执行判断。
    while(query.next()) { // 遍历查询结果。
        AttendanceRecord record; // 创建考勤记录对象。
        record.id = query.value(0).toString(); // 读取记录编号。
        record.personId = query.value(1).toString(); // 读取人员编号。
        record.personName = query.value(2).toString(); // 读取人员姓名。
        record.team = query.value(3).toString(); // 读取班组。
        record.direction = query.value(4).toString(); // 读取通行方向。
        record.result = query.value(5).toString(); // 读取结果。
        record.score = query.value(6).toDouble(); // 读取分数。
        record.deviceId = query.value(7).toString(); // 读取设备编号。
        record.snapshotPath = query.value(8).toString(); // 读取抓拍路径。
        record.uploaded = query.value(9).toInt() != 0; // 读取上传状态。
        record.createdAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODate); // 读取创建时间。
        records.append(record); // 追加到结果数组。
    } // 结束结果遍历。
    return records; // 返回考勤记录数组。
} // 结束考勤记录查询函数。
bool DatabaseManager::saveRecognizedFace(const RecognizedFaceRecord& record) { // 实现识别人脸记录保存函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    if(!database_.isOpen() && !database_.open()) { // 判断数据库是否可用。
        lastError_ = database_.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("INSERT INTO recognized_faces(id, frame_id, person_id, person_name, team, status, access_allowed, decision, message, score, cosine, euclidean, quality, blink_detected, face_x, face_y, face_width, face_height, snapshot_path, device_id, created_at) VALUES(:id, :frame_id, :person_id, :person_name, :team, :status, :access_allowed, :decision, :message, :score, :cosine, :euclidean, :quality, :blink_detected, :face_x, :face_y, :face_width, :face_height, :snapshot_path, :device_id, :created_at) ON CONFLICT(id) DO UPDATE SET frame_id=excluded.frame_id, person_id=excluded.person_id, person_name=excluded.person_name, team=excluded.team, status=excluded.status, access_allowed=excluded.access_allowed, decision=excluded.decision, message=excluded.message, score=excluded.score, cosine=excluded.cosine, euclidean=excluded.euclidean, quality=excluded.quality, blink_detected=excluded.blink_detected, face_x=excluded.face_x, face_y=excluded.face_y, face_width=excluded.face_width, face_height=excluded.face_height, snapshot_path=excluded.snapshot_path, device_id=excluded.device_id, created_at=excluded.created_at")); // 准备识别记录插入或更新语句。
    const QString id = record.id.isEmpty() ? QStringLiteral("FACE-%1-%2").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz")), QUuid::createUuid().toString(QUuid::Id128).left(8)) : record.id; // 生成兜底编号。
    query.bindValue(QStringLiteral(":id"), id); // 绑定识别记录编号。
    query.bindValue(QStringLiteral(":frame_id"), record.frameId); // 绑定帧序号。
    query.bindValue(QStringLiteral(":person_id"), record.personId); // 绑定人员编号。
    query.bindValue(QStringLiteral(":person_name"), record.personName); // 绑定人员姓名。
    query.bindValue(QStringLiteral(":team"), record.team); // 绑定班组。
    query.bindValue(QStringLiteral(":status"), record.status); // 绑定识别状态。
    query.bindValue(QStringLiteral(":access_allowed"), record.accessAllowed ? 1 : 0); // 绑定通行结果。
    query.bindValue(QStringLiteral(":decision"), record.decision); // 绑定通行判定。
    query.bindValue(QStringLiteral(":message"), record.message); // 绑定识别说明。
    query.bindValue(QStringLiteral(":score"), record.score); // 绑定综合得分。
    query.bindValue(QStringLiteral(":cosine"), record.cosine); // 绑定余弦相似度。
    query.bindValue(QStringLiteral(":euclidean"), record.euclidean); // 绑定欧氏距离。
    query.bindValue(QStringLiteral(":quality"), record.quality); // 绑定图像质量。
    query.bindValue(QStringLiteral(":blink_detected"), record.blinkDetected ? 1 : 0); // 绑定活体结果。
    query.bindValue(QStringLiteral(":face_x"), record.faceRect.x()); // 绑定人脸横坐标。
    query.bindValue(QStringLiteral(":face_y"), record.faceRect.y()); // 绑定人脸纵坐标。
    query.bindValue(QStringLiteral(":face_width"), record.faceRect.width()); // 绑定人脸宽度。
    query.bindValue(QStringLiteral(":face_height"), record.faceRect.height()); // 绑定人脸高度。
    query.bindValue(QStringLiteral(":snapshot_path"), record.snapshotPath); // 绑定抓拍图片路径。
    query.bindValue(QStringLiteral(":device_id"), record.deviceId); // 绑定设备编号。
    query.bindValue(QStringLiteral(":created_at"), record.createdAt.isValid() ? record.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate)); // 绑定创建时间。
    if(!query.exec()) { // 执行写入语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束识别人脸记录保存函数。
QVector<RecognizedFaceRecord> DatabaseManager::loadRecognizedFaces(int limit) const { // 实现识别人脸记录查询函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库读取。
    QVector<RecognizedFaceRecord> records; // 创建识别记录数组。
    if(!database_.isOpen()) { // 判断数据库是否已打开。
        return records; // 返回空数组。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("SELECT id, frame_id, person_id, person_name, team, status, access_allowed, decision, message, score, cosine, euclidean, quality, blink_detected, face_x, face_y, face_width, face_height, snapshot_path, device_id, created_at FROM recognized_faces ORDER BY created_at DESC LIMIT :limit")); // 准备查询语句。
    query.bindValue(QStringLiteral(":limit"), limit); // 绑定查询数量。
    if(!query.exec()) { // 执行查询。
        return records; // 返回空数组。
    } // 结束 SQL 执行判断。
    while(query.next()) { // 遍历查询结果。
        RecognizedFaceRecord record; // 创建识别记录对象。
        record.id = query.value(0).toString(); // 读取记录编号。
        record.frameId = query.value(1).toLongLong(); // 读取帧序号。
        record.personId = query.value(2).toString(); // 读取人员编号。
        record.personName = query.value(3).toString(); // 读取人员姓名。
        record.team = query.value(4).toString(); // 读取班组。
        record.status = query.value(5).toString(); // 读取识别状态。
        record.accessAllowed = query.value(6).toInt() != 0; // 读取通行结果。
        record.decision = query.value(7).toString(); // 读取通行判定。
        record.message = query.value(8).toString(); // 读取识别说明。
        record.score = query.value(9).toDouble(); // 读取综合得分。
        record.cosine = query.value(10).toDouble(); // 读取余弦相似度。
        record.euclidean = query.value(11).toDouble(); // 读取欧氏距离。
        record.quality = query.value(12).toDouble(); // 读取图像质量。
        record.blinkDetected = query.value(13).toInt() != 0; // 读取活体结果。
        record.faceRect = QRect(query.value(14).toInt(), query.value(15).toInt(), query.value(16).toInt(), query.value(17).toInt()); // 读取人脸矩形。
        record.snapshotPath = query.value(18).toString(); // 读取抓拍路径。
        record.deviceId = query.value(19).toString(); // 读取设备编号。
        record.createdAt = QDateTime::fromString(query.value(20).toString(), Qt::ISODate); // 读取创建时间。
        records.append(record); // 追加到结果数组。
    } // 结束结果遍历。
    return records; // 返回识别记录数组。
} // 结束识别人脸记录查询函数。
QVector<Person> DatabaseManager::loadWhitelist() const { // 实现白名单查询函数。
    return loadPersons(QStringLiteral("white")); // 返回白名单人员。
} // 结束白名单查询函数。
QVector<Person> DatabaseManager::loadBlacklist() const { // 实现黑名单查询函数。
    return loadPersons(QStringLiteral("black")); // 返回黑名单人员。
} // 结束黑名单查询函数。
bool DatabaseManager::markUploaded(const QString& recordId, bool uploaded) { // 实现上传状态更新函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("UPDATE attendance_records SET uploaded = :uploaded WHERE id = :id")); // 准备更新语句。
    query.bindValue(QStringLiteral(":uploaded"), uploaded ? 1 : 0); // 绑定上传状态。
    query.bindValue(QStringLiteral(":id"), recordId); // 绑定记录编号。
    if(!query.exec()) { // 执行更新语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束上传状态更新函数。
bool DatabaseManager::appendLog(const QString& level, const QString& message) { // 实现日志追加函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库写入。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("INSERT INTO device_logs(level, message, created_at) VALUES(:level, :message, :created_at)")); // 准备日志插入语句。
    query.bindValue(QStringLiteral(":level"), level); // 绑定日志级别。
    query.bindValue(QStringLiteral(":message"), message); // 绑定日志内容。
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTime().toString(Qt::ISODate)); // 绑定日志时间。
    if(!query.exec()) { // 执行插入语句。
        lastError_ = query.lastError().text(); // 记录错误信息。
        return false; // 返回失败。
    } // 结束 SQL 执行判断。
    return true; // 返回成功。
} // 结束日志追加函数。
QStringList DatabaseManager::loadLogs(int limit) const { // 实现日志查询函数。
    QMutexLocker locker(&mutex_); // 上锁保护数据库读取。
    QStringList logs; // 创建日志字符串列表。
    if(!database_.isOpen()) { // 判断数据库是否已打开。
        return logs; // 返回空列表。
    } // 结束数据库判断。
    QSqlQuery query(database_); // 创建 SQL 查询对象。
    query.prepare(QStringLiteral("SELECT level, message, created_at FROM device_logs ORDER BY id DESC LIMIT :limit")); // 准备日志查询语句。
    query.bindValue(QStringLiteral(":limit"), limit); // 绑定查询条数。
    if(!query.exec()) { // 执行查询语句。
        return logs; // 返回空列表。
    } // 结束 SQL 执行判断。
    while(query.next()) { // 遍历查询结果。
        logs.append(QStringLiteral("[%1] %2 %3").arg(query.value(2).toString(), query.value(0).toString(), query.value(1).toString())); // 拼接日志文本。
    } // 结束结果遍历。
    return logs; // 返回日志列表。
} // 结束日志查询函数。
QString DatabaseManager::lastError() const { // 实现错误信息读取函数。
    return lastError_; // 返回最近一次错误。
} // 结束错误读取函数。
bool DatabaseManager::ensureConnection() const { // 实现连接检查函数。
    return database_.isValid() && database_.isOpen(); // 返回数据库连接是否可用。
} // 结束连接检查函数。
QByteArray DatabaseManager::featureToBlob(const QVector<double>& feature) { // 实现特征向量转二进制函数。
    QByteArray blob; // 创建二进制容器。
    blob.resize(static_cast<int>(feature.size() * static_cast<int>(sizeof(double)))); // 预留二进制大小。
    if(feature.isEmpty()) { // 判断向量是否为空。
        return blob; // 返回空二进制。
    } // 结束空向量判断。
    memcpy(blob.data(), feature.constData(), static_cast<size_t>(blob.size())); // 直接拷贝 double 数据。
    return blob; // 返回二进制结果。
} // 结束特征转二进制函数。
QVector<double> DatabaseManager::blobToFeature(const QByteArray& blob) { // 实现二进制转特征向量函数。
    QVector<double> feature; // 创建特征数组。
    if(blob.isEmpty()) { // 判断二进制是否为空。
        return feature; // 返回空数组。
    } // 结束空判断。
    const int count = blob.size() / static_cast<int>(sizeof(double)); // 计算 double 数量。
    feature.resize(count); // 调整数组大小。
    memcpy(feature.data(), blob.constData(), static_cast<size_t>(count * static_cast<int>(sizeof(double)))); // 拷贝二进制数据。
    return feature; // 返回特征数组。
} // 结束二进制转特征函数。
