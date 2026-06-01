/**
 * @file TerminalLayoutManager.cpp
 * @brief 终端布局管理器实现 — 管理终端区域的布局和搜索栏动画
 *
 * 负责组装TerminalWidget、TerminalSearchBar、DirectionFilter等组件，
 * 处理搜索栏的展开/收起动画，以及终端区域的布局调整。
 */
#include "terminal/TerminalLayoutManager.h"
#include "terminal/TerminalWidget.h"
#include "terminal/TerminalSearchBar.h"
#include "terminal/TerminalModel.h"

#include <QVBoxLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

/** @brief 构造终端布局管理器 @param parent 父对象 */
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

/** @brief 设置共享的数据模型(分栏终端与主终端共用) @param model TerminalModel指针 */
void TerminalLayoutManager::setTerminalModel(TerminalModel* model)
{
    m_model = model;
}

/** @brief 返回终端容器Widget @return 容器Widget指针 */
QWidget* TerminalLayoutManager::container() const
{
    return m_container;
}

/** @brief 返回当前布局模式 @return TerminalLayout枚举值 */
TerminalLayout TerminalLayoutManager::layout() const
{
    return m_layout;
}

/** @brief 返回当前布局下的所有终端控件列表 @return 终端控件指针列表(混合模式返回主终端，分栏模式返回RX/TX终端) */
QList<TerminalWidget*> TerminalLayoutManager::terminalWidgets() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return {m_mainTerminal};
    } else {
        return {m_rxTerminal, m_txTerminal};
    }
}

/** @brief 返回主要终端控件(混合模式返回主终端，分栏模式返回RX终端) @return 主终端控件指针 */
TerminalWidget* TerminalLayoutManager::primaryTerminal() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return m_mainTerminal;
    }
    return m_rxTerminal;
}

/** @brief 通过下拉框索引设置布局模式(0=混合, 1=水平分栏, 2=垂直分栏) @param layoutIndex 下拉框索引 */
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

/** @brief 设置布局模式并重新应用布局 @param layout 目标布局模式 */
void TerminalLayoutManager::setLayout(TerminalLayout layout)
{
    if (m_layout == layout) return;
    m_layout = layout;
    applyLayout();
    emit layoutChanged(m_layout);
}

/** @brief 应用当前布局模式(清空容器→销毁旧分割器→重建搜索栏和终端区域) */
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
}

/** @brief 应用混合布局: 销毁分栏终端，恢复主终端无过滤状态 */
void TerminalLayoutManager::applyMixedLayout()
{
    auto* containerLayout = qobject_cast<QBoxLayout*>(m_container->layout());
    if (!containerLayout) return;

    // ---- 混合模式: 显示主终端（包含TX和RX所有数据） ----
    // 销毁分栏终端（如果存在）
    if (m_rxTerminal) {
        m_rxTerminal->deleteLater();
        m_rxTerminal = nullptr;
    }
    if (m_txTerminal) {
        m_txTerminal->deleteLater();
        m_txTerminal = nullptr;
    }

    // 恢复主终端的无过滤状态
    if (m_mainTerminal) {
        m_mainTerminal->clearDirectionFilter();
        m_mainTerminal->setParent(m_container);
        containerLayout->addWidget(m_mainTerminal, 1);
        m_mainTerminal->show();
    }
}

/** @brief 应用分栏布局: 创建RX/TX终端，用QSplitter分割 */
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
    }
    if (m_txTerminal) {
        m_txTerminal->deleteLater();
    }

    // RX终端: 左侧(左右分栏) 或 上方(上下分栏)
    m_rxTerminal = createSplitTerminal(DataDirection::Rx,
        tr("RX (接收)"));
    m_splitter->addWidget(m_rxTerminal);

    // TX终端: 右侧(左右分栏) 或 下方(上下分栏)
    m_txTerminal = createSplitTerminal(DataDirection::Tx,
        tr("TX (发送)"));
    m_splitter->addWidget(m_txTerminal);

    // 均分分割器空间
    m_splitter->setSizes({1, 1});

    containerLayout->addWidget(m_splitter, 1);
}

/** @brief 创建分栏终端控件(设置共享模型、方向过滤、同步显示设置) @param direction 数据方向(RX/TX) @param label 终端标签 @return 新创建的终端控件指针 */
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

/** @brief 将主终端的显示设置同步到目标终端(显示模式、时间戳、方向前缀、自动滚动) @param target 目标终端控件指针 */
void TerminalLayoutManager::syncDisplaySettings(TerminalWidget* target) const
{
    if (!target) return;
    target->setDisplayMode(m_displayMode);
    target->setShowTimestamp(m_showTimestamp);
    target->setShowDirectionPrefix(m_showDirectionPrefix);
    target->setAutoScroll(true);
}
