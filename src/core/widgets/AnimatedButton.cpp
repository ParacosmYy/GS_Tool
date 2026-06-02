/**
 * @file AnimatedButton.cpp
 * @brief 通用动画按钮实现 - hover渐变和press回弹微交互
 *
 * 动画通过 QGraphicsOpacityEffect + QPropertyAnimation 实现，
 * 不改变按钮几何/布局，QSS样式完全不受影响。
 */

#include "core/widgets/AnimatedButton.h"
#include "core/theme/Constants.h"

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

/** @brief 鼠标进入：触发hover渐入动画(opacity→1.0) @param event 进入事件 */
void AnimatedButton::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);
    startOpacityAnim(1.0, Animations::kButtonHoverMs, QEasingCurve::OutCubic);
}

/** @brief 鼠标离开：触发hover渐出动画(opacity→0.85) @param event 离开事件 */
void AnimatedButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    startOpacityAnim(0.85, Animations::kButtonHoverMs, QEasingCurve::OutCubic);
}

/** @brief 鼠标按下：触发按下反馈动画(opacity→0.75) @param event 鼠标事件 */
void AnimatedButton::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    startOpacityAnim(0.75, Animations::kButtonPressMs, QEasingCurve::Linear);
}

/** @brief 鼠标释放：触发回弹动画(opacity→1.0) @param event 鼠标事件 */
void AnimatedButton::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);
    startOpacityAnim(1.0, Animations::kButtonPressMs, QEasingCurve::OutCubic);
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

/** @brief 启动opacity动画，自动停止前一个动画防止并发冲突
 *  @param targetOpacity 目标opacity值
 *  @param durationMs 持续时间(毫秒)
 *  @param curve 缓动曲线 */
void AnimatedButton::startOpacityAnim(qreal targetOpacity, int durationMs, QEasingCurve curve)
{
    // 停止前一个动画: stop()触发DeleteWhenStopped自动deleteLater，无需手动deleteLater
    if (m_activeAnim) {
        m_activeAnim->stop();   // DeleteWhenStopped会自动调用deleteLater
        m_activeAnim = nullptr;
    }
    m_activeAnim = new QPropertyAnimation(this, "animOpacity");
    m_activeAnim->setStartValue(m_animOpacity);
    m_activeAnim->setEndValue(targetOpacity);
    m_activeAnim->setDuration(durationMs);
    m_activeAnim->setEasingCurve(curve);
    // 动画结束后自动清理指针（但不清除m_activeAnim，由deleteLater处理）
    connect(m_activeAnim, &QPropertyAnimation::finished, this, [this]() {
        m_activeAnim = nullptr;
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
