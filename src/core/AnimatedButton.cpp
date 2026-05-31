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

AnimatedButton::AnimatedButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    ensureOpacityEffect();
}

// ---- 属性访问 ----

qreal AnimatedButton::animOpacity() const
{
    return m_animOpacity;
}

void AnimatedButton::setAnimOpacity(qreal opacity)
{
    m_animOpacity = opacity;
    ensureOpacityEffect();
    m_opacityEffect->setOpacity(opacity);
}

// ---- 事件处理 ----

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

void AnimatedButton::ensureOpacityEffect()
{
    if (!m_opacityEffect) {
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(m_opacityEffect);
        m_opacityEffect->setOpacity(1.0);
    }
}
