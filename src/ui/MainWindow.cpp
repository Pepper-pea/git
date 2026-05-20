#include "ui/MainWindow.h" // 引入主窗口声明。
#include <QApplication> // 引入应用程序对象。
#include <QDateTime> // 引入日期时间。
#include <QAbstractItemView> // 引入表格视图基类。
#include <QCoreApplication> // 引入核心应用对象。
#include <QDir> // 引入目录类型。
#include <QFile> // 引入文件处理。
#include <QHeaderView> // 引入表头视图。
#include <QImage> // 引入图像类型。
#include <QJsonDocument> // 引入 JSON 文档。
#include <QJsonObject> // 引入 JSON 对象。
#include <QLabel> // 引入标签。
#include <QLineEdit> // 引入单行输入框。
#include <QMessageBox> // 引入消息框。
#include <QPixmap> // 引入位图显示。
#include <QRandomGenerator> // 引入随机数生成器。
#include <QSplitter> // 引入分割器。
#include <QStatusBar> // 引入状态栏。
#include <QVBoxLayout> // 引入垂直布局。
#include <QHBoxLayout> // 引入水平布局。
#include <QGroupBox> // 引入分组框。
#include <QFileInfo> // 引入文件信息。
#include <QUuid> // 引入唯一编号生成。
#include <QStringList> // 引入字符串列表。
#include <QTableWidgetItem> // 引入表格项。
#include <QFormLayout> // 引入表单布局。
namespace { // 定义内部辅助命名空间。
constexpr double kTemplateMaxHeadTurnOffset = 0.10; // 正脸模板采集允许的最大头部偏移。
constexpr double kTemplateMaxMouthOpenRatio = 0.35; // 正脸模板采集允许的最大张嘴比例。
constexpr int kTemplateStableFrameCount = 3; // 正脸模板采集需要连续稳定帧数。
constexpr int kTemplateTimeoutSeconds = 8; // 正脸模板采集超时时间。
bool hasRelativeFile(const QString& root, const QString& relativePath) { // 判断某个根目录下是否存在指定相对文件。
    return QFileInfo(QDir(root).absoluteFilePath(relativePath)).exists();
} // 结束相对文件判断函数。
QString findRootUpward(const QString& startPath, bool requireSourceRoot) { // 从指定目录向上查找项目或部署根目录。
    QDir dir(startPath);
    if(!dir.exists()) { // 判断起点是否为文件或不存在。
        dir = QFileInfo(startPath).absoluteDir();
    } // 结束起点修正。
    while(dir.exists()) { // 逐级向上检查目录。
        const QString root = dir.absolutePath();
        const bool isSourceRoot = hasRelativeFile(root, QStringLiteral("SmartSiteFaceAttendance.pro")) && hasRelativeFile(root, QStringLiteral("models/haarcascade_frontalface_default.xml"));
        const bool isDeployRoot = hasRelativeFile(root, QStringLiteral("config/device_config.json"));
        if((requireSourceRoot && isSourceRoot) || (!requireSourceRoot && isDeployRoot)) { // 判断是否命中需要的根目录。
            return root;
        } // 结束命中判断。
        if(!dir.cdUp()) { // 到达盘符根目录时停止。
            break;
        } // 结束向上移动。
    } // 结束目录遍历。
    return QString();
} // 结束向上查找函数。
QString discoverRuntimeRoot() { // 自动发现项目源码根目录或部署根目录。
    const QStringList starts = { // 构造常见启动位置。
        QDir::currentPath(),
        QCoreApplication::applicationDirPath()
    }; // 结束起点列表。
    for(const QString& start : starts) { // 优先查找源码根目录，确保 Qt Creator build 目录能回到项目目录。
        const QString root = findRootUpward(start, true);
        if(!root.isEmpty()) {
            return root;
        } // 结束源码根目录判断。
    } // 结束源码根目录查找。
    for(const QString& start : starts) { // 源码根目录不存在时，再查找部署根目录。
        const QString root = findRootUpward(start, false);
        if(!root.isEmpty()) {
            return root;
        } // 结束部署根目录判断。
    } // 结束部署根目录查找。
    return QDir::currentPath(); // 返回兜底目录。
} // 结束运行根目录发现函数。
QJsonObject readConfigFile(const QString& path, const QString& projectRoot) { // 定义配置文件读取函数。
    const QStringList candidates = { // 创建候选路径列表。
        QDir(projectRoot).absoluteFilePath(path), // 优先使用发现到的项目根目录。
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
QString resolveStoragePath(const QString& configuredPath, const QString& fallbackPath, const QString& projectRoot) { // 将存储路径解析到本地项目目录。
    const QString path = configuredPath.trimmed().isEmpty() ? fallbackPath : configuredPath.trimmed(); // 选择配置路径或默认路径。
    const QFileInfo info(path); // 创建路径信息对象。
    if(info.isAbsolute()) { // 判断是否已经是绝对路径。
        return info.absoluteFilePath(); // 返回绝对路径。
    } // 结束绝对路径判断。
    const QString baseDir = projectRoot.isEmpty() ? QDir::currentPath() : projectRoot; // 选择项目根目录或当前目录。
    return QDir(baseDir).absoluteFilePath(path); // 返回项目目录下的完整路径。
} // 结束存储路径解析函数。
QString recognitionStatusText(RecognitionStatus status) { // 将识别状态转换成便于 Navicat 查看的人类可读文本。
    switch(status) { // 按识别状态分支。
    case RecognitionStatus::Accepted: return QStringLiteral("通过"); // 通过状态。
    case RecognitionStatus::Denied: return QStringLiteral("拒绝"); // 拒绝状态。
    case RecognitionStatus::Stranger: return QStringLiteral("陌生人"); // 陌生人状态。
    case RecognitionStatus::Spoof: return QStringLiteral("活体失败"); // 活体失败状态。
    case RecognitionStatus::LowQuality: return QStringLiteral("图像质量不足"); // 质量不足状态。
    } // 结束分支。
    return QStringLiteral("未知"); // 返回兜底文本。
} // 结束识别状态转换函数。
QString enrollmentChallengeText(EnrollmentChallengeType challenge) { // 将录入活体动作转换成界面提示文本。
    switch(challenge) { // 按动作类型分支。
    case EnrollmentChallengeType::Blink: return QStringLiteral("眨一下眼"); // 眨眼动作。
    case EnrollmentChallengeType::TurnLeft: return QStringLiteral("向左转头"); // 左侧转头动作。
    case EnrollmentChallengeType::TurnRight: return QStringLiteral("向右转头"); // 右侧转头动作。
    case EnrollmentChallengeType::OpenMouth: return QStringLiteral("张嘴"); // 张嘴动作。
    } // 结束分支。
    return QStringLiteral("完成活体动作"); // 返回兜底文本。
} // 结束录入动作文本函数。
bool isTurnChallenge(EnrollmentChallengeType challenge) { // 判断当前录入动作是否为转头动作。
    return challenge == EnrollmentChallengeType::TurnLeft || challenge == EnrollmentChallengeType::TurnRight; // 返回转头动作判断结果。
} // 结束转头动作判断函数。
EnrollmentChallengeType randomEnrollmentChallenge() { // 随机生成一个录入活体动作。
    switch(QRandomGenerator::global()->bounded(4)) { // 在四种动作中随机选择。
    case 0: return EnrollmentChallengeType::Blink; // 选择眨眼。
    case 1: return EnrollmentChallengeType::TurnLeft; // 选择向左转头。
    case 2: return EnrollmentChallengeType::TurnRight; // 选择向右转头。
    default: return EnrollmentChallengeType::OpenMouth; // 选择张嘴。
    } // 结束随机分支。
} // 结束随机动作函数。
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
    qRegisterMetaType<RecognizedFaceRecord>(); // 注册识别人脸记录类型。
    buildUi(); // 构建界面。
    connect(&recognizer_, &FaceRecognizer::logMessage, this, &MainWindow::onLogMessage, Qt::UniqueConnection); // 先连接识别日志，便于显示模型加载结果。
    loadConfig(); // 读取配置。
    connect(&camera_, &CameraWorker::frameCaptured, this, &MainWindow::onFrameCaptured); // 连接摄像头帧信号。
    connect(&camera_, &CameraWorker::statusMessage, this, &MainWindow::onStatusMessage); // 连接摄像头状态信号。
    connect(&recognizer_, &FaceRecognizer::logMessage, this, &MainWindow::onLogMessage, Qt::UniqueConnection); // 连接识别日志。
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
    connect(enrollButton_, &QPushButton::clicked, this, &MainWindow::enrollCurrentFace); // 连接人员录入按钮。
    connect(clearEnrollButton_, &QPushButton::clicked, this, &MainWindow::clearEnrollmentForm); // 连接清空录入表单按钮。
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
    QGroupBox* enrollBox = new QGroupBox(QStringLiteral("人员录入"), rightPanel); // 创建人员录入区域。
    QFormLayout* enrollForm = new QFormLayout(enrollBox); // 创建录入表单布局。
    enrollIdEdit_ = new QLineEdit(enrollBox); // 创建人员编号输入框。
    enrollNameEdit_ = new QLineEdit(enrollBox); // 创建人员姓名输入框。
    enrollTeamEdit_ = new QLineEdit(enrollBox); // 创建班组输入框。
    enrollRoleEdit_ = new QLineEdit(enrollBox); // 创建角色输入框。
    enrollCardEdit_ = new QLineEdit(enrollBox); // 创建证件号输入框。
    enrollListTypeCombo_ = new QComboBox(enrollBox); // 创建名单类型下拉框。
    enrollAccessSpin_ = new QSpinBox(enrollBox); // 创建权限等级输入框。
    enrollStatusLabel_ = new QLabel(QStringLiteral("请先启动摄像头，再录入当前画面"), enrollBox); // 创建录入状态提示。
    enrollButton_ = new QPushButton(QStringLiteral("录入当前人脸"), enrollBox); // 创建录入按钮。
    clearEnrollButton_ = new QPushButton(QStringLiteral("清空"), enrollBox); // 创建清空按钮。
    enrollIdEdit_->setPlaceholderText(QStringLiteral("留空自动生成")); // 设置编号提示。
    enrollNameEdit_->setPlaceholderText(QStringLiteral("必填")); // 设置姓名提示。
    enrollTeamEdit_->setPlaceholderText(QStringLiteral("例如：钢筋班组")); // 设置班组提示。
    enrollRoleEdit_->setPlaceholderText(QStringLiteral("例如：工人")); // 设置角色提示。
    enrollCardEdit_->setPlaceholderText(QStringLiteral("身份证号或实名制卡号")); // 设置证件提示。
    enrollListTypeCombo_->addItem(QStringLiteral("白名单"), QStringLiteral("white")); // 添加白名单选项。
    enrollListTypeCombo_->addItem(QStringLiteral("黑名单"), QStringLiteral("black")); // 添加黑名单选项。
    enrollAccessSpin_->setRange(0, 99); // 设置权限等级范围。
    enrollAccessSpin_->setValue(1); // 设置默认权限等级。
    enrollStatusLabel_->setWordWrap(true); // 允许状态提示换行。
    enrollForm->addRow(QStringLiteral("编号"), enrollIdEdit_); // 添加编号输入。
    enrollForm->addRow(QStringLiteral("姓名"), enrollNameEdit_); // 添加姓名输入。
    enrollForm->addRow(QStringLiteral("班组"), enrollTeamEdit_); // 添加班组输入。
    enrollForm->addRow(QStringLiteral("角色"), enrollRoleEdit_); // 添加角色输入。
    enrollForm->addRow(QStringLiteral("证件"), enrollCardEdit_); // 添加证件输入。
    enrollForm->addRow(QStringLiteral("名单"), enrollListTypeCombo_); // 添加名单类型。
    enrollForm->addRow(QStringLiteral("权限"), enrollAccessSpin_); // 添加权限等级。
    QHBoxLayout* enrollActions = new QHBoxLayout; // 创建录入按钮布局。
    enrollActions->addWidget(enrollButton_); // 添加录入按钮。
    enrollActions->addWidget(clearEnrollButton_); // 添加清空按钮。
    enrollForm->addRow(enrollActions); // 添加按钮行。
    enrollForm->addRow(enrollStatusLabel_); // 添加状态提示。
    personTable_ = createTable(5, {QStringLiteral("编号"), QStringLiteral("姓名"), QStringLiteral("班组"), QStringLiteral("类型"), QStringLiteral("权限")}); // 创建人员表格。
    recordTable_ = createTable(6, {QStringLiteral("时间"), QStringLiteral("姓名"), QStringLiteral("方向"), QStringLiteral("结果"), QStringLiteral("得分"), QStringLiteral("上传")}); // 创建记录表格。
    rightLayout->addWidget(enrollBox); // 添加人员录入区域。
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
    const QString projectRoot = discoverRuntimeRoot(); // 自动发现源码根目录或部署根目录。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("项目根目录：%1").arg(projectRoot)); // 记录路径解析结果。
    const QJsonObject config = readConfigFile(QStringLiteral("config/device_config.json"), projectRoot); // 读取设备配置。
    deviceId_ = config.value(QStringLiteral("deviceId")).toString(deviceId_); // 读取设备编号。
    recognitionInterval_ = config.value(QStringLiteral("recognitionIntervalFrames")).toInt(recognitionInterval_); // 读取识别间隔。
    const QJsonObject storage = config.value(QStringLiteral("storage")).toObject(); // 读取存储配置。
    attendance_.setDeviceId(deviceId_); // 设置考勤设备编号。
    attendance_.setDuplicateSeconds(config.value(QStringLiteral("duplicateSeconds")).toInt(60)); // 设置重复打卡窗口。
    const QJsonObject thresholds = config.value(QStringLiteral("thresholds")).toObject(); // 读取阈值配置。
    attendance_.setScoreThresholds(thresholds.value(QStringLiteral("cosine")).toDouble(0.82), thresholds.value(QStringLiteral("euclidean")).toDouble(0.75)); // 设置识别阈值。
    recognizer_.setCosineThreshold(thresholds.value(QStringLiteral("cosine")).toDouble(0.82)); // 设置识别余弦阈值。
    recognizer_.setEuclideanThreshold(thresholds.value(QStringLiteral("euclidean")).toDouble(0.75)); // 设置识别欧氏阈值。
    recognizer_.setQualityThreshold(thresholds.value(QStringLiteral("quality")).toDouble(0.35)); // 设置图像质量阈值。
    const QJsonObject liveness = config.value(QStringLiteral("liveness")).toObject(); // 读取活体策略配置。
    const double earThreshold = thresholds.value(QStringLiteral("ear")).toDouble(0.21); // 读取 EAR 阈值。
    recognizer_.setEarThreshold(earThreshold); // 设置 dlib 68 点 EAR 活体阈值。
    enrollmentLiveness_.setEarThreshold(earThreshold); // 设置录入专用 EAR 阈值。
    enrollmentLiveness_.setMouthOpenThreshold(liveness.value(QStringLiteral("mouthOpenRatio")).toDouble(0.45)); // 设置录入张嘴阈值。
    enrollmentTurnOffsetThreshold_ = liveness.value(QStringLiteral("turnOffsetRatio")).toDouble(0.18); // 读取录入转头阈值。
    enrollmentLiveness_.setTurnOffsetThreshold(enrollmentTurnOffsetThreshold_); // 设置录入转头阈值。
    enrollmentLiveness_.setStableFrameCount(liveness.value(QStringLiteral("stableFrames")).toInt(2)); // 设置动作连续通过帧数。
    enrollmentChallengeTimeoutSeconds_ = qMax(1, liveness.value(QStringLiteral("enrollmentChallengeTimeoutSeconds")).toInt(15)); // 设置录入动作超时时间。
    const QJsonObject models = config.value(QStringLiteral("models")).toObject(); // 读取模型路径配置。
    const QString faceCascadePath = resolveStoragePath(models.value(QStringLiteral("faceCascade")).toString(), QStringLiteral("models/haarcascade_frontalface_default.xml"), projectRoot); // 计算 OpenCV Haar 模型路径。
    const QString shapePredictorPath = resolveStoragePath(models.value(QStringLiteral("shapePredictor68")).toString(), QStringLiteral("models/shape_predictor_68_face_landmarks.dat"), projectRoot); // 计算 dlib 68 点模型路径。
    recognizer_.setUseDlib(true); // 要求使用 dlib 68 点模型。
    recognizer_.setModelPaths(faceCascadePath, shapePredictorPath); // 设置真实模型路径。
    recognizer_.loadModels(); // 加载 OpenCV Haar 与 dlib 68 点模型。
    const bool requireLivenessForRecognition = liveness.value(QStringLiteral("requireForRecognition")).toBool(false); // 读取识别是否强制活体。
    attendance_.setRequiredLiveness(requireLivenessForRecognition && recognizer_.livenessAvailable()); // 普通识别默认不强制活体。
    if(requireLivenessForRecognition && !recognizer_.livenessAvailable()) { // 判断配置是否要求识别活体但模型不可用。
        appendLogRow(QStringLiteral("WARN"), QStringLiteral("识别活体已配置但 dlib 68 点不可用，普通识别不会强制眨眼")); // 记录配置降级。
    } // 结束识别活体降级日志。
    if(!recognizer_.livenessAvailable()) { // 判断是否需要降级到 Haar 基础流程。
        appendLogRow(QStringLiteral("WARN"), QStringLiteral("当前 dlib 68 点活体不可用，普通识别可用，活体动作录入不可用")); // 记录降级说明。
    } // 结束降级日志。
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
    const QString databasePath = resolveStoragePath(storage.value(QStringLiteral("databasePath")).toString(), QStringLiteral("data/smartsite.sqlite"), projectRoot); // 计算数据库路径。
    captureDir_ = resolveStoragePath(storage.value(QStringLiteral("captureDir")).toString(), QStringLiteral("data/captures"), projectRoot); // 计算抓拍目录。
    QDir().mkpath(QFileInfo(databasePath).absolutePath()); // 创建数据库目录。
    QDir().mkpath(captureDir_); // 创建抓拍目录。
    DatabaseManager::instance().open(databasePath); // 打开数据库。
    DatabaseManager::instance().initialize(); // 初始化数据库。
    if(DatabaseManager::instance().loadPersons().isEmpty()) { // 判断人员库是否为空。
        appendLogRow(QStringLiteral("INFO"), QStringLiteral("人员库为空，请通过右侧人员录入采集真实人脸")); // 提示录入真实人员。
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
QString MainWindow::buildEnrollmentPersonId() const { // 实现默认人员编号生成函数。
    return QStringLiteral("PER-%1-%2").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz")), QUuid::createUuid().toString(QUuid::Id128).left(6)); // 返回时间加随机段的编号。
} // 结束默认人员编号生成函数。
QString MainWindow::saveRecognitionSnapshot(const FramePacket& packet) const { // 实现识别抓拍保存函数。
    if(packet.image.isNull() || captureDir_.isEmpty()) { // 判断图像和目录是否有效。
        return QString(); // 无效时返回空路径。
    } // 结束有效性判断。
    QDir().mkpath(captureDir_); // 确保抓拍目录存在。
    const QString fileName = QStringLiteral("%1_frame_%2_%3.jpg").arg(deviceId_, QString::number(packet.frameId), QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz"))); // 生成抓拍文件名。
    const QString filePath = QDir(captureDir_).absoluteFilePath(fileName); // 生成抓拍完整路径。
    return packet.image.save(filePath, "JPG", 85) ? filePath : QString(); // 保存图片并返回路径。
} // 结束识别抓拍保存函数。
RecognizedFaceRecord MainWindow::buildRecognizedFaceRecord(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath, bool accessAllowed, const QString& decision) const { // 实现识别记录构建函数。
    RecognizedFaceRecord record; // 创建识别记录对象。
    record.id = QStringLiteral("FACE-%1-%2").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz")), QUuid::createUuid().toString(QUuid::Id128).left(8)); // 生成识别记录编号。
    record.frameId = frameId; // 写入帧序号。
    record.personId = result.person.id; // 写入人员编号。
    record.personName = result.person.name.isEmpty() ? QStringLiteral("未知人员") : result.person.name; // 写入人员姓名。
    record.team = result.person.team; // 写入班组。
    record.status = recognitionStatusText(result.status); // 写入识别状态。
    record.accessAllowed = accessAllowed; // 写入通行结果。
    record.decision = decision; // 写入最终判定。
    record.message = result.message; // 写入识别说明。
    record.score = result.score; // 写入综合得分。
    record.cosine = result.cosine; // 写入余弦相似度。
    record.euclidean = result.euclidean; // 写入欧氏距离。
    record.quality = result.detection.quality; // 写入图像质量。
    record.blinkDetected = result.detection.blinkDetected; // 写入活体结果。
    record.faceRect = result.detection.rect; // 写入人脸矩形。
    record.snapshotPath = snapshotPath; // 写入抓拍路径。
    record.deviceId = deviceId_; // 写入设备编号。
    record.createdAt = result.createdAt.isValid() ? result.createdAt : QDateTime::currentDateTime(); // 写入识别时间。
    return record; // 返回识别记录。
} // 结束识别记录构建函数。
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
    lastFrame_ = packet.image.copy(); // 保存最近一帧供人员录入使用。
    lastFrameId_ = packet.frameId; // 保存最近一帧序号。
    videoLabel_->setPixmap(QPixmap::fromImage(packet.image).scaled(videoLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 显示画面。
    ++frameCounter_; // 增加帧计数。
    if(enrollmentChallengeActive_) { // 判断是否正在进行录入活体动作。
        processEnrollmentChallenge(packet.image, packet.frameId); // 优先处理录入活体，暂停普通识别。
        return; // 避免录入过程中触发开门识别。
    } // 结束录入活体判断。
    if(systemRunning_ && frameCounter_ % recognitionInterval_ == 0) { // 判断是否到达识别帧。
        const RecognitionResult result = recognizer_.recognize(packet.image); // 执行识别。
        const QString snapshotPath = saveRecognitionSnapshot(packet); // 保存识别抓拍图片。
        onRecognitionReady(result, packet.frameId, snapshotPath); // 处理识别结果。
    } // 结束识别帧判断。
} // 结束帧处理函数。
void MainWindow::onRecognitionReady(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath) { // 实现识别结果处理函数。
    handleRecognition(result, frameId, snapshotPath); // 处理识别逻辑。
} // 结束识别结果处理函数。
void MainWindow::handleRecognition(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath) { // 实现识别逻辑函数。
    const bool allowed = attendance_.shouldAllow(result); // 计算是否允许通行。
    const QString decision = allowed ? QStringLiteral("允许通行") : attendance_.explainDecision(result); // 生成最终通行判定。
    const RecognizedFaceRecord faceRecord = buildRecognizedFaceRecord(result, frameId, snapshotPath, allowed, decision); // 构建识别落库记录。
    if(!DatabaseManager::instance().saveRecognizedFace(faceRecord)) { // 保存识别记录。
        appendLogRow(QStringLiteral("ERROR"), QStringLiteral("识别记录保存失败：%1").arg(DatabaseManager::instance().lastError())); // 写入错误日志。
    } // 结束识别记录保存判断。
    if(allowed) { // 判断是否允许通行。
        const AttendanceRecord record = attendance_.createRecord(result, QStringLiteral("in"), snapshotPath); // 创建考勤记录。
        attendance_.rememberAttendance(record); // 记录最近通行时间。
        accessController_.openGate(result.message); // 打开闸机。
        cloudClient_.uploadRecord(record); // 上传考勤记录。
        appendLogRow(QStringLiteral("INFO"), QStringLiteral("%1：%2").arg(result.person.name.isEmpty() ? QStringLiteral("未知人员") : result.person.name, result.message)); // 追加通过日志。
    } else { // 处理拒绝通行。
        accessController_.denyAccess(decision); // 拒绝通行并触发声光。
        appendLogRow(QStringLiteral("WARN"), decision); // 追加拒绝日志。
    } // 结束通行判断。
    refreshRecords(); // 刷新最近记录。
    updateMetrics(); // 刷新状态信息。
} // 结束识别逻辑函数。
void MainWindow::processEnrollmentChallenge(const QImage& image, qint64 frameId) { // 实现录入活体动作挑战处理函数。
    if(!enrollmentChallengeActive_) { // 判断是否没有挑战任务。
        return; // 无任务时直接返回。
    } // 结束任务判断。
    if(enrollmentStage_ == EnrollmentStage::CollectTemplate) { // 判断是否已经进入正脸模板采集阶段。
        processEnrollmentTemplateCapture(image, frameId); // 处理正脸模板采集。
        return; // 停止当前阶段处理。
    } // 结束模板采集阶段判断。
    if(enrollmentStage_ != EnrollmentStage::Challenge) { // 判断当前阶段是否异常。
        return; // 异常阶段直接等待下一帧。
    } // 结束阶段判断。
    const QString actionText = enrollmentChallengeText(enrollmentChallenge_); // 获取当前动作文本。
    const int elapsedSeconds = enrollmentChallengeStartedAt_.secsTo(QDateTime::currentDateTime()); // 计算已用时间。
    const int remainingSeconds = qMax(0, enrollmentChallengeTimeoutSeconds_ - elapsedSeconds); // 计算剩余时间。
    if(elapsedSeconds > enrollmentChallengeTimeoutSeconds_) { // 判断挑战是否超时。
        const QString reason = isTurnChallenge(enrollmentChallenge_) ? QStringLiteral("录入超时：没有检测到有效%1。请重新录入，按本人方向%1并保持 1 秒。").arg(actionText) : QStringLiteral("录入超时：没有检测到有效%1。请重新录入并保持脸部清晰。").arg(actionText); // 生成更明确的失败原因。
        const QString diagnostic = QStringLiteral("录入动作超时诊断：动作=%1，人脸=%2，68点=%3，最后转头偏移=%4，阈值=%5，张嘴比例=%6").arg(actionText, enrollmentLastFaceDetected_ ? QStringLiteral("已检测到") : QStringLiteral("未检测到"), enrollmentLastLandmarksReady_ ? QStringLiteral("就绪") : QStringLiteral("未就绪"), QString::number(enrollmentLastHeadTurnOffset_, 'f', 3), QString::number(enrollmentTurnOffsetThreshold_, 'f', 3), QString::number(enrollmentLastMouthOpenRatio_, 'f', 3)); // 生成诊断日志。
        finishEnrollmentChallenge(true); // 结束挑战并清空待录人员。
        enrollStatusLabel_->setText(reason); // 更新录入状态。
        appendLogRow(QStringLiteral("WARN"), reason); // 写入日志。
        appendLogRow(QStringLiteral("WARN"), diagnostic); // 写入诊断日志。
        statusBar()->showMessage(reason, 4000); // 显示状态栏提示。
        return; // 停止处理。
    } // 结束超时判断。
    QVector<FaceDetection> faces = recognizer_.detectFaces(image); // 检测当前画面人脸。
    if(faces.isEmpty()) { // 判断是否没有检测到人脸。
        enrollmentLastFaceDetected_ = false; // 记录最近没有检测到人脸。
        enrollmentLastLandmarksReady_ = false; // 记录关键点未就绪。
        enrollmentLiveness_.reset(); // 人脸丢失时重置连续通过状态。
        enrollStatusLabel_->setText(QStringLiteral("请正对摄像头，让脸完整出现在画面中。正在等待：%1（剩余 %2 秒）").arg(actionText, QString::number(remainingSeconds))); // 提示调整位置。
        return; // 等待下一帧。
    } // 结束人脸判断。
    FaceDetection detection = faces.first(); // 取最大人脸作为录入目标。
    enrollmentLastFaceDetected_ = true; // 记录最近已检测到人脸。
    if(detection.landmarks.size() < 68) { // 判断是否缺少关键点。
        enrollmentLastLandmarksReady_ = false; // 记录关键点未就绪。
        enrollmentLiveness_.reset(); // 关键点丢失时重置连续通过状态。
        enrollStatusLabel_->setText(QStringLiteral("请保持脸部清晰，稍微靠近摄像头。正在等待：%1（剩余 %2 秒）").arg(actionText, QString::number(remainingSeconds))); // 提示关键点状态。
        return; // 等待下一帧。
    } // 结束关键点判断。
    enrollmentLastLandmarksReady_ = true; // 记录关键点已就绪。
    if(!enrollmentLiveness_.updateChallenge(detection, enrollmentChallenge_)) { // 判断当前动作是否尚未通过。
        if(isTurnChallenge(enrollmentChallenge_)) { // 判断当前是否为转头动作。
            enrollmentLastHeadTurnOffset_ = detection.headTurnOffset; // 保存最近一次转头偏移。
            enrollStatusLabel_->setText(QStringLiteral("请继续%1，幅度稍微大一点，保持一会儿（剩余 %2 秒）").arg(actionText, QString::number(remainingSeconds))); // 更新转头动作提示。
        } else { // 处理其他动作。
            if(enrollmentChallenge_ == EnrollmentChallengeType::OpenMouth) { // 判断是否为张嘴动作。
                enrollmentLastMouthOpenRatio_ = detection.mouthOpenRatio; // 保存最近一次张嘴比例。
            } // 结束张嘴比例记录。
            enrollStatusLabel_->setText(QStringLiteral("正在检测动作：%1，通过后请回正脸采集模板（剩余 %2 秒）").arg(actionText, QString::number(remainingSeconds))); // 更新动作提示。
        } // 结束动作提示判断。
        return; // 等待下一帧。
    } // 结束动作判断。
    if(isTurnChallenge(enrollmentChallenge_)) { // 判断当前是否为转头动作。
        enrollmentLastHeadTurnOffset_ = detection.headTurnOffset; // 保存通过时的转头偏移。
    } else if(enrollmentChallenge_ == EnrollmentChallengeType::OpenMouth) { // 判断是否为张嘴动作。
        enrollmentLastMouthOpenRatio_ = detection.mouthOpenRatio; // 保存通过时的张嘴比例。
    } // 结束转头偏移记录。
    enrollmentStage_ = EnrollmentStage::CollectTemplate; // 切换到正脸模板采集阶段。
    enrollmentTemplateStartedAt_ = QDateTime::currentDateTime(); // 记录正脸模板采集开始时间。
    enrollmentTemplateStableFrames_ = 0; // 重置稳定帧计数。
    enrollmentBestTemplateFrame_ = QImage(); // 清空旧最佳模板帧。
    enrollmentBestTemplateFrameId_ = 0; // 清空旧最佳模板帧号。
    enrollmentBestTemplateQuality_ = -1.0; // 清空旧最佳模板质量。
    enrollmentLiveness_.reset(); // 重置活体检测器状态，避免影响后续录入。
    enrollStatusLabel_->setText(QStringLiteral("动作通过，请正对摄像头保持 1 秒，正在采集人脸模板。")); // 更新模板采集提示。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("录入动作通过：%1（帧号 %2），开始采集正脸模板").arg(actionText, QString::number(frameId))); // 写入阶段切换日志。
    statusBar()->showMessage(QStringLiteral("动作通过，请正对摄像头"), 4000); // 显示状态栏提示。
} // 结束录入活体动作挑战处理函数。
void MainWindow::processEnrollmentTemplateCapture(const QImage& image, qint64 frameId) { // 实现录入正脸模板采集函数。
    const QString actionText = enrollmentChallengeText(enrollmentChallenge_); // 获取已完成动作文本。
    const int elapsedSeconds = enrollmentTemplateStartedAt_.secsTo(QDateTime::currentDateTime()); // 计算模板采集已用时间。
    const int remainingSeconds = qMax(0, kTemplateTimeoutSeconds - elapsedSeconds); // 计算模板采集剩余时间。
    if(elapsedSeconds > kTemplateTimeoutSeconds) { // 判断正脸模板采集是否超时。
        const QString reason = QStringLiteral("录入失败：正脸模板采集超时，请重新录入，动作通过后正对摄像头保持 1 秒。"); // 生成失败原因。
        const QString diagnostic = QStringLiteral("正脸模板采集超时诊断：动作=%1，人脸=%2，68点=%3，最后转头偏移=%4，允许偏移=%5，最后张嘴比例=%6，允许比例=%7，最佳质量=%8").arg(actionText, enrollmentLastFaceDetected_ ? QStringLiteral("已检测到") : QStringLiteral("未检测到"), enrollmentLastLandmarksReady_ ? QStringLiteral("就绪") : QStringLiteral("未就绪"), QString::number(enrollmentLastHeadTurnOffset_, 'f', 3), QString::number(kTemplateMaxHeadTurnOffset, 'f', 3), QString::number(enrollmentLastMouthOpenRatio_, 'f', 3), QString::number(kTemplateMaxMouthOpenRatio, 'f', 3), QString::number(enrollmentBestTemplateQuality_, 'f', 3)); // 生成诊断日志。
        finishEnrollmentChallenge(true); // 结束录入流程并清空待录人员。
        enrollStatusLabel_->setText(reason); // 更新录入状态。
        appendLogRow(QStringLiteral("WARN"), reason); // 写入失败日志。
        appendLogRow(QStringLiteral("WARN"), diagnostic); // 写入诊断日志。
        statusBar()->showMessage(QStringLiteral("正脸模板采集超时"), 4000); // 显示状态栏提示。
        return; // 停止处理。
    } // 结束超时判断。
    QVector<FaceDetection> faces = recognizer_.detectFaces(image); // 检测当前画面人脸。
    if(faces.isEmpty()) { // 判断是否没有检测到人脸。
        enrollmentLastFaceDetected_ = false; // 记录最近没有检测到人脸。
        enrollmentLastLandmarksReady_ = false; // 记录关键点未就绪。
        enrollmentTemplateStableFrames_ = 0; // 重置稳定帧计数。
        enrollStatusLabel_->setText(QStringLiteral("动作已通过。请正对摄像头，让脸完整出现在画面中（剩余 %1 秒）").arg(remainingSeconds)); // 提示正对摄像头。
        return; // 等待下一帧。
    } // 结束人脸判断。
    FaceDetection detection = faces.first(); // 取最大人脸作为模板采集目标。
    enrollmentLastFaceDetected_ = true; // 记录最近检测到人脸。
    if(detection.landmarks.size() < 68 || !enrollmentLiveness_.measurePose(detection)) { // 判断关键点和姿态测量是否就绪。
        enrollmentLastLandmarksReady_ = false; // 记录关键点未就绪。
        enrollmentTemplateStableFrames_ = 0; // 重置稳定帧计数。
        enrollStatusLabel_->setText(QStringLiteral("动作已通过。请保持脸部清晰，稍微靠近摄像头（剩余 %1 秒）").arg(remainingSeconds)); // 提示关键点状态。
        return; // 等待下一帧。
    } // 结束关键点判断。
    enrollmentLastLandmarksReady_ = true; // 记录关键点已就绪。
    enrollmentLastHeadTurnOffset_ = detection.headTurnOffset; // 记录最近头部偏移。
    enrollmentLastMouthOpenRatio_ = detection.mouthOpenRatio; // 记录最近张嘴比例。
    if(qAbs(detection.headTurnOffset) > kTemplateMaxHeadTurnOffset) { // 判断头部是否未回正。
        enrollmentTemplateStableFrames_ = 0; // 重置稳定帧计数。
        enrollStatusLabel_->setText(QStringLiteral("动作已通过。请把头回正，正对摄像头保持一会儿（剩余 %1 秒）").arg(remainingSeconds)); // 提示回正。
        return; // 等待下一帧。
    } // 结束头部回正判断。
    if(detection.mouthOpenRatio > kTemplateMaxMouthOpenRatio) { // 判断嘴部是否仍处于动作状态。
        enrollmentTemplateStableFrames_ = 0; // 重置稳定帧计数。
        enrollStatusLabel_->setText(QStringLiteral("动作已通过。请恢复自然表情，闭合嘴部后保持一会儿（剩余 %1 秒）").arg(remainingSeconds)); // 提示恢复自然表情。
        return; // 等待下一帧。
    } // 结束嘴部状态判断。
    if(detection.quality > enrollmentBestTemplateQuality_) { // 判断当前帧质量是否更高。
        enrollmentBestTemplateFrame_ = image.copy(); // 保存质量最高的完整帧。
        enrollmentBestTemplateFrameId_ = frameId; // 保存质量最高帧号。
        enrollmentBestTemplateQuality_ = detection.quality; // 保存质量分。
    } // 结束最佳帧更新。
    ++enrollmentTemplateStableFrames_; // 增加连续稳定帧数。
    if(enrollmentTemplateStableFrames_ < kTemplateStableFrameCount) { // 判断是否还未达到稳定帧要求。
        enrollStatusLabel_->setText(QStringLiteral("正在采集正脸模板，请保持不动（%1/%2，剩余 %3 秒）").arg(QString::number(enrollmentTemplateStableFrames_), QString::number(kTemplateStableFrameCount), QString::number(remainingSeconds))); // 更新稳定采集提示。
        return; // 等待下一帧。
    } // 结束稳定帧判断。
    const Person enrolledPerson = pendingEnrollmentPerson_; // 保存待录人员副本。
    const QImage templateFrame = enrollmentBestTemplateFrame_.isNull() ? image : enrollmentBestTemplateFrame_; // 选择最佳模板帧或当前帧。
    const qint64 templateFrameId = enrollmentBestTemplateFrameId_ == 0 ? frameId : enrollmentBestTemplateFrameId_; // 选择最佳模板帧号或当前帧号。
    const double templateQuality = enrollmentBestTemplateQuality_; // 保存模板质量用于日志。
    const bool saved = recognizer_.enrollPerson(enrolledPerson, templateFrame); // 使用正脸模板帧保存人员特征。
    finishEnrollmentChallenge(true); // 结束挑战并恢复表单。
    if(!saved) { // 判断保存是否失败。
        const QString reason = !recognizer_.lastError().isEmpty() ? recognizer_.lastError() : QStringLiteral("录入失败，请重新尝试"); // 生成失败原因。
        enrollStatusLabel_->setText(QStringLiteral("录入失败：%1").arg(reason)); // 更新录入状态。
        appendLogRow(QStringLiteral("WARN"), QStringLiteral("录入人员失败：%1").arg(reason)); // 写入日志。
        statusBar()->showMessage(QStringLiteral("人员录入失败"), 4000); // 显示状态栏提示。
        return; // 停止处理。
    } // 结束保存失败判断。
    enrollStatusLabel_->setText(QStringLiteral("录入成功：%1（%2）").arg(enrolledPerson.name, enrolledPerson.id)); // 更新录入状态。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("录入人员：%1（%2），动作：%3，模板帧号 %4，质量 %5").arg(enrolledPerson.name, enrolledPerson.id, actionText, QString::number(templateFrameId), QString::number(templateQuality, 'f', 3))); // 写入录入日志。
    refreshPersons(); // 刷新人员库。
    statusBar()->showMessage(QStringLiteral("人员已录入：%1").arg(enrolledPerson.name), 4000); // 显示状态栏提示。
} // 结束录入正脸模板采集函数。
void MainWindow::finishEnrollmentChallenge(bool clearPendingPerson) { // 实现录入活体动作挑战结束函数。
    enrollmentChallengeActive_ = false; // 标记挑战结束。
    enrollmentStage_ = EnrollmentStage::Idle; // 重置录入流程阶段。
    enrollmentChallengeStartedAt_ = QDateTime(); // 清空开始时间。
    enrollmentTemplateStartedAt_ = QDateTime(); // 清空模板采集开始时间。
    enrollmentTemplateStableFrames_ = 0; // 清空模板稳定帧计数。
    enrollmentBestTemplateFrame_ = QImage(); // 清空最佳模板帧。
    enrollmentBestTemplateFrameId_ = 0; // 清空最佳模板帧号。
    enrollmentBestTemplateQuality_ = -1.0; // 清空最佳模板质量。
    enrollmentLastFaceDetected_ = false; // 清空最近人脸检测状态。
    enrollmentLastLandmarksReady_ = false; // 清空最近关键点状态。
    enrollmentLastHeadTurnOffset_ = 0.0; // 清空最近转头偏移。
    enrollmentLastMouthOpenRatio_ = 0.0; // 清空最近张嘴比例。
    enrollmentLiveness_.reset(); // 重置活体检测器状态。
    if(clearPendingPerson) { // 判断是否清空待录人员。
        pendingEnrollmentPerson_ = Person(); // 清空待录人员。
    } // 结束待录人员清理。
    setEnrollmentInputsEnabled(true); // 恢复表单编辑。
    enrollButton_->setEnabled(true); // 恢复录入按钮。
    clearEnrollButton_->setText(QStringLiteral("清空")); // 恢复清空按钮文本。
} // 结束挑战结束函数。
void MainWindow::setEnrollmentInputsEnabled(bool enabled) { // 实现录入表单启用状态设置函数。
    enrollIdEdit_->setEnabled(enabled); // 设置编号输入状态。
    enrollNameEdit_->setEnabled(enabled); // 设置姓名输入状态。
    enrollTeamEdit_->setEnabled(enabled); // 设置班组输入状态。
    enrollRoleEdit_->setEnabled(enabled); // 设置角色输入状态。
    enrollCardEdit_->setEnabled(enabled); // 设置证件输入状态。
    enrollListTypeCombo_->setEnabled(enabled); // 设置名单下拉状态。
    enrollAccessSpin_->setEnabled(enabled); // 设置权限输入状态。
} // 结束表单启用状态设置函数。
void MainWindow::enrollCurrentFace() { // 实现当前画面人员录入函数。
    if(enrollmentChallengeActive_) { // 判断是否已有录入挑战正在进行。
        return; // 避免重复启动。
    } // 结束重复启动判断。
    const QString name = enrollNameEdit_->text().trimmed(); // 读取姓名。
    if(name.isEmpty()) { // 判断姓名是否为空。
        enrollStatusLabel_->setText(QStringLiteral("请先填写姓名")); // 更新录入状态。
        QMessageBox::warning(this, QStringLiteral("录入失败"), QStringLiteral("请先填写姓名")); // 弹出提示。
        return; // 停止录入。
    } // 结束姓名判断。
    if(lastFrame_.isNull()) { // 判断是否已有摄像头画面。
        enrollStatusLabel_->setText(QStringLiteral("还没有摄像头画面，请先点击启动")); // 更新录入状态。
        QMessageBox::warning(this, QStringLiteral("录入失败"), QStringLiteral("请先点击启动，等摄像头画面出现后再录入")); // 弹出提示。
        return; // 停止录入。
    } // 结束画面判断。
    if(!recognizer_.livenessAvailable()) { // 判断 dlib 68 点活体是否可用。
        const QString reason = QStringLiteral("当前无法进行活体动作录入，请确认 dlib 68 点模型已加载"); // 生成失败原因。
        enrollStatusLabel_->setText(reason); // 更新录入状态。
        QMessageBox::warning(this, QStringLiteral("录入失败"), reason); // 弹出失败提示。
        return; // 停止录入。
    } // 结束活体能力判断。
    Person person; // 创建人员对象。
    person.id = enrollIdEdit_->text().trimmed(); // 读取人员编号。
    if(person.id.isEmpty()) { // 判断是否需要自动生成编号。
        person.id = buildEnrollmentPersonId(); // 生成默认编号。
        enrollIdEdit_->setText(person.id); // 回填编号到界面。
    } // 结束编号判断。
    person.name = name; // 写入姓名。
    person.team = enrollTeamEdit_->text().trimmed(); // 写入班组。
    person.role = enrollRoleEdit_->text().trimmed(); // 写入角色。
    person.cardNo = enrollCardEdit_->text().trimmed(); // 写入证件号。
    person.listType = enrollListTypeCombo_->currentData().toString(); // 写入名单类型。
    person.accessLevel = enrollAccessSpin_->value(); // 写入权限等级。
    person.enabled = true; // 默认启用人员。
    pendingEnrollmentPerson_ = person; // 保存待录人员，等待活体动作通过。
    enrollmentChallenge_ = randomEnrollmentChallenge(); // 随机生成活体动作。
    enrollmentChallengeStartedAt_ = QDateTime::currentDateTime(); // 记录挑战开始时间。
    enrollmentTemplateStartedAt_ = QDateTime(); // 清空正脸模板采集开始时间。
    enrollmentChallengeActive_ = true; // 标记挑战开始。
    enrollmentStage_ = EnrollmentStage::Challenge; // 设置当前录入阶段为活体动作。
    enrollmentTemplateStableFrames_ = 0; // 重置正脸模板稳定帧计数。
    enrollmentBestTemplateFrame_ = QImage(); // 清空最佳模板帧。
    enrollmentBestTemplateFrameId_ = 0; // 清空最佳模板帧号。
    enrollmentBestTemplateQuality_ = -1.0; // 清空最佳模板质量。
    enrollmentLastFaceDetected_ = false; // 重置最近人脸检测状态。
    enrollmentLastLandmarksReady_ = false; // 重置最近关键点状态。
    enrollmentLastHeadTurnOffset_ = 0.0; // 重置最近转头偏移。
    enrollmentLastMouthOpenRatio_ = 0.0; // 重置最近张嘴比例。
    enrollmentLiveness_.reset(); // 重置活体检测状态。
    setEnrollmentInputsEnabled(false); // 挑战期间锁定表单。
    enrollButton_->setEnabled(false); // 挑战期间禁止重复点击录入。
    clearEnrollButton_->setText(QStringLiteral("取消")); // 将清空按钮临时作为取消按钮。
    const QString actionText = enrollmentChallengeText(enrollmentChallenge_); // 获取动作文本。
    enrollStatusLabel_->setText(QStringLiteral("请在 %1 秒内完成：%2。通过后请回正脸采集模板。").arg(QString::number(enrollmentChallengeTimeoutSeconds_), actionText)); // 更新挑战提示。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("开始录入活体动作：%1（%2）").arg(person.name, actionText)); // 写入日志。
    statusBar()->showMessage(QStringLiteral("请完成录入动作：%1").arg(actionText), 4000); // 显示状态栏提示。
} // 结束当前画面人员录入函数。
void MainWindow::clearEnrollmentForm() { // 实现录入表单清空函数。
    if(enrollmentChallengeActive_) { // 判断是否正在进行录入活体动作。
        finishEnrollmentChallenge(true); // 取消挑战并恢复表单。
        appendLogRow(QStringLiteral("INFO"), QStringLiteral("录入流程已取消")); // 写入取消日志。
    } // 结束活体动作取消。
    enrollIdEdit_->clear(); // 清空编号。
    enrollNameEdit_->clear(); // 清空姓名。
    enrollTeamEdit_->clear(); // 清空班组。
    enrollRoleEdit_->clear(); // 清空角色。
    enrollCardEdit_->clear(); // 清空证件号。
    enrollListTypeCombo_->setCurrentIndex(0); // 恢复白名单。
    enrollAccessSpin_->setValue(1); // 恢复默认权限。
    enrollStatusLabel_->setText(QStringLiteral("请先启动摄像头，再录入当前画面")); // 恢复提示。
} // 结束录入表单清空函数。
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
    if(enrollmentChallengeActive_) { // 判断是否正在录入活体动作。
        finishEnrollmentChallenge(true); // 停止系统时取消录入动作。
        enrollStatusLabel_->setText(QStringLiteral("系统已停止，录入已取消")); // 更新录入状态。
    } // 结束录入动作取消。
    monitor_.stop(); // 停止系统监控。
    camera_.stop(); // 停止摄像头采集。
    cloudClient_.disconnectFromCloud(); // 断开云端连接。
    accessController_.resetIndicators(); // 重置声光状态。
    updateMetrics(); // 刷新状态。
    appendLogRow(QStringLiteral("INFO"), QStringLiteral("系统已停止")); // 记录停止日志。
} // 结束系统停止函数。
