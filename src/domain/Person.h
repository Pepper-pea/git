#pragma once // 防止头文件被重复包含。
#include <QJsonArray> // 引入 JSON 数组类型用于保存特征向量。
#include <QJsonObject> // 引入 JSON 对象类型用于人员序列化。
#include <QMetaType> // 引入 Qt 元类型声明支持。
#include <QString> // 引入 Qt 字符串类型。
#include <QVector> // 引入 Qt 动态数组类型。
struct Person { // 定义实名制人员信息结构。
    QString id; // 保存人员唯一编号。
    QString name; // 保存人员姓名。
    QString team; // 保存所属班组。
    QString role; // 保存工种或管理角色。
    QString cardNo; // 保存身份证号或实名制卡号。
    QString listType; // 保存名单类型，white 表示白名单，black 表示黑名单。
    int accessLevel = 0; // 保存通行权限等级。
    bool enabled = true; // 保存人员是否启用。
    QVector<double> feature; // 保存人脸特征向量。
    QJsonObject toJson() const { // 将人员对象转成 JSON 对象。
        QJsonObject object; // 创建 JSON 对象容器。
        object.insert(QStringLiteral("id"), id); // 写入人员编号。
        object.insert(QStringLiteral("name"), name); // 写入人员姓名。
        object.insert(QStringLiteral("team"), team); // 写入所属班组。
        object.insert(QStringLiteral("role"), role); // 写入人员角色。
        object.insert(QStringLiteral("cardNo"), cardNo); // 写入证件编号。
        object.insert(QStringLiteral("listType"), listType); // 写入名单类型。
        object.insert(QStringLiteral("accessLevel"), accessLevel); // 写入通行等级。
        object.insert(QStringLiteral("enabled"), enabled); // 写入启用状态。
        QJsonArray featureArray; // 创建特征数组。
        for(double value : feature) { // 遍历每一个特征值。
            featureArray.append(value); // 将特征值追加到数组。
        } // 结束特征遍历。
        object.insert(QStringLiteral("feature"), featureArray); // 写入特征数组。
        return object; // 返回序列化后的 JSON 对象。
    } // 结束人员序列化函数。
    static Person fromJson(const QJsonObject& object) { // 从 JSON 对象恢复人员信息。
        Person person; // 创建人员对象。
        person.id = object.value(QStringLiteral("id")).toString(); // 读取人员编号。
        person.name = object.value(QStringLiteral("name")).toString(); // 读取人员姓名。
        person.team = object.value(QStringLiteral("team")).toString(); // 读取所属班组。
        person.role = object.value(QStringLiteral("role")).toString(); // 读取人员角色。
        person.cardNo = object.value(QStringLiteral("cardNo")).toString(); // 读取证件编号。
        person.listType = object.value(QStringLiteral("listType")).toString(QStringLiteral("white")); // 读取名单类型。
        person.accessLevel = object.value(QStringLiteral("accessLevel")).toInt(); // 读取通行等级。
        person.enabled = object.value(QStringLiteral("enabled")).toBool(true); // 读取启用状态。
        const QJsonArray featureArray = object.value(QStringLiteral("feature")).toArray(); // 读取特征数组。
        for(const QJsonValue& value : featureArray) { // 遍历 JSON 特征值。
            person.feature.append(value.toDouble()); // 追加到人员特征向量。
        } // 结束特征读取。
        return person; // 返回恢复后的人员对象。
    } // 结束反序列化函数。
}; // 结束人员结构定义。
Q_DECLARE_METATYPE(Person) // 将人员结构声明为 Qt 元类型。
