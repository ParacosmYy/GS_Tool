/**
 * @file LayoutConstants.h
 * @brief 布局间距常量 - 统一管理面板内边距、控件间距、尺寸下限和 OTA 面板布局
 *
 * 公共基础层中的布局常量入口。
 * 所有复用的间距、宽度、高度、阈值优先在这里统一定义。
 */
#ifndef SHARED_LAYOUT_CONSTANTS_H
#define SHARED_LAYOUT_CONSTANTS_H

namespace Layout {
    constexpr int kPanelPadding = 12;
    constexpr int kPanelSpacing = 12;
    constexpr int kToolbarPadding = 8;
    constexpr int kToolbarSpacing = 4;
    constexpr int kGroupSpacing = 8;
    constexpr int kControlSpacing = 6;
    constexpr int kMinButtonWidth = 60;
    constexpr int kMinButtonHeight = 28;
    constexpr int kComboFixedWidth = 90;
    constexpr int kInputHeight = 32;
    constexpr int kBrowseBtnWidth = 80;
    constexpr int kSendBtnWidth = 70;
    constexpr int kNewlineComboWidth = 70;
    constexpr int kPortComboMinWidth = 150;
    constexpr int kSearchInputMinWidth = 240;
    constexpr int kSearchResultMinWidth = 80;
    constexpr int kSearchBarHeight = 36;
    constexpr int kLabelFixedWidth = 70;
    constexpr int kSliderValueWidth = 36;
    constexpr int kNavTreeMinWidth = 180;
    constexpr int kNavTreeMaxWidth = 280;
    constexpr int kConnectBtnMinHeight = 36;
    constexpr int kQuickCmdBtnMaxWidth = 160;
}

namespace OtaLayout {
    constexpr int kHistoryColTime = 150;
    constexpr int kHistoryColFileName = 160;
    constexpr int kHistoryColProtocol = 80;
    constexpr int kHistoryColSize = 80;
    constexpr int kHistoryColDuration = 80;
    constexpr int kProgressBarHeight = 24;
    constexpr int kLogViewMaxHeight = 160;
    constexpr int kFieldTableMinHeight = 120;
    constexpr int kPreviewMinHeight = 60;
    constexpr int kTableBtnWidth = 60;
}

#endif // SHARED_LAYOUT_CONSTANTS_H
