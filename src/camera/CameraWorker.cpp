#include "camera/CameraWorker.h" // 引入摄像头工作对象声明。
#include "utils/MatImageConverter.h" // 引入图像转换工具。
#include <QDateTime> // 引入日期时间。
#include <QColor> // 引入颜色类型。
#include <QFont> // 引入字体类型。
#include <QPainter> // 引入绘图工具。
#include <QRandomGenerator> // 引入随机数。
#include <QPen> // 引入画笔类型。
#include <QRect> // 引入矩形类型。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/imgproc.hpp> // 引入 OpenCV 图像预处理函数。
#endif // 结束 OpenCV 判断。
CameraWorker::CameraWorker(QObject* parent) : QObject(parent) { // 实现构造函数。
    timer_.setTimerType(Qt::PreciseTimer); // 设置高精度定时器。
    connect(&timer_, &QTimer::timeout, this, &CameraWorker::grabFrame); // 连接定时器超时到抓帧槽。
} // 结束构造函数。
void CameraWorker::setCameraIndex(int index) { // 实现摄像头编号设置函数。
    cameraIndex_ = qMax(0, index); // 保存摄像头编号。
} // 结束设置函数。
void CameraWorker::setFrameIntervalMs(int intervalMs) { // 实现帧间隔设置函数。
    frameIntervalMs_ = qMax(10, intervalMs); // 保存采集间隔。
    if(running_) { // 判断采集是否已运行。
        timer_.start(frameIntervalMs_); // 重新启动定时器以应用新间隔。
    } // 结束运行判断。
} // 结束设置函数。
void CameraWorker::start() { // 实现采集启动函数。
    if(running_) { // 判断是否已在运行。
        return; // 直接返回避免重复启动。
    } // 结束运行判断。
    running_ = true; // 设置运行状态。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    if(!capture_.open(cameraIndex_)) { // 尝试打开真实摄像头。
        emit statusMessage(QStringLiteral("摄像头打开失败，切换到演示画面")); // 发出降级提示。
    } else { // 处理摄像头打开成功。
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, 1280); // 设置采集宽度。
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 720); // 设置采集高度。
        emit statusMessage(QStringLiteral("摄像头采集已启动，索引 %1").arg(cameraIndex_)); // 发出真实采集提示。
    } // 结束摄像头打开判断。
#else // 处理没有 OpenCV 的构建。
    emit statusMessage(QStringLiteral("未启用 OpenCV，使用演示画面")); // 发出演示采集提示。
#endif // 结束 OpenCV 判断。
    timer_.start(frameIntervalMs_); // 启动定时器。
} // 结束启动函数。
void CameraWorker::stop() { // 实现采集停止函数。
    if(!running_) { // 判断是否未运行。
        return; // 直接返回。
    } // 结束运行判断。
    running_ = false; // 重置运行状态。
    timer_.stop(); // 停止定时器。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    if(capture_.isOpened()) { // 判断真实摄像头是否打开。
        capture_.release(); // 释放摄像头设备。
    } // 结束摄像头释放判断。
#endif // 结束 OpenCV 判断。
    emit statusMessage(QStringLiteral("摄像头采集已停止")); // 发出状态消息。
} // 结束停止函数。
void CameraWorker::grabFrame() { // 实现抓帧函数。
    if(!running_) { // 判断是否正在采集。
        return; // 直接返回。
    } // 结束运行判断。
    FramePacket packet; // 创建帧数据包。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    if(capture_.isOpened()) { // 判断真实摄像头是否可用。
        cv::Mat frame; // 创建原始图像容器。
        capture_ >> frame; // 从摄像头读取一帧。
        if(!frame.empty()) { // 判断读取是否成功。
            cv::Mat resized; // 创建缩放后的图像容器。
            cv::resize(frame, resized, cv::Size(960, 540)); // 统一缩放到界面显示尺寸。
            packet.mat = resized.clone(); // 保存 OpenCV 图像。
            packet.image = MatImageConverter::matToQImage(resized); // 转换为 Qt 图像。
        } // 结束真实图像读取判断。
    } // 结束真实摄像头判断。
#endif // 结束 OpenCV 判断。
    if(!packet.isValid()) { // 判断是否需要降级画面。
        packet = buildDemoFrame(); // 构造一帧演示图像。
    } // 结束降级判断。
    packet.frameId = ++frameCounter_; // 设置帧序号。
    packet.capturedAt = QDateTime::currentDateTime(); // 设置采集时间。
    emit frameCaptured(packet); // 发出采集完成信号。
} // 结束抓帧函数。
FramePacket CameraWorker::buildDemoFrame() const { // 实现演示帧构造函数。
    FramePacket packet; // 创建数据包。
    QImage image(960, 540, QImage::Format_RGB32); // 创建固定分辨率图像。
    image.fill(QColor(18, 24, 38)); // 填充深色背景。
    QPainter painter(&image); // 创建绘图对象。
    painter.setRenderHint(QPainter::Antialiasing, true); // 打开抗锯齿。
    painter.setPen(QPen(QColor(0, 191, 255), 3)); // 设置主边框颜色。
    painter.drawRoundedRect(20, 20, image.width() - 40, image.height() - 40, 8, 8); // 绘制边框。
    painter.setPen(Qt::white); // 设置白色文字。
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 24, QFont::Bold)); // 设置标题字体。
    painter.drawText(QRect(40, 40, image.width() - 80, 60), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("智慧工地人脸识别实名制考勤终端")); // 绘制标题。
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 16)); // 设置说明字体。
    painter.drawText(QRect(40, 110, 500, 30), QStringLiteral("设备编号：SITE-GATE-001")); // 绘制设备编号。
    painter.drawText(QRect(40, 150, 500, 30), QStringLiteral("状态：摄像头 / OpenCV / dlib 运行中")); // 绘制状态文本。
    painter.drawText(QRect(40, 190, 520, 30), QStringLiteral("说明：当前为无摄像头环境下的演示画面")); // 绘制说明文本。
    QRect faceRect(610, 90, 250, 300); // 定义人脸示意区域。
    painter.setBrush(QColor(255, 224, 189)); // 设置肤色填充。
    painter.setPen(QPen(QColor(255, 224, 189), 2)); // 设置人脸边框。
    painter.drawEllipse(faceRect); // 绘制脸部椭圆。
    painter.setBrush(Qt::black); // 设置眼睛填充颜色。
    painter.drawEllipse(QRect(faceRect.left() + 55, faceRect.top() + 90, 28, 28)); // 绘制左眼。
    painter.drawEllipse(QRect(faceRect.right() - 83, faceRect.top() + 90, 28, 28)); // 绘制右眼。
    painter.setPen(QPen(Qt::black, 4)); // 设置嘴巴画笔。
    painter.drawArc(QRect(faceRect.left() + 60, faceRect.top() + 150, 130, 80), 0, -180 * 16); // 绘制微笑嘴角。
    painter.setPen(QPen(QColor(0, 255, 160), 2)); // 设置信息框边线。
    painter.setBrush(QColor(0, 255, 160, 40)); // 设置透明绿色填充。
    painter.drawRoundedRect(QRect(40, 260, 520, 180), 8, 8); // 绘制状态面板。
    painter.setPen(Qt::white); // 设置文字颜色。
    painter.drawText(QRect(60, 280, 480, 140), Qt::TextWordWrap, QStringLiteral("模块说明：\n1. 摄像头采集线程\n2. 人脸检测与活体识别\n3. SQLite 离线考勤\n4. MQTT/HTTP 云端同步\n5. 闸机、指示灯、蜂鸣器联动")); // 绘制功能说明。
    painter.end(); // 结束绘图。
    packet.image = image; // 保存图像到数据包。
    return packet; // 返回演示帧。
} // 结束演示帧构造函数。
