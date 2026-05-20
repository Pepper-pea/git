#pragma once // 防止头文件被重复包含。
#include "business/AttendanceManager.h" // 引入考勤管理器。
#include "camera/CameraWorker.h" // 引入摄像头工作对象。
#include "comm/CloudClient.h" // 引入云端客户端。
#include "domain/Person.h" // 引入人员结构。
#include "domain/RecognizedFaceRecord.h" // 引入识别人脸落库记录。
#include "hardware/AccessController.h" // 引入硬件控制器。
#include "monitor/SystemMonitor.h" // 引入系统监控器。
#include "vision/FaceRecognizer.h" // 引入识别器。
#include <QComboBox> // 引入下拉框控件。
#include <QDateTime> // 引入日期时间类型。
#include <QImage> // 引入图像类型。
#include <QLabel> // 引入标签控件。
#include <QLineEdit> // 引入单行输入控件。
#include <QListWidget> // 引入列表控件。
#include <QMainWindow> // 引入主窗口基类。
#include <QPlainTextEdit> // 引入纯文本编辑框。
#include <QPushButton> // 引入按钮控件。
#include <QSpinBox> // 引入数字输入控件。
#include <QTableWidget> // 引入表格控件。
class MainWindow : public QMainWindow { // 定义主界面窗口。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit MainWindow(QWidget* parent = nullptr); // 构造主窗口。
    ~MainWindow() override; // 析构主窗口。
private slots: // 声明界面槽函数。
    void onFrameCaptured(const FramePacket& packet); // 处理摄像头帧。
    void onRecognitionReady(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath); // 处理识别结果。
    void onStatusMessage(const QString& message); // 处理状态消息。
    void onLogMessage(const QString& level, const QString& message); // 处理日志消息。
    void onRecordCreated(const AttendanceRecord& record); // 处理考勤记录生成。
    void onOnlineChanged(bool online); // 处理在线状态变化。
    void onWatchdogTriggered(const QString& reason); // 处理看门狗告警。
    void enrollCurrentFace(); // 录入当前画面中的人脸。
    void clearEnrollmentForm(); // 清空录入表单。
    void startSystem(); // 启动整套系统。
    void stopSystem(); // 停止整套系统。
private: // 声明私有工具函数。
    void buildUi(); // 构建界面。
    void loadConfig(); // 读取配置。
    void refreshPersons(); // 刷新人员列表。
    void refreshRecords(); // 刷新考勤记录。
    void appendLogRow(const QString& level, const QString& message); // 追加日志到界面。
    void updateMetrics(); // 刷新统计信息。
    QString buildEnrollmentPersonId() const; // 生成默认人员编号。
    QString saveRecognitionSnapshot(const FramePacket& packet) const; // 保存本次识别抓拍图片。
    RecognizedFaceRecord buildRecognizedFaceRecord(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath, bool accessAllowed, const QString& decision) const; // 构建识别落库记录。
    void handleRecognition(const RecognitionResult& result, qint64 frameId, const QString& snapshotPath); // 处理识别逻辑。
    void processEnrollmentChallenge(const QImage& image, qint64 frameId); // 处理录入活体动作挑战。
    void finishEnrollmentChallenge(bool clearPendingPerson); // 结束录入活体动作挑战。
    void setEnrollmentInputsEnabled(bool enabled); // 设置录入表单是否可编辑。
    QLabel* videoLabel_ = nullptr; // 保存视频显示控件。
    QLabel* statusLabel_ = nullptr; // 保存状态显示控件。
    QLabel* gateLabel_ = nullptr; // 保存闸机状态控件。
    QLabel* cloudLabel_ = nullptr; // 保存云端状态控件。
    QListWidget* logList_ = nullptr; // 保存日志列表控件。
    QTableWidget* recordTable_ = nullptr; // 保存考勤记录表格。
    QTableWidget* personTable_ = nullptr; // 保存人员表格。
    QLineEdit* enrollIdEdit_ = nullptr; // 保存录入人员编号输入框。
    QLineEdit* enrollNameEdit_ = nullptr; // 保存录入人员姓名输入框。
    QLineEdit* enrollTeamEdit_ = nullptr; // 保存录入人员班组输入框。
    QLineEdit* enrollRoleEdit_ = nullptr; // 保存录入人员角色输入框。
    QLineEdit* enrollCardEdit_ = nullptr; // 保存录入证件号输入框。
    QComboBox* enrollListTypeCombo_ = nullptr; // 保存录入名单类型下拉框。
    QSpinBox* enrollAccessSpin_ = nullptr; // 保存录入通行权限输入框。
    QLabel* enrollStatusLabel_ = nullptr; // 保存录入状态提示。
    QPushButton* enrollButton_ = nullptr; // 保存录入按钮。
    QPushButton* clearEnrollButton_ = nullptr; // 保存清空录入表单按钮。
    QPushButton* startButton_ = nullptr; // 保存启动按钮。
    QPushButton* stopButton_ = nullptr; // 保存停止按钮。
    QPushButton* refreshButton_ = nullptr; // 保存刷新按钮。
    CameraWorker camera_; // 保存摄像头采集对象。
    FaceRecognizer recognizer_; // 保存识别对象。
    LivenessDetector enrollmentLiveness_; // 保存录入专用活体检测器。
    AttendanceManager attendance_; // 保存考勤管理器。
    AccessController accessController_; // 保存硬件控制器。
    CloudClient cloudClient_; // 保存云端客户端。
    SystemMonitor monitor_; // 保存系统监控器。
    bool systemRunning_ = false; // 保存系统运行状态。
    int frameCounter_ = 0; // 保存帧计数。
    int recognitionInterval_ = 8; // 保存识别间隔帧数。
    qint64 lastFrameId_ = 0; // 保存最近一帧序号。
    QString deviceId_ = QStringLiteral("SITE-GATE-001"); // 保存设备编号。
    QString captureDir_; // 保存本地抓拍目录。
    QImage lastFrame_; // 保存最近一帧图像用于人员录入。
    Person pendingEnrollmentPerson_; // 保存等待活体动作通过后录入的人员。
    EnrollmentChallengeType enrollmentChallenge_ = EnrollmentChallengeType::Blink; // 保存当前录入活体动作。
    QDateTime enrollmentChallengeStartedAt_; // 保存录入活体动作开始时间。
    bool enrollmentChallengeActive_ = false; // 保存是否正在进行录入活体动作。
    int enrollmentChallengeTimeoutSeconds_ = 15; // 保存录入活体动作超时时间。
    bool enrollmentLastFaceDetected_ = false; // 保存录入挑战最近是否检测到人脸。
    bool enrollmentLastLandmarksReady_ = false; // 保存录入挑战最近是否有 68 点关键点。
    double enrollmentLastHeadTurnOffset_ = 0.0; // 保存录入挑战最近一次转头偏移。
    double enrollmentTurnOffsetThreshold_ = 0.18; // 保存录入挑战转头阈值。
}; // 结束主窗口定义。
