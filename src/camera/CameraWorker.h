#pragma once // 防止头文件被重复包含。
#include "camera/FramePacket.h" // 引入视频帧数据包。
#include <QImage> // 引入 Qt 图像类型。
#include <QObject> // 引入 Qt 对象基类。
#include <QTimer> // 引入定时器。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/videoio.hpp> // 引入 OpenCV 摄像头采集类型。
#endif // 结束 OpenCV 判断。
#ifdef SMARTSITE_HAS_QTMULTIMEDIA // 判断是否启用 Qt Multimedia 摄像头采集。
#include <QCamera> // 引入 Qt 摄像头对象。
#include <QMediaCaptureSession> // 引入 Qt 采集会话。
#include <QVideoFrame> // 引入 Qt 视频帧。
#include <QVideoSink> // 引入 Qt 视频帧接收器。
#endif // 结束 Qt Multimedia 判断。
class CameraWorker : public QObject { // 定义摄像头采集工作线程对象。
    Q_OBJECT // 启用 Qt 元对象系统。
public: // 声明公共接口。
    explicit CameraWorker(QObject* parent = nullptr); // 构造采集对象。
    void setCameraIndex(int index); // 设置摄像头编号。
    void setFrameIntervalMs(int intervalMs); // 设置采集间隔。
public slots: // 声明槽函数。
    void start(); // 开始采集。
    void stop(); // 停止采集。
private slots: // 声明私有槽函数。
    void grabFrame(); // 抓取一帧图像。
#ifdef SMARTSITE_HAS_QTMULTIMEDIA // 判断是否启用 Qt Multimedia 摄像头采集。
    void onVideoFrameChanged(const QVideoFrame& frame); // 接收 Qt 摄像头视频帧。
#endif // 结束 Qt Multimedia 判断。
signals: // 声明信号。
    void frameCaptured(const FramePacket& packet); // 发出采集到的图像帧。
    void statusMessage(const QString& message); // 发出状态消息。
private: // 声明私有成员。
    FramePacket buildDemoFrame() const; // 构造演示帧。
#ifdef SMARTSITE_HAS_QTMULTIMEDIA // 判断是否启用 Qt Multimedia 摄像头采集。
    bool startQtCamera(); // 启动 Qt Multimedia 摄像头。
    void stopQtCamera(); // 停止 Qt Multimedia 摄像头。
#endif // 结束 Qt Multimedia 判断。
    bool running_ = false; // 保存采集运行状态。
    int cameraIndex_ = 0; // 保存摄像头编号。
    int frameIntervalMs_ = 33; // 保存采集间隔。
    qint64 frameCounter_ = 0; // 保存帧计数。
    QTimer timer_; // 保存帧定时器。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    cv::VideoCapture capture_; // 保存真实摄像头采集对象。
#endif // 结束 OpenCV 判断。
#ifdef SMARTSITE_HAS_QTMULTIMEDIA // 判断是否启用 Qt Multimedia 摄像头采集。
    QCamera* qtCamera_ = nullptr; // 保存 Qt 摄像头对象。
    QMediaCaptureSession qtCaptureSession_; // 保存 Qt 采集会话。
    QVideoSink qtVideoSink_; // 保存 Qt 视频帧接收器。
    QImage latestQtFrame_; // 保存最近一帧 Qt 摄像头图像。
    bool qtCameraActive_ = false; // 保存 Qt 摄像头是否正在采集。
#endif // 结束 Qt Multimedia 判断。
}; // 结束摄像头工作对象定义。
