#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

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
    Rtt         // SEGGER RTT (通过J-Link)
};

// 终端显示模式
enum class DisplayMode {
    Text,       // 纯文本
    Hex,        // 十六进制
    Mixed       // 文本+HEX混合
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

#endif // CONSTANTS_H
