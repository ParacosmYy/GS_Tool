/**
 * @file AppConstants.h
 * @brief 应用全局常量和枚举定义 — 应用标识、语言选项、连接/显示/终端类型枚举
 *
 * 包含应用名称、版本、组织等标识常量，语言选项，以及连接类型、显示模式、
 * 连接状态、数据方向、终端布局等全局共享枚举。
 */
#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

#include <QtTypes>

// 应用全局常量
namespace App {
    constexpr const char* APP_NAME = "EmbedDebug";
    constexpr const char* APP_VERSION = "0.1.0";
    constexpr const char* APP_ORG = "EmbedDebug";
    constexpr const char* SETTINGS_FILE = "embeddebug_settings.json";
    constexpr const char* DEFAULT_THEME = "dark_terminal";
}

// 连接类型枚举
enum class ConnectionType {
    Serial,     // 串口
    TcpClient,  // TCP客户端
    TcpServer,  // TCP服务端
    Udp,        // UDP
    Rtt,        // SEGGER RTT (通过J-Link)
    WebSocket,  // WebSocket客户端
    Mqtt,       // MQTT客户端
    Tls,        // TLS加密TCP
    Ble,        // 蓝牙低功耗
    Can,        // CAN总线(LAWICEL)
    Spi,        // SPI主机
    I2c,        // I2C主机
    Usb         // USB设备
};

// 终端显示模式
enum class DisplayMode {
    Text,       // 纯文本
    Hex,        // 十六进制
    Mixed,      // 文本+HEX混合
    Decimal     // 十进制
};

// 连接状态
enum class ConnectionState {
    Disconnected,   // 已断开
    Connecting,     // 连接中
    Connected,      // 已连接
    Error           // 错误
};

// 数据方向
enum class DataDirection {
    Rx,     // 接收
    Tx      // 发送
};

// 终端布局模式 — 控制TX/RX数据的显示方式
enum class TerminalLayout {
    Mixed,            // 混合模式: TX和RX数据在同一终端中按时间顺序显示
    SplitHorizontal,  // 左右分栏: 左侧显示RX数据，右侧显示TX数据
    SplitVertical     // 上下分栏: 上方显示RX数据，下方显示TX数据
};

// 语言选项
namespace Language {
    constexpr const char* CHINESE  = "zh_CN";
    constexpr const char* ENGLISH  = "en";
    constexpr const char* DEFAULT  = CHINESE;
}

#endif // APP_CONSTANTS_H
