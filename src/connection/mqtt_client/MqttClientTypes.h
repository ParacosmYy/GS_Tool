/**
 * @file MqttClientTypes.h
 * @brief MQTT客户端公共类型定义 — QoS枚举、消息结构体、连接参数
 *
 * 职责: 为MqttClientEngine和MqttClientPanel提供统一的类型定义，
 * 避免循环依赖。所有MQTT相关枚举和值类型集中在此文件。
 */

#ifndef MQTTCLIENTTYPES_H
#define MQTTCLIENTTYPES_H

#include <QString>
#include <QByteArray>
#include <QDateTime>

/**
 * @brief MQTT消息服务质量等级
 */
enum class MqttQos {
    QoS0,   ///< 至多一次（fire-and-forget）
    QoS1,   ///< 至少一次（需PUBACK确认）
    QoS2    ///< 恰好一次（PUBREC/PUBREL/PUBCOMP握手）
};

/**
 * @brief MQTT消息结构体，封装发布/接收的消息元数据
 */
struct MqttMessage {
    QString     topic;       ///< 消息主题（UTF-8）
    QByteArray  payload;     ///< 消息负载（二进制安全）
    MqttQos     qos;         ///< QoS等级
    bool        retained;    ///< 是否为保留消息
    QDateTime   timestamp;   ///< 接收/发送时间戳
    quint16     packetId;    ///< 报文标识符（QoS1/2时有效）
};

/**
 * @brief MQTT连接参数结构体，封装建立连接所需的全部参数
 */
struct MqttConnectionParams {
    QString  broker;       ///< 服务器地址（IP或域名）
    quint16  port;         ///< 服务器端口（默认1883 / TLS 8883）
    QString  clientId;     ///< 客户端标识符
    QString  username;     ///< 认证用户名（空则跳过认证）
    QString  password;     ///< 认证密码
    bool     useTls;       ///< 是否启用TLS加密
    int      keepAlive;    ///< KeepAlive心跳间隔（秒，默认60）
};

#endif // MQTTCLIENTTYPES_H
