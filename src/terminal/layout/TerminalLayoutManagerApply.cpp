/**
 * @file TerminalLayoutManagerApply.cpp
 * @brief 终端布局管理器 - 布局应用与模式切换实现
 *
 * 从 TerminalLayoutManager.cpp 拆分而来，包含:
 *   - applyLayout(): 布局应用入口(清空容器+重建)
 *   - applyMixedLayout(): 混合模式布局恢复
 *   - setLayout(): 布局模式设置
 *
 * 分栏布局见 TerminalLayoutSplit.cpp。
 * 统计接口见 TerminalLayoutManagerStats.cpp。
 */

#include "terminal/layout/TerminalLayoutManager.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/search/TerminalSearchBar.h"

#include <QVBoxLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

/** @brief 通过下拉框索引设置布局模式，0=混合，1=水平分栏，2=垂直分栏 @param layoutIndex 下拉框索引 */
void TerminalLayoutManager::setLayout(int layoutIndex)
{
    // 下拉框索引映射: 0=Mixed, 1=SplitHorizontal, 2=SplitVertical
    TerminalLayout layouts[] = {
        TerminalLayout::Mixed,
        TerminalLayout::SplitHorizontal,
        TerminalLayout::SplitVertical
    };
    if (layoutIndex >= 0 && layoutIndex < 3) {
        setLayout(layouts[layoutIndex]);
    }
}

/** @brief 设置布局模式并重新应用布局，相同布局时跳过 @param layout 目标布局模式 */
void TerminalLayoutManager::setLayout(TerminalLayout layout)
{
    if (m_layout == layout) return;
    ++m_totalSwitches;  ///< 统计: 布局切换次数递增
    ++m_totalViewModeChanges;  ///< 统计: 视图模式变更次数递增
    // 统计: 混合↔分栏切换时递增分栏开关计数
    if ((m_layout == TerminalLayout::Mixed) != (layout == TerminalLayout::Mixed)) {
        ++m_totalSplitToggles;
    }
    m_layout = layout;
    applyLayout();
    emit layoutChanged(m_layout);
}

/** @brief 应用当前布局模式，清空容器、销毁旧分割器、重建搜索栏和终端区域 */
void TerminalLayoutManager::applyLayout()
{
    // 清空容器中的所有子widget
    // 使用QBoxLayout指针以支持addWidget(widget, stretch)语法
    auto* containerLayout = qobject_cast<QBoxLayout*>(m_container->layout());
    if (!containerLayout) return; ///< 防御性检查: layout必须为QBoxLayout

    // 先从布局中移除所有子项（不删除widget，只是脱离布局）
    while (containerLayout->count() > 0) {
        QLayoutItem* item = containerLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->setParent(nullptr);
        }
        delete item;
    }

    // 销毁旧的分割器
    if (m_splitter) {
        m_splitter->deleteLater();
        m_splitter = nullptr;
    }

    // 从主终端缓存最新的显示设置
    if (m_mainTerminal) {
        m_displayMode = m_mainTerminal->displayMode();
        m_showTimestamp = m_mainTerminal->showTimestamp();
        m_showDirectionPrefix = m_mainTerminal->showDirectionPrefix();
    }

    // 搜索栏始终显示在顶部（三种模式共用一个搜索栏）
    if (m_searchBar) {
        m_searchBar->setParent(m_container);
        containerLayout->addWidget(m_searchBar);
        m_searchBar->show();
    }

    if (m_layout == TerminalLayout::Mixed) {
        applyMixedLayout();
    } else {
        applySplitLayout();
    }
    ++m_totalLayoutChanges;  ///< 统计: 每次应用布局递增
}

/** @brief 应用混合布局，销毁分栏终端并恢复主终端无过滤状态 */
void TerminalLayoutManager::applyMixedLayout()
{
    auto* containerLayout = qobject_cast<QBoxLayout*>(m_container->layout());
    if (!containerLayout) return;

    // ---- 混合模式: 显示主终端（包含TX和RX所有数据） ----
    // 统计: 如果之前存在分栏终端，则记录一次合并操作
    const bool hadSplitTerminals = (m_rxTerminal != nullptr || m_txTerminal != nullptr);
    // 销毁分栏终端（如果存在）
    if (m_rxTerminal) {
        m_rxTerminal->deleteLater();
        m_rxTerminal = nullptr;
        ++m_totalTabRemoves; ///< 统计: RX终端Tab移除
    }
    if (m_txTerminal) {
        m_txTerminal->deleteLater();
        m_txTerminal = nullptr;
        ++m_totalTabRemoves; ///< 统计: TX终端Tab移除
    }
    if (hadSplitTerminals) {
        ++m_totalMerges; ///< 统计: 从分栏合并到混合模式递增
    }

    // 恢复主终端的无过滤状态
    if (m_mainTerminal) {
        m_mainTerminal->clearDirectionFilter();
        m_mainTerminal->setParent(m_container);
        containerLayout->addWidget(m_mainTerminal, 1);
        m_mainTerminal->show();
    }
}

// ---- 分栏布局方法见 TerminalLayoutSplit.cpp ----
// ---- 统计方法见 TerminalLayoutManagerStats.cpp ----
