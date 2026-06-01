/**
 * @file NavigationControllerAnimations.cpp
 * @brief 导航控制器动画方法实现 - 面板滑动切换动画、呼吸动画
 *
 * 从 NavigationController.cpp 拆分而来，包含所有动画相关方法:
 *   - parentContainerWidth(): 获取面板父容器宽度（滑动距离计算）
 *   - animateSlideOut(): 旧面板滑出+淡出动画
 *   - animateSlideIn(): 新面板滑入+淡入动画
 *   - switchToPanel(): 面板切换协调器（防重入+并行动画组）
 *   - startBreathingAnimation(): 连接状态呼吸脉冲动画
 *   - stopBreathingAnimation(): 停止呼吸动画并恢复状态
 *
 * 动画规范 (CLAUDE.md 6.5):
 *   旧面板: pos (0,0)->(-width,0) 200ms InCubic, opacity 1->0
 *   新面板: pos (width,0)->(0,0) 250ms OutCubic, opacity 0->1
 *   呼吸动画: opacity 0.3<->1.0, 1500ms/半周期, InOutSine, 无限循环
 */

#include "core/NavigationController.h"
#include "core/Constants.h"
#include <QWidget>
#include <QLabel>
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

/**
 * @brief 启动连接状态呼吸动画
 *
 * 使用 QSequentialAnimationGroup 实现平滑往返脉冲:
 *   上半周期: opacity 0.3 -> 1.0, 1500ms, InOutSine (淡入)
 *   下半周期: opacity 1.0 -> 0.3, 1500ms, InOutSine (淡出)
 *   无限循环, 避免单方向动画结束时从1.0跳变到0.3的突兀感
 *
 * @param statusLabel 状态标签控件
 */
void NavigationController::startBreathingAnimation(QLabel* statusLabel)
{
    // 如果已有呼吸动画在运行，不重复创建
    if (m_breathingAnim && m_breathingAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    // 为状态标签创建透明度效果
    if (!m_connStatusEffect) {
        m_connStatusEffect = new QGraphicsOpacityEffect(statusLabel);
        statusLabel->setGraphicsEffect(m_connStatusEffect);
    }
    m_connStatusEffect->setOpacity(1.0);

    // 销毁旧动画(如果存在)
    // 旧动画使用 DeleteWhenStopped + 无限循环，stop() 触发自动销毁
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }

    // 构建呼吸动画: 顺序组 [0.3->1.0, 1500ms] + [1.0->0.3, 1500ms], 无限循环
    auto* group = new QSequentialAnimationGroup(this);

    // 上半周期: 0.3 -> 1.0 (淡入)
    auto* fadeIn = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeIn->setStartValue(0.3);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kBreatheCycleMs);
    fadeIn->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeIn);

    // 下半周期: 1.0 -> 0.3 (淡出)
    auto* fadeOut = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.3);
    fadeOut->setDuration(Animations::kBreatheCycleMs);
    fadeOut->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeOut);

    group->setLoopCount(-1);  // 无限循环
    group->start(QAbstractAnimation::DeleteWhenStopped);
    m_breathingAnim = group;
}

/**
 * @brief 停止连接状态呼吸动画
 *
 * 停止动画组 -> 恢复标签完全不透明 -> 清理 effect 对象。
 * 动画组使用 DeleteWhenStopped，stop() 后 Qt 自动销毁，此处仅清空指针。
 *
 * @param statusLabel 状态标签控件（析构时可传 nullptr）
 */
void NavigationController::stopBreathingAnimation(QLabel* statusLabel)
{
    if (m_breathingAnim) {
        // DeleteWhenStopped: stop() 后 Qt 自动 delete，不可再次 delete
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }
    // 恢复状态标签完全不透明
    // 注意: setGraphicsEffect(nullptr) 会自动 delete 旧的 effect，不可手动再 delete
    if (m_connStatusEffect) {
        m_connStatusEffect->setOpacity(1.0);
        if (statusLabel) {
            statusLabel->setGraphicsEffect(nullptr);  // Qt 自动 delete m_connStatusEffect
        }
        m_connStatusEffect = nullptr;
    }
}
