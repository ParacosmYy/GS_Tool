/**
 * @file TerminalLayoutManager.cpp
 * @brief 终端布局管理器实现 — 构造/初始化/查询入口
 *
 * 负责组装TerminalWidget、TerminalSearchBar、DirectionFilter等组件，
 * 处理搜索栏的展开/收起动画，以及终端区域的布局调整。
 *
 * 布局应用与模式切换见 TerminalLayoutManagerApply.cpp。
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

// ---- 布局应用与模式切换见 TerminalLayoutManagerApply.cpp ----
// ---- 统计方法见 TerminalLayoutManagerStats.cpp ----
