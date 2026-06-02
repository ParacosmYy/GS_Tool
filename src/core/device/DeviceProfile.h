/**
 * @file DeviceProfile.h
 * @brief 设备配置文件结构（仅头文件）
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 定义设备配置文件结构体，包含连接参数、协议配置和快捷命令。
 */

#ifndef DEVICEPROFILE_H
#define DEVICEPROFILE_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @struct DeviceProfile
 * @brief 设备配置文件，描述一个设备的连接和通信参数
 */
struct DeviceProfile
{
    QString name;                       ///< 设备名称
    QString connectionType;             ///< 连接类型（serial/tcp/udp）
    QVariantMap connectionParams;       ///< 连接参数
    QVariantMap protocolConfig;         ///< 协议配置
    QStringList quickCommands;          ///< 快捷命令列表
    QVariantMap channels;               ///< 数据通道配置

    /**
     * @brief 创建默认设备配置
     * @return 默认配置文件
     */
    static DeviceProfile createDefault()
    {
        DeviceProfile profile;
        profile.name = QStringLiteral("New Device");
        profile.connectionType = QStringLiteral("serial");
        profile.connectionParams = {
            {QStringLiteral("port"),     QString{}},
            {QStringLiteral("baudRate"), 115200},
            {QStringLiteral("dataBits"), 8},
            {QStringLiteral("parity"),   QStringLiteral("none")},
            {QStringLiteral("stopBits"), 1}
        };
        profile.protocolConfig = {
            {QStringLiteral("frameFormat"), QStringLiteral("hex")},
            {QStringLiteral("byteOrder"),   QStringLiteral("little")}
        };
        profile.quickCommands = {};
        profile.channels = {};
        return profile;
    }

    /**
     * @brief 将设备配置序列化为 JSON 对象
     * @param profile 设备配置
     * @return QJsonObject
     */
    static QJsonObject toJson(const DeviceProfile &profile)
    {
        QJsonObject obj;
        obj[QStringLiteral("name")] = profile.name;
        obj[QStringLiteral("connectionType")] = profile.connectionType;

        QJsonObject connObj;
        for (auto it = profile.connectionParams.begin();
             it != profile.connectionParams.end(); ++it) {
            connObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj[QStringLiteral("connectionParams")] = connObj;

        QJsonObject protoObj;
        for (auto it = profile.protocolConfig.begin();
             it != profile.protocolConfig.end(); ++it) {
            protoObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj[QStringLiteral("protocolConfig")] = protoObj;

        QJsonArray cmds;
        for (const auto &cmd : profile.quickCommands) {
            cmds.append(cmd);
        }
        obj[QStringLiteral("quickCommands")] = cmds;

        return obj;
    }

    /**
     * @brief 从 JSON 对象反序列化设备配置
     * @param json JSON 对象
     * @return DeviceProfile
     */
    static DeviceProfile fromJson(const QJsonObject &json)
    {
        DeviceProfile profile;
        profile.name = json[QStringLiteral("name")].toString();
        profile.connectionType = json[QStringLiteral("connectionType")].toString();

        QJsonObject connObj = json[QStringLiteral("connectionParams")].toObject();
        for (auto it = connObj.begin(); it != connObj.end(); ++it) {
            profile.connectionParams[it.key()] = it.value().toVariant();
        }

        QJsonObject protoObj = json[QStringLiteral("protocolConfig")].toObject();
        for (auto it = protoObj.begin(); it != protoObj.end(); ++it) {
            profile.protocolConfig[it.key()] = it.value().toVariant();
        }

        QJsonArray cmds = json[QStringLiteral("quickCommands")].toArray();
        for (const auto &cmd : cmds) {
            profile.quickCommands.append(cmd.toString());
        }

        return profile;
    }
};

Q_DECLARE_METATYPE(DeviceProfile)

#endif // DEVICEPROFILE_H
