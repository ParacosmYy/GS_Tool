/**
 * @file AnimationConstants.h
 * @brief 动画时长常量 — 统一管理所有 UI 动画持续时间
 *
 * 所有动画时长集中定义，便于全局调整和保持一致性。
 * 命名规范: k<Action><Purpose>Ms
 * 缓动曲线约定: 展开/滑入用 OutCubic, 收起/滑出用 InCubic
 */
#ifndef ANIMATION_CONSTANTS_H
#define ANIMATION_CONSTANTS_H

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

#endif // ANIMATION_CONSTANTS_H
