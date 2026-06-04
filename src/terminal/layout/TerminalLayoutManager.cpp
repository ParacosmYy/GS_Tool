/**
 * @file TerminalLayoutManager.cpp
 * @brief 终端布局管理器实现 — 管理终端区域的布局和搜索栏动画
 *
 * 负责组装TerminalWidget、TerminalSearchBar、DirectionFilter等组件，
 * 处理搜索栏的展开/收起动画，以及终端区域的布局调整。
 */
#include "terminal/layout/TerminalLayoutManager.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/search/TerminalSearchBar.h"

#include <QVBoxLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

/** @brief 构造终端布局管理器，创建容器Widget并初始化默认混合布局 @param parent 父对象(通常为MainWindow) */
TerminalLayoutManager::TerminalLayoutManager(QObject* parent)
    : QObject(parent)
    , m_mainTerminal(nullptr)
    , m_rxTerminal(nullptr)
    , m_txTerminal(nullptr)
    , m_searchBar(nullptr)
    , m_container(nullptr)
    , m_splitter(nullptr)
    , m_layout(TerminalLayout::Mixed)
    , m_model(nullptr)
    , m_displayMode(DisplayMode::Text)
    , m_showTimestamp(false)
    , m_showDirectionPrefix(false)
{
    // 创建容器widget: 所有布局变化都在这个容器内进行
    // parent (QObject*) 实际是 MainWindow*，转型为 QWidget* 用于 QWidget 构造
    m_container = new QWidget(qobject_cast<QWidget*>(QObject::parent()));
    m_container->setObjectName("terminalContainer");
    auto* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

/** @brief 初始化布局管理器，缓存主终端显示设置并应用默认混合布局 @param mainTerminal 主终端控件指针 @param searchBar 搜索栏控件指针 */
void TerminalLayoutManager::initialize(TerminalWidget* mainTerminal, TerminalSearchBar* searchBar)
{
    m_mainTerminal = mainTerminal;
    m_searchBar = searchBar;

    // 缓存主终端的初始显示设置
    if (m_mainTerminal) {
        m_displayMode = m_mainTerminal->displayMode();
        m_showTimestamp = m_mainTerminal->showTimestamp();
        m_showDirectionPrefix = m_mainTerminal->showDirectionPrefix();
    }

    // 初始布局: 混合模式
    applyLayout();
}

/** @brief 设置共享的数据模型，分栏终端与主终端共用同一个TerminalModel @param model TerminalModel指针 */
void TerminalLayoutManager::setTerminalModel(TerminalModel* model)
{
    m_model = model;
}

/** @brief 返回终端容器Widget @return 容器Widget指针，所有终端组件的父容器 */
QWidget* TerminalLayoutManager::container() const
{
    return m_container;
}

/** @brief 返回当前布局模式 @return TerminalLayout枚举值(Mixed/SplitHorizontal/SplitVertical) */
TerminalLayout TerminalLayoutManager::layout() const
{
    return m_layout;
}

/** @brief 返回当前布局下的所有终端控件列表，混合模式返回主终端，分栏模式返回RX/TX终端 @return 终端控件指针列表 */
QList<TerminalWidget*> TerminalLayoutManager::terminalWidgets() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return {m_mainTerminal};
    } else {
        return {m_rxTerminal, m_txTerminal};
    }
}

/** @brief 返回主要终端控件，混合模式返回主终端，分栏模式返回RX终端 @return 主终端控件指针 */
TerminalWidget* TerminalLayoutManager::primaryTerminal() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return m_mainTerminal;
    }
    return m_rxTerminal;
}

// ---- 统计方法见 TerminalLayoutManagerStats.cpp ----

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

// ---- 分栏布局方法已拆分至 TerminalLayoutSplit.cpp ----
// applySplitLayout() / createSplitTerminal() / syncDisplaySettings()
