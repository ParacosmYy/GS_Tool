/**
 * @file TimerConstants.h
 * @brief 定时器间隔和终端默认参数 - 统一管理所有定时器间隔和终端显示默认值
 *
 * 公共基础层中的定时器与终端默认值入口。
 */
#ifndef SHARED_TIMER_CONSTANTS_H
#define SHARED_TIMER_CONSTANTS_H

namespace Timers {
    constexpr int kConnectTimeoutMs = 10000;
    constexpr int kStatsRefreshMs = 500;
    constexpr int kPortPollMs = 2000;
    constexpr int kDataStatsRefreshMs = 1000;
    constexpr int kHealthCheckMs = 5000;
    constexpr int kPinoutPollMs = 200;
    constexpr int kRippleFrameMs = 16;
    constexpr int kPlaybackPrecisionMs = 1;
    constexpr int kConnectFailedDisplayMs = 3000;
    constexpr int kCompletionDelayMs = 400;
    constexpr int kProgressAnimMaxMs = 500;
    constexpr int kBreakDurationMs = 100;
}

namespace TerminalDefaults {
    constexpr const char* kFontFamily = "Consolas";
    constexpr int kFontSize = 13;
}

#endif // SHARED_TIMER_CONSTANTS_H
