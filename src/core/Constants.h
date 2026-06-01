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
#include <QStringList>
#include <QtTypes>

/**
 * @brief 标准串口波特率常量 — 统一管理波特率列表和默认选项
 *
 * 涵盖从110到1000000的工业标准波特率值，升序排列。
 * 列表供 SerialConfigPanelUI 的波特率下拉框使用。
 */
namespace BaudRates {
    /// 标准串口波特率列表(升序), 涵盖从110到1000000的工业标准值
    inline const QStringList kStandardRates = {
        "110","300","600","1200","2400","4800","9600","14400",
        "19200","28800","38400","57600","56000","115200",
        "128000","230400","256000","460800","921600","1000000"
    };
    constexpr int kDefaultBaudIndex = 12; ///< 默认选中115200(列表第13项)
}

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

/**
 * @brief 动画时长常量 — 统一管理所有 UI 动画持续时间(CLAUDE.md §6.5)
 *
 * 所有动画时长集中定义，便于全局调整和保持一致性。
 * 命名规范: k<Action><Purpose>Ms
 * 缓动曲线约定: 展开/滑入用 OutCubic, 收起/滑出用 InCubic
 */
namespace Animations {
    constexpr int kPanelSlideInMs    = 250;   ///< 面板滑入动画时长(OutCubic)
    constexpr int kPanelSlideOutMs   = 200;   ///< 面板滑出动画时长(InCubic)
    constexpr int kSearchExpandMs    = 200;   ///< 搜索栏展开动画时长(OutCubic)
    constexpr int kSearchCollapseMs  = 150;   ///< 搜索栏收起动画时长(InCubic)
    constexpr int kThemeFadeMs       = 300;   ///< 主题切换淡入淡出动画时长(InOutCubic)
    constexpr int kBreatheCycleMs    = 1500;  ///< 连接状态呼吸动画周期(InOutSine)
    constexpr int kConnPulseMs      = 1500;  ///< 连接脉冲动画周期
    constexpr int kToastPopMs       = 300;    ///< 通知弹出动画时长(OutBack)
    constexpr int kToastDismissMs   = 250;    ///< 通知消失动画时长(InCubic)
    constexpr int kNavIndicatorMs   = 250;    ///< 导航指示线滑动时长(OutCubic)
    constexpr int kButtonHoverMs    = 200;    ///< 按钮悬浮动画时长(OutCubic)
    constexpr int kButtonPressMs    = 100;    ///< 按钮按下动画时长
}

/**
 * @brief 布局间距常量 — 统一管理面板内边距、控件间距、尺寸下限(CLAUDE.md §6.3)
 *
 * 所有布局数值集中定义，确保全局一致的视觉节奏。
 * 命名规范: k<ElementType><Property>
 */
namespace Layout {
    constexpr int kPanelPadding     = 12;  ///< 面板内边距
    constexpr int kPanelSpacing     = 12;  ///< 面板内控件间距
    constexpr int kToolbarPadding   = 8;   ///< 工具栏内边距
    constexpr int kToolbarSpacing   = 4;   ///< 工具栏内控件间距
    constexpr int kGroupSpacing     = 8;   ///< 分组间距
    constexpr int kControlSpacing   = 6;   ///< 同行控件间距
    constexpr int kMinButtonWidth   = 60;  ///< 按钮最小宽度
    constexpr int kMinButtonHeight  = 28;  ///< 按钮最小高度(CLAUDE.md §6.3)
    constexpr int kComboFixedWidth  = 90;  ///< 下拉框固定宽度
    constexpr int kInputHeight      = 32;  ///< 输入框高度(CLAUDE.md §6.3)
    constexpr int kBrowseBtnWidth   = 80;  ///< 文件浏览按钮固定宽度
    constexpr int kSendBtnWidth     = 70;  ///< 发送按钮固定宽度
    constexpr int kNewlineComboWidth = 70; ///< 换行符下拉框固定宽度
    constexpr int kPortComboMinWidth = 150;///< 串口下拉框最小宽度
    constexpr int kSearchInputMinWidth = 240;///< 搜索输入框最小宽度
    constexpr int kSearchResultMinWidth = 80;///< 搜索结果标签最小宽度
    constexpr int kSearchBarHeight   = 36; ///< 搜索栏高度(CLAUDE.md §6.3)
    constexpr int kLabelFixedWidth   = 70; ///< 设置弹窗标签固定宽度
    constexpr int kSliderValueWidth  = 36; ///< 滑块数值标签固定宽度
    constexpr int kNavTreeMinWidth   = 180;///< 导航树最小宽度(CLAUDE.md §6.3)
    constexpr int kNavTreeMaxWidth   = 280;///< 导航树最大宽度(CLAUDE.md §6.3)
    constexpr int kConnectBtnMinHeight = 36;///< 连接按钮最小高度
    constexpr int kQuickCmdBtnMaxWidth = 160;///< 快捷指令按钮最大宽度
}

/**
 * @brief OTA面板布局常量 — 统一管理OTA面板中的固定尺寸和列宽
 *
 * 集中定义OTA历史表格列宽、进度条高度、日志高度等，避免硬编码。
 */
namespace OtaLayout {
    constexpr int kHistoryColTime     = 150;///< 历史表格-时间列宽
    constexpr int kHistoryColFileName = 160;///< 历史表格-文件名列宽
    constexpr int kHistoryColProtocol = 80; ///< 历史表格-协议列宽
    constexpr int kHistoryColSize     = 80; ///< 历史表格-大小列宽
    constexpr int kHistoryColDuration = 80; ///< 历史表格-耗时列宽
    constexpr int kProgressBarHeight  = 24; ///< OTA进度条固定高度
    constexpr int kLogViewMaxHeight   = 160;///< OTA日志视图最大高度
    constexpr int kFieldTableMinHeight = 120;///< 帧编辑器字段表格最小高度
    constexpr int kPreviewMinHeight   = 60; ///< 帧编辑器预览区最小高度
    constexpr int kTableBtnWidth      = 60; ///< 表格操作按钮(上移/下移/清除)固定宽度
}

/**
 * @brief 网络连接默认参数 — 统一管理TCP/UDP连接的默认主机地址和端口
 *
 * 集中定义默认网络连接参数，避免在各Connection类中硬编码。
 */
namespace ConnectionDefaults {
    constexpr const char* kDefaultHost = "127.0.0.1";  ///< 默认TCP/UDP主机地址
    constexpr quint16 kDefaultPort = 8080;              ///< 默认TCP/UDP端口
}

#endif // CONSTANTS_H
