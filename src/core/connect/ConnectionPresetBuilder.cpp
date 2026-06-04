/**
 * @file ConnectionPresetBuilder.cpp
 * @brief 连接默认参数构造器实现
 */

#include "core/connect/ConnectionPresetBuilder.h"

QVariantMap ConnectionPresetBuilder::build(ConnectionType type)
{
    QVariantMap params;

    if (type == ConnectionType::TcpClient) {
        params["mode"] = "client";
        params["host"] = "127.0.0.1";
        params["port"] = 8080;
    } else if (type == ConnectionType::TcpServer) {
        params["mode"] = "server";
        params["port"] = 8080;
    } else if (type == ConnectionType::Udp) {
        params["localPort"] = 8888;
        params["remoteHost"] = "127.0.0.1";
        params["remotePort"] = 8080;
    } else if (type == ConnectionType::WebSocket) {
        params["url"] = "ws://127.0.0.1:8080";
    } else if (type == ConnectionType::Mqtt) {
        params["host"] = "127.0.0.1";
        params["port"] = 1883;
    } else if (type == ConnectionType::Tls) {
        params["host"] = "127.0.0.1";
        params["port"] = 443;
    } else if (type == ConnectionType::Ble) {
        params["deviceName"] = "";
    } else if (type == ConnectionType::Can) {
        params["adapter"] = "can0";
        params["bitrate"] = 500000;
    } else if (type == ConnectionType::Spi) {
        params["device"] = "/dev/spidev0.0";
        params["speed"] = 1000000;
    } else if (type == ConnectionType::I2c) {
        params["device"] = "/dev/i2c-0";
        params["address"] = 0x50;
    } else if (type == ConnectionType::Usb) {
        params["vid"] = 0;
        params["pid"] = 0;
    }

    return params;
}
