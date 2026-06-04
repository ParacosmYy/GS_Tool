/**
 * @file NavigationControllerAnimations.cpp
 * @brief 导航控制器动画方法实现 - 面板滑动切换动画、呼吸动画
 *
 * 从 NavigationController.cpp 拆分而来，包含面板滑动切换动画:
 *   - parentContainerWidth(): 获取面板父容器宽度（滑动距离计算）
 *   - animateSlideOut(): 旧面板滑出+淡出动画
 *   - animateSlideIn(): 新面板滑入+淡入动画
 *   - switchToPanel(): 面板切换协调器（防重入+并行动画组）
 *
 * 动画规范 (CLAUDE.md 6.5):
 *   旧面板: pos (0,0)->(-width,0) 200ms InCubic, opacity 1->0
 *   新面板: pos (width,0)->(0,0) 250ms OutCubic, opacity 0->1
 *   呼吸动画: opacity 0.3<->1.0, 1500ms/半周期, InOutSine, 无限循环
 */

#include "core/navigation/NavigationController.h"
#include "shared/AnimationConstants.h"
#include <QWidget>
#include <QTreeView>

/** @brief 获取面板父容器宽度（serialPanel）作为滑动距离 */
int NavigationController::parentContainerWidth(QWidget* panel) const
{
    QWidget* parent = panel ? panel->parentWidget() : nullptr;
    return parent ? parent->width() : 0;
}

/** @brief 旧面板滑出+淡出: pos (0,0)->(-width,0) InCubic, opacity 1->0 */
void NavigationController::animateSlideOut(QWidget* oldPanel, QParallelAnimationGroup* group)
{
    const int slideWidth = parentContainerWidth(oldPanel);
    if (slideWidth <= 0) { oldPanel->setVisible(false); return; } ///< 容器宽度无效时跳过

    QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
    fadeOutEffect->setOpacity(1.0);
    oldPanel->setGraphicsEffect(fadeOutEffect);

    QPropertyAnimation* slideOut = new QPropertyAnimation(oldPanel, "pos");
    slideOut->setStartValue(QPoint(0, 0));
    slideOut->setEndValue(QPoint(-slideWidth, 0));
    slideOut->setDuration(Animations::kPanelSlideOutMs);
    slideOut->setEasingCurve(QEasingCurve::InCubic);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(Animations::kPanelSlideOutMs);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    // 动画组完成后再清除effect + 隐藏 + 恢复位置
    // 注意: 不能连接slideOut单独的finished，因为fadeOut可能还在使用fadeOutEffect
    connect(group, &QParallelAnimationGroup::finished, this, [oldPanel]() {
        if (oldPanel->graphicsEffect()) oldPanel->setGraphicsEffect(nullptr);
        oldPanel->setVisible(false);
        oldPanel->move(0, 0);
    });

    group->addAnimation(slideOut);
    group->addAnimation(fadeOut);
}

/** @brief 新面板滑入+淡入: pos (width,0)->(0,0) OutCubic, opacity 0->1 */
void NavigationController::animateSlideIn(QWidget* newPanel, QParallelAnimationGroup* group)
{
    const int slideWidth = parentContainerWidth(newPanel);
    if (slideWidth <= 0) {  ///< 容器宽度无效时跳过动画
        newPanel->setGraphicsEffect(nullptr);
        newPanel->move(0, 0);
        newPanel->setVisible(true);
        return;
    }

    QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(newPanel);
    fadeInEffect->setOpacity(0.0);
    newPanel->setGraphicsEffect(fadeInEffect);
    newPanel->move(slideWidth, 0);  ///< 放置到右侧屏幕外
    newPanel->setVisible(true);
    newPanel->raise();              ///< 提升到最前（覆盖旧面板）

    QPropertyAnimation* slideIn = new QPropertyAnimation(newPanel, "pos");
    slideIn->setStartValue(QPoint(slideWidth, 0));
    slideIn->setEndValue(QPoint(0, 0));
    slideIn->setDuration(Animations::kPanelSlideInMs);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kPanelSlideInMs);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 滑入完成: 清除 effect 恢复正常绘制性能
    connect(slideIn, &QPropertyAnimation::finished, this, [newPanel, fadeInEffect]() {
        if (newPanel->graphicsEffect() == fadeInEffect) newPanel->setGraphicsEffect(nullptr);
    });

    group->addAnimation(slideIn);
    group->addAnimation(fadeIn);
}

/**
 * @brief 面板切换（带滑入滑出动画）
 *
 * 流程 (CLAUDE.md 6.5): 防重入 -> 清理残留 -> 创建并行动画组
 * 旧面板: pos (0,0)->(-width,0) 200ms InCubic + opacity 1->0
 * 新面板: pos (width,0)->(0,0) 250ms OutCubic + opacity 0->1
 * 动画期间禁用导航树，完成后恢复。动画组使用 deleteLater 自动清理。
 *
 * @param newPanel 目标面板 widget
 */
void NavigationController::switchToPanel(QWidget* newPanel)
{
    if (m_panelSwitching) return;           ///< 防重入: 动画进行中忽略
    if (m_currentPanel == newPanel) return; ///< 目标与当前相同，无需切换

    ++m_totalNavigations;                   ///< 统计: 导航切换总次数递增
    ++m_totalPanelSwitches;                 ///< 统计: 面板实际变更次数递增

    QWidget* oldPanel = m_currentPanel;
    m_currentPanel = newPanel;

    // 隐藏所有非当前、非旧面板，并清除残留的 opacity effect 和位移偏移
    for (auto* w : allSwitchablePanels()) {
        if (w && w != newPanel && w != oldPanel) {
            if (w->graphicsEffect()) w->setGraphicsEffect(nullptr);
            w->move(0, 0);
            w->setVisible(false);
        }
    }

    if (!newPanel) return;

    m_panelSwitching = true;
    if (m_navTree) m_navTree->setDisabled(true);  ///< 动画期间禁用导航树

    // 创建并行动画组: 旧面板滑出(如有)和新面板滑入同时执行
    auto* animGroup = new QParallelAnimationGroup(this);
    m_switchAnimGroup = animGroup;

    if (oldPanel && oldPanel->isVisible()) {
        animateSlideOut(oldPanel, animGroup);  ///< 旧面板: 滑出+淡出 200ms InCubic
    }
    animateSlideIn(newPanel, animGroup);       ///< 新面板: 滑入+淡入 250ms OutCubic

    // 动画完成: 恢复导航树交互 + 重置切换标志 + 自动清理动画组
    connect(animGroup, &QParallelAnimationGroup::finished, this, [this]() {
        m_panelSwitching = false;
        m_switchAnimGroup = nullptr;
        if (m_navTree) m_navTree->setDisabled(false);
    });
    connect(animGroup, &QParallelAnimationGroup::finished,
            animGroup, &QObject::deleteLater);

    animGroup->start();
}

// startBreathingAnimation/stopBreathingAnimation/onBreakpointNavCollapse
// 见 NavigationControllerStatus.cpp
