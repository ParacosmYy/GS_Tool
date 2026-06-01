/**
 * @file AnimatedButton.cpp
 * @brief 通用动画按钮实现 - hover渐变和press回弹微交互
 *
 * 动画通过 QGraphicsOpacityEffect + QPropertyAnimation 实现，
 * 不改变按钮几何/布局，QSS样式完全不受影响。
 */

#include "core/AnimatedButton.h"

#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QEnterEvent>

// ---- 构造 ----

/** @brief 构造动画按钮，初始化opacity效果 @param text 按钮文字 @param parent 父控件 */
AnimatedButton::AnimatedButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    ensureOpacityEffect();
}

// ---- 属性访问 ----

/** @brief 获取当前动画opacity值 @return opacity 0.0~1.0 */
qreal AnimatedButton::animOpacity() const
{
    return m_animOpacity;
}

/** @brief 设置动画opacity值(QPropertyAnimation写端) @param opacity 目标opacity */
void AnimatedButton::setAnimOpacity(qreal opacity)
{
    m_animOpacity = opacity;
    ensureOpacityEffect();
    m_opacityEffect->setOpacity(opacity);
}

// ---- 事件处理 ----

/** @brief 鼠标进入：触发hover渐入动画(opacity→1.0, 200ms) @param event 进入事件 */
void AnimatedButton::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);
    // hover渐入: 当前opacity → 1.0, 200ms OutCubic
    auto* anim = new QPropertyAnimation(this, "animOpacity");
    anim->setStartValue(m_animOpacity);
    anim->setEndValue(1.0);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 鼠标离开：触发hover渐出动画(opacity→0.85, 200ms) @param event 离开事件 */
void AnimatedButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    // hover渐出: 当前opacity → 0.85, 200ms OutCubic
    auto* anim = new QPropertyAnimation(this, "animOpacity");
    anim->setStartValue(m_animOpacity);
    anim->setEndValue(0.85);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 鼠标按下：触发按下反馈动画(opacity→0.75, 100ms) @param event 鼠标事件 */
void AnimatedButton::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    // 按下反馈: 当前opacity → 0.75, 100ms Linear
    auto* anim = new QPropertyAnimation(this, "animOpacity");
    anim->setStartValue(m_animOpacity);
    anim->setEndValue(0.75);
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 鼠标释放：触发回弹动画(opacity→1.0, 100ms) @param event 鼠标事件 */
void AnimatedButton::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);
    // 释放回弹: 当前opacity → 1.0, 100ms OutCubic
    auto* anim = new QPropertyAnimation(this, "animOpacity");
    anim->setStartValue(m_animOpacity);
    anim->setEndValue(1.0);
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---- 内部方法 ----

/** @brief 延迟创建QGraphicsOpacityEffect(首次使用时创建，避免未使用时浪费资源) */
void AnimatedButton::ensureOpacityEffect()
{
    if (!m_opacityEffect) {
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(m_opacityEffect);
        m_opacityEffect->setOpacity(1.0);
    }
}
