/**
 * @file AnimationUtilityEffects.cpp
 * @brief 动画工具集 — 滑动/缩放/弹跳/抖动效果实现
 *
 * 从 AnimationUtility.cpp 拆分而来，包含:
 *   - slideOffset(): 滑动偏移量计算
 *   - slideIn() / slideOut(): 滑动进入/退出动画
 *   - scaleIn(): 缩放弹入效果(几何+透明度并行)
 *   - bounceIn(): 弹性滑入效果
 *   - shake(): 水平抖动效果
 *
 * 淡入淡出(fadeIn/fadeOut)和辅助方法(ensureOpacityEffect)
 * 见 AnimationUtility.cpp。
 */

#include "core/animation/AnimationUtility.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>

/** @brief 计算滑动偏移量 @param direction 滑动方向 @param widget 目标控件 @return 偏移坐标 */
QPoint AnimationUtility::slideOffset(SlideDirection direction,
                                       const QWidget* widget)
{
    int w = widget ? widget->width() : 400;
    int h = widget ? widget->height() : 300;
    switch (direction) {
    case SlideDirection::FromLeft:   return QPoint(-w, 0);
    case SlideDirection::FromRight:  return QPoint(w, 0);
    case SlideDirection::FromTop:    return QPoint(0, -h);
    case SlideDirection::FromBottom: return QPoint(0, h);
    }
    return QPoint(0, 0);
}

/** @brief 滑入动画 @param widget 目标控件 @param direction 滑动方向 @param durationMs 持续时间 @param curve 缓动曲线 @return 动画指针 */
QPropertyAnimation* AnimationUtility::slideIn(QWidget* widget,
                                                SlideDirection direction,
                                                int durationMs,
                                                QEasingCurve curve)
{
    if (!widget) return nullptr;

    QPoint target = widget->pos();
    QPoint start = target + slideOffset(direction, widget);
    widget->move(start);

    auto* anim = new QPropertyAnimation(widget, "pos");
    anim->setDuration(durationMs);
    anim->setStartValue(start);
    anim->setEndValue(target);
    anim->setEasingCurve(curve);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

/** @brief 滑出动画 @param widget 目标控件 @param direction 滑动方向 @param durationMs 持续时间 @param curve 缓动曲线 @param onFinished 完成回调 @return 动画指针 */
QPropertyAnimation* AnimationUtility::slideOut(QWidget* widget,
                                                 SlideDirection direction,
                                                 int durationMs,
                                                 QEasingCurve curve,
                                                 std::function<void()> onFinished)
{
    if (!widget) return nullptr;

    QPoint start = widget->pos();
    QPoint end = start + slideOffset(direction, widget);

    auto* anim = new QPropertyAnimation(widget, "pos");
    anim->setDuration(durationMs);
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(curve);
    if (onFinished) {
        QObject::connect(anim, &QAbstractAnimation::finished,
                         anim, [onFinished]() { onFinished(); });
    }
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

/** @brief 缩放进入动画(几何+透明度并行) @param widget 目标控件 @param durationMs 持续时间 */
void AnimationUtility::scaleIn(QWidget* widget, int durationMs)
{
    if (!widget) return;

    auto* effect = ensureOpacityEffect(widget);
    if (effect) effect->setOpacity(0.0);

    // 缩放模拟: 通过geometry动画(从中心缩小到正常)
    QRect normal = widget->geometry();
    QRect small(normal.center().x() - normal.width() / 4,
                normal.center().y() - normal.height() / 4,
                normal.width() / 2,
                normal.height() / 2);

    auto* geoAnim = new QPropertyAnimation(widget, "geometry");
    geoAnim->setDuration(durationMs);
    geoAnim->setStartValue(small);
    geoAnim->setEndValue(normal);
    geoAnim->setEasingCurve(QEasingCurve::OutBack);

    QPropertyAnimation* fadeAnim = nullptr;
    if (effect) {
        fadeAnim = new QPropertyAnimation(effect, "opacity");
        fadeAnim->setDuration(durationMs);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    }

    auto* group = new QParallelAnimationGroup(widget);
    group->addAnimation(geoAnim);
    if (fadeAnim) group->addAnimation(fadeAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 弹跳进入动画 @param widget 目标控件 @param direction 滑动方向 @param durationMs 持续时间 @return 动画指针 */
QPropertyAnimation* AnimationUtility::bounceIn(QWidget* widget,
                                                  SlideDirection direction,
                                                  int durationMs)
{
    if (!widget) return nullptr;

    QPoint target = widget->pos();
    QPoint start = target + slideOffset(direction, widget);
    widget->move(start);

    auto* anim = new QPropertyAnimation(widget, "pos");
    anim->setDuration(durationMs);
    anim->setStartValue(start);
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

/** @brief 抖动动画(水平往返) @param widget 目标控件 @param amplitude 振幅像素 @param count 往返次数 */
void AnimationUtility::shake(QWidget* widget, int amplitude, int count)
{
    if (!widget) return;

    auto* group = new QSequentialAnimationGroup(widget);
    QPoint origin = widget->pos();

    for (int i = 0; i < count; ++i) {
        auto* right = new QPropertyAnimation(widget, "pos");
        right->setDuration(50);
        right->setEndValue(origin + QPoint(amplitude, 0));
        group->addAnimation(right);

        auto* left = new QPropertyAnimation(widget, "pos");
        left->setDuration(50);
        left->setEndValue(origin - QPoint(amplitude, 0));
        group->addAnimation(left);
    }

    auto* back = new QPropertyAnimation(widget, "pos");
    back->setDuration(50);
    back->setEndValue(origin);
    group->addAnimation(back);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}
