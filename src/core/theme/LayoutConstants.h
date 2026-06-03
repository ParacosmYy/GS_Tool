/**
 * @file LayoutConstants.h
 * @brief 布局间距常量 — 统一管理面板内边距、控件间距、尺寸下限和OTA面板布局
 *
 * 包含通用布局间距常量和OTA面板专用布局常量。
 * 命名规范: k<ElementType><Property>
 */
#ifndef LAYOUT_CONSTANTS_H
#define LAYOUT_CONSTANTS_H

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

#endif // LAYOUT_CONSTANTS_H
