#include "ui/MainWindow.h" // 引入主窗口。
#include <QApplication> // 引入 Qt 应用对象。
#include <QDateTime> // 引入日期时间。
#include <QFile> // 引入文件。
#include <QJsonDocument> // 引入 JSON 文档。
#include <QJsonObject> // 引入 JSON 对象。
#include <QStandardPaths> // 引入标准路径。
#include <QDir> // 引入目录处理。
static void ensureAppData() { // 定义应用数据目录初始化函数。
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); // 获取应用数据目录。
    QDir().mkpath(baseDir); // 创建根数据目录。
    QDir().mkpath(baseDir + QStringLiteral("/data")); // 创建数据目录。
    QDir().mkpath(baseDir + QStringLiteral("/data/captures")); // 创建抓拍目录。
    QDir().mkpath(baseDir + QStringLiteral("/data/logs")); // 创建日志目录。
} // 结束应用数据目录初始化函数。
int main(int argc, char* argv[]) { // 定义程序入口。
    QApplication app(argc, argv); // 创建 Qt 应用对象。
    QApplication::setApplicationName(QStringLiteral("SmartSiteFaceAttendance")); // 设置应用名称。
    QApplication::setOrganizationName(QStringLiteral("Pepper-pea")); // 设置组织名称。
    ensureAppData(); // 确保应用数据目录存在。
    MainWindow window; // 创建主窗口。
    window.show(); // 显示主窗口。
    return app.exec(); // 进入事件循环。
} // 结束程序入口。
