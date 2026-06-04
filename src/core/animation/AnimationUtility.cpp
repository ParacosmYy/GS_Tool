/**
 * @file AnimationUtility.cpp
 * @brief 动画工具集实现 — 淡入淡出/滑动/缩放/弹跳/抖动
 */

#include "core/animation/AnimationUtility.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>

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
    if (onFinished) {
        QObject::connect(anim, &QAbstractAnimation::finished,
                         anim, [onFinished]() { onFinished(); });
    }
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

// 滑动/缩放/弹跳/抖动效果(slideIn/slideOut/scaleIn/bounceIn/shake/slideOffset)
// 见 AnimationUtilityEffects.cpp
