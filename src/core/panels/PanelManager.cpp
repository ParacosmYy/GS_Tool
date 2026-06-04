/**
 * @file PanelManager.cpp
 * @brief 面板管理器实现 - 构造函数、紧凑模式、切换通知和统计
 *
 * 本文件实现 PanelManager 的构造函数、紧凑模式切换、面板切换通知
 * 和统计计数器。
 * 面板Getter/映射表/包装器见 PanelManagerGetters.cpp。
 * 面板创建逻辑见 PanelManagerCreation.cpp。
 */

#include "core/panels/PanelManager.h"
#include "core/widgets/BasePanel.h"
#include <QStyle>
#include "chart/fft/FftWidget.h"
#include "chart/stats/ScatterWidget.h"
#include "chart/stats/HistogramWidget.h"

/** @brief 构造面板管理器，所有面板指针由头文件初始化为 nullptr */
PanelManager::PanelManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置紧凑模式，隐藏折叠按钮和标题栏图标，缩小面板间距 */
void PanelManager::setCompactMode(bool compact)
{
    if (m_compactMode == compact) { return; }
    m_compactMode = compact;

    for (auto it = m_wrappers.constBegin(); it != m_wrappers.constEnd(); ++it) {
        BasePanel* panel = it.value();
        if (panel) {
            panel->setProperty("compactMode", compact);
            if (compact && !panel->isVisible()) {
                panel->setCollapsed(true);
            }
            panel->style()->unpolish(panel);
            panel->style()->polish(panel);
        }
    }

    if (compact) {
        if (m_fftWidget && m_fftWidget->parentWidget()) {
            m_fftWidget->parentWidget()->setProperty("compactHidden", true);
        }
        if (m_scatterWidget && m_scatterWidget->parentWidget()) {
            m_scatterWidget->parentWidget()->setProperty("compactHidden", true);
        }
        if (m_histogramWidget && m_histogramWidget->parentWidget()) {
            m_histogramWidget->parentWidget()->setProperty("compactHidden", true);
        }
    }
}

/** @brief 查询是否处于紧凑模式 */
bool PanelManager::isCompactMode() const
{
    return m_compactMode;
}

/** @brief 通知面板切换发生(由NavigationController调用)，更新切换计数和最大并发面板数 */
void PanelManager::onPanelSwitched(int visibleCount)
{
    ++m_totalPanelSwitches;
    ++m_totalCategoryExpands;
    m_totalActivePanelsTracked += static_cast<quint64>(visibleCount);
    if (static_cast<quint64>(visibleCount) > m_maxConcurrentPanels) {
        m_maxConcurrentPanels = static_cast<quint64>(visibleCount);
    }
}

// ==================== 统计接口 ====================

/** @brief 获取累计面板切换次数 @return 切换总次数 */
quint64 PanelManager::totalPanelSwitches() const { return m_totalPanelSwitches; }
/** @brief 获取累计创建面板总数 @return 面板总数 */
quint64 PanelManager::totalPanelsCreated() const { return m_totalPanelsCreated; }
/** @brief 获取历史最大并发面板数 @return 最大并发数 */
quint64 PanelManager::maxConcurrentPanels() const { return m_maxConcurrentPanels; }

/** @brief 获取累计面板创建次数 @return 创建次数 */
quint64 PanelManager::totalPanelCreations() const { return m_totalPanelCreations; }

/** @brief 获取累计面板删除次数 @return 删除次数 */
quint64 PanelManager::totalPanelDeletions() const { return m_totalPanelDeletions; }

/** @brief 获取累计活跃面板追踪次数 @return 追踪总数 */
quint64 PanelManager::totalActivePanelsTracked() const { return m_totalActivePanelsTracked; }

/** @brief 获取累计面板注册次数(wrapPanels中) @return 注册总数 */
quint64 PanelManager::totalPanelRegisters() const { return m_totalPanelRegisters; }

/** @brief 获取累计面板注销次数 @return 注销总数 */
quint64 PanelManager::totalPanelUnregisters() const { return m_totalPanelUnregisters; }

/** @brief 获取累计分类展开次数 @return 展开总数 */
quint64 PanelManager::totalCategoryExpands() const { return m_totalCategoryExpands; }

/** @brief 重置所有统计计数器为零 */
void PanelManager::resetStats()
{
    m_totalPanelSwitches = 0;
    m_totalPanelsCreated = 0;
    m_maxConcurrentPanels = 0;
    m_totalPanelCreations = 0;
    m_totalPanelDeletions = 0;
    m_totalActivePanelsTracked = 0;
    m_totalPanelRegisters = 0;
    m_totalPanelUnregisters = 0;
    m_totalCategoryExpands = 0;
}
