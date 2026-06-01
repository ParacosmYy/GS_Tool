/**
 * @file Constants.h
 * @brief 全局枚举和常量定义 — 应用范围内共享的数据类型
 *
 * 包含连接类型(ConnectionType)、连接状态(ConnectionState)、数据方向(DataDirection)、
 * 终端显示模式等枚举定义。所有模块通过此文件共享类型定义，避免循环依赖。
 */
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

// 语义色和导航指示色已迁移至 ThemeManager 动态提供

// 语言选项
namespace Language {
    constexpr const char* CHINESE  = "zh_CN";
    constexpr const char* ENGLISH  = "en";
    constexpr const char* DEFAULT  = CHINESE;
}

/**
 * @brief 定时器间隔常量 — 统一管理所有定时器间隔
 *
 * 所有定时器间隔集中定义，方便统一调整和维护。
 * 命名规范: k<Action><Purpose>Ms
 */
namespace Timers {
    constexpr int kConnectTimeoutMs      = 10000;  ///< TCP连接超时(10s)
    constexpr int kStatsRefreshMs        = 500;    ///< 终端统计刷新间隔(500ms)
    constexpr int kPortPollMs            = 2000;   ///< 串口热插拔轮询间隔(2s)
    constexpr int kDataStatsRefreshMs    = 1000;   ///< 数据统计面板刷新间隔(1s)
    constexpr int kHealthCheckMs         = 5000;   ///< 连接健康检查间隔(5s)
    constexpr int kPinoutPollMs          = 200;    ///< 串口引脚状态轮询(200ms)
    constexpr int kRippleFrameMs         = 16;     ///< 涟漪动画帧间隔(~60fps)
    constexpr int kPlaybackPrecisionMs   = 1;      ///< 回放定时器精度(1ms)
    constexpr int kConnectFailedDisplayMs = 3000;  ///< 连接失败提示显示时长(3s)
    constexpr int kCompletionDelayMs     = 400;    ///< OTA完成动画延迟(400ms)
}

#endif // CONSTANTS_H
