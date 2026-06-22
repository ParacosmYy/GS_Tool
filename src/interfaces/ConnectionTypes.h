/**
 * @file ConnectionTypes.h
 * @brief 连接类型与状态枚举
 */
#ifndef CONNECTION_TYPES_H
#define CONNECTION_TYPES_H
#include <QtTypes>
#ifndef CONNECTION_TYPE_DEFINED
#define CONNECTION_TYPE_DEFINED
enum class ConnectionType {
    Serial, TcpClient, TcpServer, Udp, Rtt, WebSocket, Mqtt, Tls, Ble, Can, Spi, I2c, Usb
};
#endif
#ifndef CONNECTION_STATE_DEFINED
#define CONNECTION_STATE_DEFINED
enum class ConnectionState { Disconnected, Connecting, Connected, Error };
#endif
#endif
