/**
 * @file TimerConstants.h
 * @brief 定时器间隔和终端默认参数 — 统一管理所有定时器间隔和终端显示默认值
 *
 * 包含定时器间隔常量和终端字体/字号默认值。
 * 定时器命名规范: k<Action><Purpose>Ms
 */
#ifndef TIMER_CONSTANTS_H
#define TIMER_CONSTANTS_H

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
    constexpr int kProgressAnimMaxMs     = 500;    ///< 进度条动画最大持续时间(500ms)
    constexpr int kBreakDurationMs       = 100;    ///< Break信号持续时间(100ms)
}

/**
 * @brief 终端默认显示参数 — 统一管理终端控件的字体和字号默认值
 *
 * 终端使用等宽字体以保持字符对齐，Consolas 是 Windows 下最常用的等宽字体。
 */
namespace TerminalDefaults {
    constexpr const char* kFontFamily = "Consolas";  ///< 终端默认字体(等宽)
    constexpr int kFontSize = 13;                     ///< 终端默认字号(像素)
}

#endif // TIMER_CONSTANTS_H
