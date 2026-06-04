/**
 * @file TerminalLayoutSplit.cpp
 * @brief 终端布局管理器 — 分栏布局相关方法实现
 *
 * 从 TerminalLayoutManager.cpp 拆分而来，包含分栏布局的创建、
 * 分栏终端控件的构建与显示设置同步方法。
 */

#include "terminal/layout/TerminalLayoutManager.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/model/TerminalModel.h"

#include <QSplitter>
#include <QBoxLayout>

/** @brief 应用分栏布局，创建RX/TX终端并用QSplitter分割 */
void TerminalLayoutManager::applySplitLayout()
{
    auto* containerLayout = qobject_cast<QBoxLayout*>(m_container->layout());
    if (!containerLayout) return;

    // ---- 分栏模式: 创建RX/TX两个终端，用QSplitter分割 ----
    Qt::Orientation orient = (m_layout == TerminalLayout::SplitHorizontal)
        ? Qt::Horizontal : Qt::Vertical;

    // 隐藏主终端（分栏模式下不使用主终端显示数据，但仍保留其模型连接）
    if (m_mainTerminal) {
        m_mainTerminal->hide();
        m_mainTerminal->setParent(nullptr);
    }

    // 创建分割器
    m_splitter = new QSplitter(orient, m_container);
    m_splitter->setObjectName("terminalSplitter");
    m_splitter->setChildrenCollapsible(false);  // 不允许完全折叠子面板

    // 创建/重建分栏终端
    if (m_rxTerminal) {
        m_rxTerminal->deleteLater();
        ++m_totalTabRemoves; ///< 统计: 旧RX终端Tab移除
    }
    if (m_txTerminal) {
        m_txTerminal->deleteLater();
        ++m_totalTabRemoves; ///< 统计: 旧TX终端Tab移除
    }

    // RX终端: 左侧(左右分栏) 或 上方(上下分栏)
    m_rxTerminal = createSplitTerminal(DataDirection::Rx,
        tr("RX (接收)"));
    m_splitter->addWidget(m_rxTerminal);
    ++m_totalSplits;  ///< 统计: 每次创建分栏递增
    ++m_totalTabAdds; ///< 统计: RX终端Tab添加

    // TX终端: 右侧(左右分栏) 或 下方(上下分栏)
    m_txTerminal = createSplitTerminal(DataDirection::Tx,
        tr("TX (发送)"));
    m_splitter->addWidget(m_txTerminal);
    ++m_totalTabAdds; ///< 统计: TX终端Tab添加

    // 均分分割器空间
    m_splitter->setSizes({1, 1});

    containerLayout->addWidget(m_splitter, 1);
}

/** @brief 创建分栏终端控件，设置共享模型、方向过滤和同步显示设置 @param direction 数据方向(RX/TX) @param label 终端标签 @return 新创建的终端控件指针 */
TerminalWidget* TerminalLayoutManager::createSplitTerminal(DataDirection direction, const QString& label)
{
    Q_UNUSED(label);

    auto* terminal = new TerminalWidget(m_container);
    terminal->setObjectName((direction == DataDirection::Rx)
        ? "rxTerminal" : "txTerminal");

    // 设置共享的数据模型（与主终端共用同一个TerminalModel）
    if (m_model) {
        terminal->setModel(m_model);
    }

    // 设置方向过滤器: 只显示指定方向的数据
    terminal->setDirectionFilter(direction);

    // 同步主终端的显示设置
    syncDisplaySettings(terminal);

    // 分栏终端设置最小尺寸（比主终端小，适合分栏显示）
    terminal->setMinimumSize(200, 100);

    return terminal;
}

/** @brief 将主终端的显示设置同步到目标终端，包括显示模式、时间戳、方向前缀和自动滚动 @param target 目标终端控件指针 */
void TerminalLayoutManager::syncDisplaySettings(TerminalWidget* target) const
{
    if (!target) return;
    target->setDisplayMode(m_displayMode);
    target->setShowTimestamp(m_showTimestamp);
    target->setShowDirectionPrefix(m_showDirectionPrefix);
    target->setAutoScroll(true);
}
