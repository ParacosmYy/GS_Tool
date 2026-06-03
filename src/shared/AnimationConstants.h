/**
 * @file AnimationConstants.h
 * @brief 动画时长常量 - 统一管理所有 UI 动画持续时间
 *
 * 公共基础层中的动画常量入口。
 * 这些值应尽量保持稳定，供多个 UI 组件共享。
 */
#ifndef SHARED_ANIMATION_CONSTANTS_H
#define SHARED_ANIMATION_CONSTANTS_H

namespace Animations {
    constexpr int kPanelSlideInMs = 250;
    constexpr int kPanelSlideOutMs = 200;
    constexpr int kSearchExpandMs = 200;
    constexpr int kSearchCollapseMs = 150;
    constexpr int kThemeFadeMs = 300;
    constexpr int kBreatheCycleMs = 1500;
    constexpr int kConnPulseMs = 1500;
    constexpr int kToastPopMs = 300;
    constexpr int kToastDismissMs = 250;
    constexpr int kNavIndicatorMs = 250;
    constexpr int kButtonHoverMs = 200;
    constexpr int kButtonPressMs = 100;
    constexpr int kBreakpointTransitionMs = 250; ///< 断点切换过渡动画时长
}

#endif // SHARED_ANIMATION_CONSTANTS_H
