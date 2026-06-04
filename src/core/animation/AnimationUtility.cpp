/**
 * @file AnimationUtility.cpp
 * @brief 动画工具集实现 — 淡入淡出/滑动/缩放/弹跳/抖动 + 统计
 */

#include "core/animation/AnimationUtility.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>

/// 静态统计实例
AnimationUtility::Stats AnimationUtility::s_stats;
quint64 AnimationUtility::s_durationSumMs = 0;
quint64 AnimationUtility::s_durationCount = 0;

/** @brief 获取统计计数器只读引用 @return 当前统计快照 */
const AnimationUtility::Stats& AnimationUtility::stats()
{
    return s_stats;
}

/** @brief 重置所有统计计数器为零 */
void AnimationUtility::resetStatistics()
{
    s_stats = Stats{};
    s_durationSumMs = 0;
    s_durationCount = 0;
}

/** @brief 确保控件有透明度特效 @param widget 目标控件 @return 透明度特效指针 */
QGraphicsOpacityEffect* AnimationUtility::ensureOpacityEffect(QWidget* widget)
{
    if (!widget) return nullptr;
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    return effect;
}

/** @brief 淡入动画 @param widget 目标控件 @param durationMs 持续时间 @param curve 缓动曲线 @return 动画指针 */
QPropertyAnimation* AnimationUtility::fadeIn(QWidget* widget,
                                               int durationMs,
                                               QEasingCurve curve)
{
    auto* effect = ensureOpacityEffect(widget);
    if (!effect) return nullptr;
    effect->setOpacity(0.0);

    auto* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(durationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(curve);

    /* 统计: 创建 + 完成时记录时长 */
    ++s_stats.totalAnimationsCreated;
    if (curve != QEasingCurve::OutCubic) ++s_stats.totalEasingChanges;
    QObject::connect(anim, &QAbstractAnimation::finished, anim, [dur = durationMs]() {
        ++s_stats.totalAnimationsCompleted;
        s_durationSumMs += dur;
        ++s_durationCount;
        s_stats.avgDurationMs = s_durationCount > 0
            ? static_cast<double>(s_durationSumMs) / static_cast<double>(s_durationCount)
            : 0.0;
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

/** @brief 淡出动画 @param widget 目标控件 @param durationMs 持续时间 @param curve 缓动曲线 @param onFinished 完成回调 @return 动画指针 */
QPropertyAnimation* AnimationUtility::fadeOut(QWidget* widget,
                                                int durationMs,
                                                QEasingCurve curve,
                                                std::function<void()> onFinished)
{
    auto* effect = ensureOpacityEffect(widget);
    if (!effect) return nullptr;

    auto* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(durationMs);
    anim->setStartValue(effect->opacity());
    anim->setEndValue(0.0);
    anim->setEasingCurve(curve);

    /* 统计: 创建 + 缓动变更 */
    ++s_stats.totalAnimationsCreated;
    if (curve != QEasingCurve::InCubic) ++s_stats.totalEasingChanges;

    /* 连接 finished 信号: 先更新统计，再调用用户回调 */
    QObject::connect(anim, &QAbstractAnimation::finished, anim,
        [onFinished, durationMs]() {
            ++s_stats.totalAnimationsCompleted;
            s_durationSumMs += durationMs;
            ++s_durationCount;
            s_stats.avgDurationMs = s_durationCount > 0
                ? static_cast<double>(s_durationSumMs) / static_cast<double>(s_durationCount)
                : 0.0;
            if (onFinished) onFinished();
        });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

// 滑动/缩放/弹跳/抖动效果(slideIn/slideOut/scaleIn/bounceIn/shake/slideOffset)
// 见 AnimationUtilityEffects.cpp
