/**
 * @file NavigationControllerStatus.cpp
 * @brief 导航控制器状态动画方法实现 - 呼吸脉冲与断点折叠
 *
 * 从 NavigationControllerAnimations.cpp 拆分而来，包含:
 *   - startBreathingAnimation(): 连接状态呼吸脉冲动画
 *   - stopBreathingAnimation(): 停止呼吸动画并恢复状态
 *   - onBreakpointNavCollapse(): 响应断点变化折叠/展开导航树
 *
 * 呼吸动画规范 (CLAUDE.md 6.5):
 *   opacity 0.3<->1.0, 1500ms/半周期, InOutSine, 无限循环
 */

#include "core/navigation/NavigationController.h"
#include "shared/AnimationConstants.h"
#include <QLabel>
#include <QSplitter>
#include <QTreeView>

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
    /* 如果已有呼吸动画在运行，不重复创建 */
    if (m_breathingAnim && m_breathingAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    /* 为状态标签创建透明度效果 */
    if (!m_connStatusEffect) {
        m_connStatusEffect = new QGraphicsOpacityEffect(statusLabel);
        statusLabel->setGraphicsEffect(m_connStatusEffect);
    }
    m_connStatusEffect->setOpacity(1.0);

    /* 销毁旧动画(如果存在) */
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }

    /* 构建呼吸动画: 顺序组 [0.3->1.0, 1500ms] + [1.0->0.3, 1500ms], 无限循环 */
    auto* group = new QSequentialAnimationGroup(this);

    /* 上半周期: 0.3 -> 1.0 (淡入) */
    auto* fadeIn = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeIn->setStartValue(0.3);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kBreatheCycleMs);
    fadeIn->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeIn);

    /* 下半周期: 1.0 -> 0.3 (淡出) */
    auto* fadeOut = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.3);
    fadeOut->setDuration(Animations::kBreatheCycleMs);
    fadeOut->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeOut);

    group->setLoopCount(-1);
    group->start(QAbstractAnimation::DeleteWhenStopped);
    m_breathingAnim = group;
    ++m_totalBreathingStarts;  ///< 统计: 呼吸动画启动
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
        ++m_totalBreathingStops;  ///< 统计: 呼吸动画停止
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }
    if (m_connStatusEffect) {
        m_connStatusEffect->setOpacity(1.0);
        if (statusLabel) {
            statusLabel->setGraphicsEffect(nullptr);
        }
        m_connStatusEffect = nullptr;
    }
}

/**
 * @brief 响应断点变化，动画折叠/展开导航树
 *
 * 折叠: 通过 QSplitter::setSizes 将导航树宽度动画过渡到 0
 * 展开: 动画恢复到用户上次的 savedWidth
 *
 * @param collapsed true 表示应折叠导航树
 * @param splitter 主分割器（导航树 + 内容区）
 * @param savedWidth 用户上次的导航树展开宽度
 */
void NavigationController::onBreakpointNavCollapse(
    bool collapsed, QSplitter* splitter, int savedWidth)
{
    if (!splitter || splitter->sizes().size() < 2) return;

    /* 防止动画期间重复触发 */
    if (m_navCollapseAnimating) return;

    int currentNavWidth = splitter->sizes().at(0);

    /* 已在目标状态则跳过 */
    if (collapsed && currentNavWidth == 0) return;
    if (!collapsed && currentNavWidth > 0 && savedWidth <= 0) return;

    m_navCollapseAnimating = true;

    int targetWidth = collapsed ? 0 : (savedWidth > 0 ? savedWidth : 180);
    int contentWidth = splitter->width() - targetWidth;

    splitter->setSizes({targetWidth, contentWidth});

    /* 导航树可见性同步 */
    if (m_navTree) {
        m_navTree->setVisible(!collapsed);
    }

    m_navCollapseAnimating = false;
}
