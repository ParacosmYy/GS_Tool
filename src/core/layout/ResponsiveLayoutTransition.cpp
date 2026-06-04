/**
 * @file ResponsiveLayoutTransition.cpp
 * @brief 响应式布局管理器 - 过渡动画与可见性策略实现
 *
 * 从 ResponsiveLayout.cpp 拆分而来，包含:
 *   - applyVisibilityPolicy: 按断点应用widget可见性策略
 *   - startTransitionAnimation: 断点切换透明度脉冲动画
 */

#include "core/layout/ResponsiveLayout.h"
#include "shared/AnimationConstants.h"
#include <QMainWindow>
#include <QGraphicsOpacityEffect>

/**
 * @brief 应用当前断点的 widget 可见性策略
 *
 * 遍历所有已注册的 widget，根据当前断点的可见性策略设置
 * widget 的显示/隐藏/折叠状态。
 *
 * @param bp 当前断点
 */
void ResponsiveLayout::applyVisibilityPolicy(Breakpoint bp)
{
    for (auto it = m_visibilityPolicies.constBegin();
         it != m_visibilityPolicies.constEnd(); ++it) {
        QWidget* widget = it.key();
        if (!widget) continue;

        const auto& policyMap = it.value();
        Visibility vis = policyMap.value(bp, Visibility::Visible);

        switch (vis) {
        case Visibility::Visible:
            widget->setVisible(true);
            widget->setMinimumWidth(0);
            widget->setMaximumWidth(QWIDGETSIZE_MAX);
            break;
        case Visibility::Hidden:
            widget->setVisible(false);
            break;
        case Visibility::Collapsed:
            widget->setVisible(true);
            widget->setMinimumWidth(0);
            widget->setMaximumWidth(0);
            break;
        }
    }
}

/**
 * @brief 启动断点过渡动画
 *
 * 对被监听窗口应用透明度过渡动画:
 *   250ms InOutCubic, opacity 1.0 -> 0.85 -> 1.0
 * 实现断点切换时的平滑视觉过渡，避免布局突然跳变。
 *
 * @param newBp 目标断点
 */
void ResponsiveLayout::startTransitionAnimation(Breakpoint newBp)
{
    Q_UNUSED(newBp)

    if (!m_window) return;

    /* 清理进行中的动画 */
    if (m_transitionAnim) {
        m_transitionAnim->stop();
        delete m_transitionAnim;
        m_transitionAnim = nullptr;
    }

    /* 确保窗口有 opacity effect */
    QGraphicsOpacityEffect* effect =
        qobject_cast<QGraphicsOpacityEffect*>(m_window->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_window);
        m_window->setGraphicsEffect(effect);
    }

    /* 透明度脉冲动画: 1.0 -> 0.85 -> 1.0 */
    effect->setOpacity(1.0);

    m_transitionAnim = new QPropertyAnimation(effect, "opacity");
    m_transitionAnim->setStartValue(1.0);
    m_transitionAnim->setKeyValueAt(0.3, 0.85);
    m_transitionAnim->setEndValue(1.0);
    m_transitionAnim->setDuration(Animations::kBreakpointTransitionMs);
    m_transitionAnim->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this]() {
        if (m_window) {
            auto* eff = qobject_cast<QGraphicsOpacityEffect*>(m_window->graphicsEffect());
            if (eff) eff->setOpacity(1.0);
        }
        m_transitionAnim = nullptr;
    });

    emit breakpointTransitionStarted(newBp);
    m_transitionAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
