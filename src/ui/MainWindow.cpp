#include "ui/MainWindow.h" // 引入主窗口声明。
#include <QApplication> // 引入应用程序对象。
#include <QDateTime> // 引入日期时间。
#include <QAbstractItemView> // 引入表格视图基类。
#include <QColor> // 引入颜色类型。
#include <QCoreApplication> // 引入核心应用对象。
#include <QDir> // 引入目录类型。
#include <QFile> // 引入文件处理。
#include <QHeaderView> // 引入表头视图。
#include <QImage> // 引入图像类型。
#include <QJsonDocument> // 引入 JSON 文档。
#include <QJsonObject> // 引入 JSON 对象。
#include <QLabel> // 引入标签。
#include <QMessageBox> // 引入消息框。
#include <QPainter> // 引入绘图类型。
#include <QPen> // 引入画笔类型。
#include <QPixmap> // 引入位图显示。
#include <QRect> // 引入矩形类型。
#include <QScreen> // 引入屏幕信息。
#include <QSplitter> // 引入分割器。
#include <QStatusBar> // 引入状态栏。
#include <QVBoxLayout> // 引入垂直布局。
#include <QHBoxLayout> // 引入水平布局。
#include <QGroupBox> // 引入分组框。
#include <QFileInfo> // 引入文件信息。
#include <QStandardPaths> // 引入标准路径。
#include <QStringList> // 引入字符串列表。
#include <QTableWidgetItem> // 引入表格项。
namespace { // 定义内部辅助命名空间。
QJsonObject readConfigFile(const QString& path) { // 定义配置文件读取函数。
    const QStringList candidates = { // 创建候选路径列表。
        path, // 使用传入路径。
        QStringLiteral("../") + path, // 使用上一级目录路径。
        QCoreApplication::applicationDirPath() + QStringLiteral("/") + path, // 使用程序目录下路径。
        QCoreApplication::applicationDirPath() + QStringLiteral("/../") + path // 使用程序目录上一级路径。
    }; // 结束候选路径列表。
    for(const QString& candidate : candidates) { // 遍历候选路径。
        QFile file(candidate); // 创建文件对象。
        if(!file.open(QIODevice::ReadOnly)) { // 判断是否打开成功。
            continue; // 失败则继续尝试。
        } // 结束打开判断。
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll()); // 解析 JSON。
        if(document.isObject()) { // 判断是否为对象。
            return document.object(); // 返回 JSON 对象。
        } // 结束对象判断。
    } // 结束候选路径遍历。
    return {}; // 返回空配置。
} // 结束配置读取函数。
QTableWidget* createTable(int columns, const QStringList& headers) { // 定义表格创建函数。
    QTableWidget* table = new QTableWidget; // 创建表格控件。
    table->setColumnCount(columns); // 设置列数。
    table->setHorizontalHeaderLabels(headers); // 设置表头。
    table->horizontalHeader()->setStretchLastSection(true); // 让最后一列自适应。
    table->verticalHeader()->setVisible(false); // 隐藏行号。
    table->setSelectionBehavior(QAbstractItemView::SelectRows); // 设置按行选择。
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止直接编辑。
    return table; // 返回表格。
} // 结束表格创建函数。
} // 结束内部辅助命名空间。
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) { // 实现主窗口构造函数。
    qRegisterMetaType<FramePacket>(); // 注册帧数据类型。
    qRegisterMetaType<RecognitionResult>(); // 注册识别结果类型。
    qRegisterMetaType<AttendanceRecord>(); // 注册考勤记录类型。
    buildUi(); // 构建界面。
    loadConfig(); // 读取配置。
    connect(&camera_, &CameraWorker::frameCaptured, this, &MainWindow::onFrameCaptured); // 连接摄像头帧信号。
    connect(&camera_, &CameraWorker::statusMessage, this, &MainWindow::onStatusMessage); // 连接摄像头状态信号。
    connect(&recognizer_, &FaceRecognizer::logMessage, this, &MainWindow::onLogMessage); // 连接识别日志。
    connect(&attendance_, &AttendanceManager::recordCreated, this, &MainWindow::onRecordCreated); // 连接考勤记录信号。
    connect(&attendance_, &AttendanceManager::logMessage, this, &MainWindow::onLogMessage); // 连接考勤日志。
    connect(&accessController_, &AccessController::statusMessage, this, &MainWindow::onLogMessage); // 连接硬件日志。
    connect(&accessController_, &AccessController::gateStateChanged, this, [this](const QString& state) { gateLabel_->setText(state); }); // 连接闸机状态更新。
    connect(&accessController_, &AccessController::lightStateChanged, this, [this](const QString& state) { appendLogRow(QStringLiteral("INFO"), QStringLiteral("灯光状态：%1").arg(state)); }); // 连接灯光状态更新。
    connect(&accessController_, &AccessController::buzzerStateChanged, this, [this](bool enabled) { appendLogRow(QStringLiteral("INFO"), enabled ? QStringLiteral("蜂鸣器已开启") : QStringLiteral("蜂鸣器已关闭")); }); // 连接蜂鸣器状态更新。
    connect(&cloudClient_, &CloudClient::logMessage, this, &MainWindow::onLogMessage); // 连接云端日志。
    connect(&cloudClient_, &CloudClient::onlineChanged, this, &MainWindow::onOnlineChanged); // 连接在线状态变化。
    connect(&cloudClient_, &CloudClient::recordUploadFinished, this, [this](const QString& recordId, bool success) { appendLogRow(success ? QStringLiteral("INFO") : QStringLiteral("WARN"), QStringLiteral("记录 %1 上传%2").arg(recordId, success ? QStringLiteral("成功") : QStringLiteral("失败"))); }); // 连接记录上传结果。
    connect(&cloudClient_, &CloudClient::remoteConfigReceived, this, [this](const QJsonObject& config) { appendLogRow(QStringLiteral("INFO"), QStringLiteral("收到远程配置")); cloudClient_.configure(config); }); // 连接远程配置。
    connect(&cloudClient_, &CloudClient::otaManifestReceived, this, [this](const QJsonObject&) { appendLogRow(QStringLiteral("INFO"), QStringLiteral("收到 OTA 清单")); }); // 连接 OTA 清单。
    connect(&monitor_, &SystemMonitor::statusMessage, this, &MainWindow::onLogMessage); // 连接监控日志。
    connect(&monitor_, &SystemMonitor::watchdogTriggered, this, &MainWindow::onWatchdogTriggered); // 连接看门狗告警。
    connect(&monitor_, &SystemMonitor::heartbeatPrepared, this, [this](const QJsonObject&) { updateMetrics(); }); // 连接心跳更新。
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::startSystem); // 连接启动按钮。
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::stopSystem); // 连接停止按钮。
    connect(refreshButton_, &QPushButton::clicked, this, [this]() { refreshPersons(); refreshRecords(); updateMetrics(); }); // 连接刷新按钮。
    refreshPersons(); // 初次刷新人员列表。
    refreshRecords(); // 初次刷新记录列表。
    updateMetrics(); // 更新初始统计。
    statusBar()->showMessage(QStringLiteral("系统已就绪")); // 设置状态栏消息。
    resize(1380, 860); // 设置窗口大小。
} // 结束构造函数。
MainWindow::~MainWindow() { // 实现析构函数。
    stopSystem(); // 析构前停止系统。
} // 结束析构函数。
void MainWindow::buildUi() { // 实现界面构建函数。
    QWidget* central = new QWidget(this); // 创建中央控件。
    QVBoxLayout* root = new QVBoxLayout(central); // 创建根布局。
    QHBoxLayout* topBar = new QHBoxLayout; // 创建顶栏布局。
    startButton_ = new QPushButton(QStringLiteral("启动")); // 创建启动按钮。
    stopButton_ = new QPushButton(QStringLiteral("停止")); // 创建停止按钮。
    refreshButton_ = new QPushButton(QStringLiteral("刷新")); // 创建刷新按钮。
    statusLabel_ = new QLabel(QStringLiteral("待机")); // 创建状态标签。
    gateLabel_ = new QLabel(QStringLiteral("闸机关闭")); // 创建闸机状态标签。
    cloudLabel_ = new QLabel(QStringLiteral("离线")); // 创建云端状态标签。
    topBar->addWidget(startButton_); // 添加启动按钮。
    topBar->addWidget(stopButton_); // 添加停止按钮。
    topBar->addWidget(refreshButton_); // 添加刷新按钮。
    topBar->addSpacing(16); // 添加间距。
    topBar->addWidget(new QLabel(QStringLiteral("系统状态："))); // 添加状态说明。
    topBar->addWidget(statusLabel_); // 添加状态标签。
    topBar->addSpacing(16); // 添加间距。
    topBar->addWidget(new QLabel(QStringLiteral("闸机："))); // 添加闸机说明。
    topBar->addWidget(gateLabel_); // 添加闸机标签。
    topBar->addSpacing(16); // 添加间距。
    topBar->addWidget(new QLabel(QStringLiteral("云端："))); // 添加云端说明。
    topBar->addWidget(cloudLabel_); // 添加云端标签。
    topBar->addStretch(); // 添加拉伸空间。
    root->addLayout(topBar); // 放入根布局。
    QSplitter* splitter = new QSplitter(Qt::Horizontal, central); // 创建横向分割器。
    QWidget* leftPanel = new QWidget(splitter); // 创建左侧面板。
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel); // 创建左侧布局。
    videoLabel_ = new QLabel(QStringLiteral("摄像头画面")); // 创建视频显示标签。
    videoLabel_->setMinimumSize(640, 360); // 设置最小尺寸。
    videoLabel_->setAlignment(Qt::AlignCenter); // 设置居中。
    videoLabel_->setStyleSheet(QStringLiteral("background:#111;color:#ddd;border:1px solid #444;")); // 设置样式。
    leftLayout->addWidget(videoLabel_); // 添加视频显示控件。
    leftLayout->addWidget(new QLabel(QStringLiteral("实时日志"))); // 添加日志标题。
    logList_ = new QListWidget(leftPanel); // 创建日志列表。
    leftLayout->addWidget(logList_); // 添加日志列表。
    QWidget* rightPanel = new QWidget(splitter); // 创建右侧面板。
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel); // 创建右侧布局。
    personTable_ = createTable(5, {QStringLiteral("编号"), QStringLiteral("姓名"), QStringLiteral("班组"), QStringLiteral("类型"), QStringLiteral("权限")}); // 创建人员表格。
    recordTable_ = createTable(6, {QStringLiteral("时间"), QStringLiteral("姓名"), QStringLiteral("方向"), QStringLiteral("结果"), QStringLiteral("得分"), QStringLiteral("上传")}); // 创建记录表格。
    rightLayout->addWidget(new QLabel(QStringLiteral("人员库"))); // 添加人员库标题。
    rightLayout->addWidget(personTable_); // 添加人员表格。
    rightLayout->addWidget(new QLabel(QStringLiteral("最近考勤"))); // 添加最近考勤标题。
    rightLayout->addWidget(recordTable_); // 添加记录表格。
    splitter->addWidget(leftPanel); // 添加左侧面板。
    splitter->addWidget(rightPanel); // 添加右侧面板。
    splitter->setStretchFactor(0, 2); // 设置左侧伸缩比例。
    splitter->setStretchFactor(1, 1); // 设置右侧伸缩比例。
    root->addWidget(splitter); // 放入根布局。
    setCentralWidget(central); // 设置中央控件。
} // 结束界面构建函数。
void MainWindow::loadConfig() { // 实现配置读取函数。
    const QJsonObject config = readConfigFile(QStringLiteral("config/device_config.json")); // 读取设备配置。
    deviceId_ = config.value(QStringLiteral("deviceId")).toString(deviceId_); // 读取设备编号。
    recognitionInterval_ = config.value(QStringLiteral("recognitionIntervalFrames")).toInt(recognitionInterval_); // 读取识别间隔。
    const QJsonObject storage = config.value(QStringLiteral("storage")).toObject(); // 读取存储配置。
    attendance_.setDeviceId(deviceId_); // 设置考勤设备编号。
    attendance_.setDuplicateSeconds(config.value(QStringLiteral("duplicateSeconds")).toInt(60)); // 设置重复打卡窗口。
    const QJsonObject thresholds = config.value(QStringLiteral("thresholds")).toObject(); // 读取阈值配置。
    attendance_.setScoreThresholds(thresholds.value(QStringLiteral("cosine")).toDouble(0.82), thresholds.value(QStringLiteral("euclidean")).toDouble(0.75)); // 设置识别阈值。
    attendance_.setRequiredLiveness(true); // 设置强制活体。
    recognizer_.setCosineThreshold(thresholds.value(QStringLiteral("cosine")).toDouble(0.82)); // 设置识别余弦阈值。
    recognizer_.setEuclideanThreshold(thresholds.value(QStringLiteral("euclidean")).toDouble(0.75)); // 设置识别欧氏阈值。
    recognizer_.setQualityThreshold(thresholds.value(QStringLiteral("quality")).toDouble(0.35)); // 设置图像质量阈值。
    camera_.setCameraIndex(config.value(QStringLiteral("cameraIndex")).toInt(0)); // 设置摄像头编号。
    camera_.setFrameIntervalMs(33); // 设置采集间隔。
    accessController_.setDeviceId(deviceId_); // 设置硬件设备编号。
    accessController_.setSimulatedMode(true); // 默认使用模拟硬件。
    cloudClient_.setDeviceId(deviceId_); // 设置云端设备编号。
    cloudClient_.configure(config); // 加载云端配置。
    monitor_.setDeviceId(deviceId_); // 设置监控设备编号。
    monitor_.attachCloudClient(&cloudClient_); // 绑定云端客户端。
    monitor_.attachAccessController(&accessController_); // 绑定硬件控制器。
    monitor_.setWatchdogEnabled(true); // 启用看门狗。
    const QString databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/") + storage.value(QStringLiteral("databasePath")).toString(QStringLiteral("data/smartsite.db")); // 计算数据库路径。
    QDir().mkpath(QFileInfo(databasePath).absolutePath()); // 创建数据库目录。
    DatabaseManager::instance().open(databasePath); // 打开数据库。
    DatabaseManager::instance().initialize(); // 初始化数据库。
    if(DatabaseManager::instance().loadPersons().isEmpty()) { // 判断是否需要演示人员数据。
        Person demo; // 创建演示人员。
        demo.id = QStringLiteral("DEMO-001"); // 设置演示人员编号。
        demo.name = QStringLiteral("演示人员"); // 设置演示人员姓名。
        demo.team = QStringLiteral("示范班组"); // 设置演示人员班组。
        demo.role = QStringLiteral("工人"); // 设置演示人员角色。
        demo.cardNo = QStringLiteral("000000000000000000"); // 设置演示证件号。
        demo.listType = QStringLiteral("white"); // 设置为白名单。
        demo.accessLevel = 1; // 设置权限等级。
        demo.enabled = true; // 设置启用状态。
        QImage demoImage(960, 540, QImage::Format_RGB32); // 创建演示图像。
        demoImage.fill(QColor(18, 24, 38)); // 填充与摄像头演示画面一致的背景。
        QPainter painter(&demoImage); // 创建绘图对象。
        painter.setRenderHint(QPainter::Antialiasing, true); // 打开抗锯齿。
        painter.setBrush(QColor(255, 224, 189)); // 设置肤色。
        painter.setPen(QPen(QColor(255, 224, 189), 2)); // 设置脸部边框。
        painter.drawEllipse(QRect(610, 90, 250, 300)); // 绘制脸部。
        painter.setBrush(Qt::black); // 设置眼睛颜色。
        painter.drawEllipse(QRect(665, 180, 28, 28)); // 绘制左眼。
        painter.drawEllipse(QRect(777, 180, 28, 28)); // 绘制右眼。
        painter.setPen(QPen(Qt::black, 4)); // 设置嘴巴画笔。
        painter.drawArc(QRect(670, 240, 130, 80), 0, -180 * 16); // 绘制嘴巴。
        painter.end(); // 结束绘图。
        recognizer_.enrollPerson(demo, demoImage); // 录入演示人员特征。
    } // 结束演示数据判断。
} // 结束配置读取函数。
void MainWindow::refreshPersons() { // 实现人员刷新函数。
    const QVector<Person> persons = DatabaseManager::instance().loadPersons(); // 读取人员列表。
    personTable_->setRowCount(persons.size()); // 设置表格行数。
    for(int row = 0; row < persons.size(); ++row) { // 遍历人员列表。
        const Person& person = persons[row]; // 取得当前人员。
        personTable_->setItem(row, 0, new QTableWidgetItem(person.id)); // 写入编号。
        personTable_->setItem(row, 1, new QTableWidgetItem(person.name)); // 写入姓名。
        personTable_->setItem(row, 2, new QTableWidgetItem(person.team)); // 写入班组。
        personTable_->setItem(row, 3, new QTableWidgetItem(person.listType)); // 写入类型。
        personTable_->setItem(row, 4, new QTableWidgetItem(QString::number(person.accessLevel))); // 写入权限。
    } // 结束人员遍历。
} // 结束人员刷新函数。
void MainWindow::refreshRecords() { // 实现记录刷新函数。
    const QVector<AttendanceRecord> records = DatabaseManager::instance().loadAttendanceRecords(30); // 读取最近记录。
    recordTable_->setRowCount(records.size()); // 设置表格行数。
    for(int row = 0; row < records.size(); ++row) { // 遍历记录列表。
        const AttendanceRecord& record = records[row]; // 取得当前记录。
        recordTable_->setItem(row, 0, new QTableWidgetItem(record.createdAt.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")))); // 写入时间。
        recordTable_->setItem(row, 1, new QTableWidgetItem(record.personName)); // 写入姓名。
        recordTable_->setItem(row, 2, new QTableWidgetItem(record.direction)); // 写入方向。
        recordTable_->setItem(row, 3, new QTableWidgetItem(record.result)); // 写入结果。
        recordTable_->setItem(row, 4, new QTableWidgetItem(QString::number(record.score, 'f', 3))); // 写入得分。
        recordTable_->setItem(row, 5, new QTableWidgetItem(record.uploaded ? QStringLiteral("已上传") : QStringLiteral("未上传"))); // 写入上传状态。
    } // 结束记录遍历。
} // 结束记录刷新函数。
void MainWindow::appendLogRow(const QString& level, const QString& message) { // 实现日志追加函数。
    const QString line = QStringLiteral("[%1] %2").arg(level, message); // 生成日志行文本。
    logList_->insertItem(0, line); // 插入到顶部。
    if(logList_->count() > 200) { // 判断日志是否过多。
        delete logList_->takeItem(logList_->count() - 1); // 删除最旧日志。
    } // 结束日志数量判断。
} // 结束日志追加函数。
void MainWindow::updateMetrics() { // 实现统计刷新函数。
    cloudLabel_->setText(cloudClient_.isOnline() ? QStringLiteral("在线") : QStringLiteral("离线")); // 刷新云端状态。
    gateLabel_->setText(accessController_.gateStateText()); // 刷新闸机状态。
    statusLabel_->setText(systemRunning_ ? QStringLiteral("运行中") : QStringLiteral("待机")); // 刷新系统状态。
} // 结束统计刷新函数。
void MainWindow::onFrameCaptured(const FramePacket& packet) { // 实现帧处理函数。
    if(packet.image.isNull()) { // 判断图像是否有效。
        return; // 无效时直接返回。
    } // 结束图像判断。
    videoLabel_->setPixmap(QPixmap::fromImage(packet.image).scaled(videoLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 显示画面。
    ++frameCounter_; // 增加帧计数。
    if(systemRunning_ && frameCounter_ % recognitionInterval_ == 0) { // 判断是否到达识别帧。
        const RecognitionResult result = recognizer_.recognize(packet.image); // 执行识别。
        onRecognitionReady(result); // 处理识别结果。
    } // 结束识别帧判断。
} // 结束帧处理函数。
void MainWindow::onRecognitionReady(const RecognitionResult& result) { // 实现识别结果处理函数。
    handleRecognition(result); // 处理识别逻辑。
} // 结束识别结果处理函数。
void MainWindow::handleRecognition(const RecognitionResult& result) { // 实现识别逻辑函数。
    if(attendance_.shouldAllow(result)) { // 判断是否允许通行。
        const AttendanceRecord record = attendance_.createRecord(result, QStringLiteral("in"), QString()); // 创建考勤记录。
        attendance_.rememberAttendance(record); // 记录最近通行时间。
        accessController_.openGate(result.message); // 打开闸机。
        cloudClient_.uploadRecord(record); // 上传考勤记录。
        appendLogRow(QStringLiteral("INFO"), QStringLiteral("%1：%2").arg(result.person.name.isEmpty() ? QStringLiteral("未知人员") : result.person.name, result.message)); // 追加通过日志。
    } else { // 处理拒绝通行。
        accessController_.denyAccess(attendance_.explainDecision(result)); // 拒绝通行并触发声光。
        appendLogRow(QStringLiteral("WARN"), attendance_.explainDecision(result)); // 追加拒绝日志。
    } // 结束通行判断。
    refreshRecords(); // 刷新最近记录。
    updateMetrics(); // 刷新状态信息。
} // 结束识别逻辑函数。
void MainWindow::onStatusMessage(const QString& message) { // 实现状态消息处理函数。
    statusBar()->showMessage(message, 4000); // 在状态栏显示消息。
    appendLogRow(QStringLiteral("INFO"), message); // 追加状态日志。
} // 结束状态消息处理函数。
void MainWindow::onLogMessage(const QString& level, const QString& message) { // 实现日志消息处理函数。
    appendLogRow(level, message); // 追加日志。
} // 结束日志消息处理函数。
void MainWindow::onRecordCreated(const AttendanceRecord& record) { // 实现记录创建处理函数。
    DatabaseManager::instance().saveAttendanceRecord(record); // 保存记录。
    refreshRecords(); // 刷新记录表。
} // 结束记录创建处理函数。
void MainWindow::onOnlineChanged(bool online) { // 实现在线状态变化处理函数。
    cloudLabel_->setText(online ? QStringLiteral("在线") : QStringLiteral("离线")); // 刷新在线显示。
} // 结束在线状态变化处理函数。
void MainWindow::onWatchdogTriggered(const QString& reason) { // 实现看门狗告警处理函数。
    appendLogRow(QStringLiteral("ERROR"), reason); // 追加告警日志。
    accessController_.emergencyStop(); // 执行紧急停止。
} // 结束看门狗处理函数。
void MainWindow::startSystem() { // 实现系统启动函数。
    if(systemRunning_) { // 判断系统是否已运行。
        return; // 已运行时直接返回。
    } // 结束运行判断。
    systemRunning_ = true; // 标记系统运行。
    camera_.start(); // 启动摄像头采集。
    cloudClient_.connectToCloud(); // 连接云端。
    monitor_.start(); // 启动系统监控。
    updateMetrics(); // 刷新状态。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("系统已启动")); // 记录启动日志。
} // 结束系统启动函数。
void MainWindow::stopSystem() { // 实现系统停止函数。
    if(!systemRunning_) { // 判断系统是否已停止。
        return; // 停止状态时直接返回。
    } // 结束运行判断。
    systemRunning_ = false; // 标记系统停止。
    monitor_.stop(); // 停止系统监控。
    camera_.stop(); // 停止摄像头采集。
    cloudClient_.disconnectFromCloud(); // 断开云端连接。
    accessController_.resetIndicators(); // 重置声光状态。
    updateMetrics(); // 刷新状态。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("系统已停止")); // 记录停止日志。
} // 结束系统停止函数。
