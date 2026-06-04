/**
 * @file BasePanelStates.cpp
 * @brief BasePanel 状态切换、动画和自绘实现
 *
 * 从 BasePanel.cpp 拆分而来，包含空状态/加载/骨架屏切换、
 * 淡入淡出动画、paintEvent微阴影绘制和统计接口。
 */

#include "core/widgets/BasePanel.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "core/theme/ThemeManager.h"
#include "core/widgets/EmptyStateWidget.h"
#include "core/widgets/LoadingSpinner.h"
#include "core/widgets/SkeletonWidget.h"
#include "shared/AnimationConstants.h"

// ============================================================================
// 空状态 / 加载状态
// ============================================================================

/** @brief 显示空状态提示，隐藏内容/加载/骨架屏 @param title 空状态标题 @param description 空状态描述 */
void BasePanel::showEmptyState(const QString& title, const QString& description)
{
    if (m_emptyState) {
        m_emptyState->setTitle(title);
        m_emptyState->setDescription(description);
        m_emptyState->setVisible(true);
    }
    if (m_content) m_content->setVisible(false);
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
    if (m_skeleton) m_skeleton->setVisible(false);
}

/** @brief 隐藏空状态提示，恢复内容区域显示 */
void BasePanel::hideEmptyState()
{
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

/** @brief 显示加载旋转指示器，隐藏内容/空状态/骨架屏 */
void BasePanel::showLoading()
{
    if (m_loadingSpinner) m_loadingSpinner->setVisible(true);
    if (m_content) m_content->setVisible(false);
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_skeleton) m_skeleton->setVisible(false);
}

/** @brief 隐藏加载旋转指示器，恢复内容区域显示 */
void BasePanel::hideLoading()
{
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

/** @brief 显示骨架屏占位动画，隐藏内容/空状态/加载指示器 */
void BasePanel::showSkeleton()
{
    if (m_skeleton) m_skeleton->setVisible(true);
    if (m_content) m_content->setVisible(false);
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
}

/** @brief 隐藏骨架屏占位动画，恢复内容区域显示 */
void BasePanel::hideSkeleton()
{
    if (m_skeleton) m_skeleton->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

// ============================================================================
// 动画槽
// ============================================================================

/** @brief 播放面板淡入+滑入显示动画(250ms OutCubic) */
void BasePanel::animateShow()
{
    ++m_totalShows;
    show();
    setPanelOpacity(0.0);

    auto* group = new QParallelAnimationGroup(this);

    auto* fadeIn = new QPropertyAnimation(this, "panelOpacity");
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kPanelSlideInMs);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(fadeIn);

    auto* slideIn = new QPropertyAnimation(this, "pos");
    QPoint basePos = pos();
    slideIn->setStartValue(basePos + QPoint(0, -20));
    slideIn->setEndValue(basePos);
    slideIn->setDuration(Animations::kPanelSlideInMs);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(slideIn);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 播放面板淡出+滑出隐藏动画(200ms InCubic)，动画结束后隐藏控件 */
void BasePanel::animateHide()
{
    ++m_totalHides;
    auto* group = new QParallelAnimationGroup(this);

    auto* fadeOut = new QPropertyAnimation(this, "panelOpacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(Animations::kPanelSlideOutMs);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(fadeOut);

    auto* slideOut = new QPropertyAnimation(this, "pos");
    QPoint basePos = pos();
    slideOut->setStartValue(basePos);
    slideOut->setEndValue(basePos + QPoint(0, -20));
    slideOut->setDuration(Animations::kPanelSlideOutMs);
    slideOut->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(slideOut);

    connect(group, &QAbstractAnimation::finished, this, &QWidget::hide);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================================
// 自绘: 微阴影
// ============================================================================

/** @brief 自绘事件，在面板底部和右侧绘制微阴影渐变效果 @param event 绘制事件 */
void BasePanel::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor shadowColor = ThemeManager::instance().color(
        ThemeManager::SemanticColor::Shadow);

    const int w = width();
    const int h = height();
    const int shadowSize = 6;

    // 底部阴影渐变
    QLinearGradient bottomGrad(0, h - shadowSize, 0, h);
    QColor cBot = shadowColor;
    cBot.setAlpha(40);
    bottomGrad.setColorAt(0.0, QColor(cBot.red(), cBot.green(), cBot.blue(), 0));
    bottomGrad.setColorAt(1.0, cBot);
    p.fillRect(0, h - shadowSize, w, shadowSize, bottomGrad);

    // 右侧阴影渐变
    QLinearGradient rightGrad(w - shadowSize, 0, w, 0);
    QColor cRight = shadowColor;
    cRight.setAlpha(25);
    rightGrad.setColorAt(0.0, QColor(cRight.red(), cRight.green(), cRight.blue(), 0));
    rightGrad.setColorAt(1.0, cRight);
    p.fillRect(w - shadowSize, 0, shadowSize, h, rightGrad);
}

// ============================================================================
// 统计计数器
// ============================================================================

/** @brief 重置面板统计计数器(切换/展开/折叠/显示/隐藏/标题变更/标题点击/拖拽/设置打开) */
void BasePanel::resetPanelStatistics()
{
    m_totalToggles = 0;
    m_totalExpansions = 0;
    m_totalCollapses = 0;
    m_totalShows = 0;
    m_totalHides = 0;
    m_totalTitleChanges = 0;
    m_totalTitleClicks = 0;
    m_totalDragStarts = 0;
    m_totalSettingsOpens = 0;
}
